#include "dx11_backend.hpp"

#if defined(_WIN32)

#include "modules/yorgl_log.hpp"
#include <algorithm>
#include <cmath>

namespace yorgl {

static constexpr float SDF_EDGE = 180.0f / 255.0f;
static constexpr float SDF_SOFTNESS = 0.55f;

static int nextCodepoint(const char*& p, const char* end) {
    if (p >= end) return 0;
    unsigned char c = static_cast<unsigned char>(*p++);
    if (c < 0x80) return c;
    if ((c >> 5) == 0x6 && p < end) {
        return ((c & 0x1F) << 6) | (static_cast<unsigned char>(*p++) & 0x3F);
    }
    if ((c >> 4) == 0xE && p + 1 < end) {
        int cp = ((c & 0x0F) << 12) | ((static_cast<unsigned char>(*p++) & 0x3F) << 6);
        return cp | (static_cast<unsigned char>(*p++) & 0x3F);
    }
    if ((c >> 3) == 0x1E && p + 2 < end) {
        int cp = ((c & 0x07) << 18) | ((static_cast<unsigned char>(*p++) & 0x3F) << 12);
        cp |= (static_cast<unsigned char>(*p++) & 0x3F) << 6;
        return cp | (static_cast<unsigned char>(*p++) & 0x3F);
    }
    return '?';
}

Dx11Backend::~Dx11Backend() {
    shutdown();
}

bool Dx11Backend::init() {
    D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};
    D3D_FEATURE_LEVEL selected{};
    UINT flags = 0;
#if defined(_DEBUG)
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    HRESULT hr = D3D11CreateDevice(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
        levels, 2, D3D11_SDK_VERSION,
        device_.GetAddressOf(), &selected, context_.GetAddressOf());
    if (FAILED(hr)) {
        LOG_ERROR("D3D11CreateDevice failed: 0x%08X", hr);
        return false;
    }
    featureLevel_ = selected;
    updateTearingSupport();
    return true;
}

void Dx11Backend::shutdown() {
    gui_.shutdown();
    cubemap_.shutdown();
    world_.shutdown();
    dsv_.Reset();
    depthBuffer_.Reset();
    rtv_.Reset();
    swapChain_.Reset();
    context_.Reset();
    device_.Reset();
}

bool Dx11Backend::createSwapChain(std::int64_t windowHandle, const SwapChainOptions& options) {
    width_ = options.width;
    height_ = options.height;
    swapChainOptions_ = options;
    presentMode_ = options.presentMode;

    Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
    Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
    Microsoft::WRL::ComPtr<IDXGIFactory2> factory;
    if (FAILED(device_.As(&dxgiDevice))) return false;
    if (FAILED(dxgiDevice->GetAdapter(&adapter))) return false;
    if (FAILED(adapter->GetParent(IID_PPV_ARGS(&factory)))) return false;

    DXGI_SWAP_CHAIN_DESC1 desc{};
    desc.Width = width_;
    desc.Height = height_;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = static_cast<UINT>(std::clamp(options.bufferCount, 2, 3));
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainFlags_ = options.allowTearing && allowTearing_ ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
    desc.Flags = swapChainFlags_;

    HRESULT hr = factory->CreateSwapChainForHwnd(
        device_.Get(), reinterpret_cast<HWND>(windowHandle), &desc, nullptr, nullptr, swapChain_.GetAddressOf());
    if (FAILED(hr)) return false;

    createRenderTarget();
    gui_.init(device_.Get(), context_.Get());
    cubemap_.init(device_.Get(), context_.Get());
    world_.init(device_.Get(), context_.Get());
    return rtv_ != nullptr && dsv_ != nullptr;
}

void Dx11Backend::resize(int width, int height) {
    if (!swapChain_) return;
    width_ = width;
    height_ = height;
    swapChainOptions_.width = width;
    swapChainOptions_.height = height;
    rtv_.Reset();
    dsv_.Reset();
    depthBuffer_.Reset();
    const UINT bufferCount = static_cast<UINT>(std::clamp(swapChainOptions_.bufferCount, 2, 3));
    lastResizeResult_ = swapChain_->ResizeBuffers(bufferCount, width, height, DXGI_FORMAT_UNKNOWN, swapChainFlags_);
    if (SUCCEEDED(lastResizeResult_)) {
        createRenderTarget();
    } else if (device_) {
        deviceRemovedReason_ = device_->GetDeviceRemovedReason();
    }
}

void Dx11Backend::beginFrame() {
    if (rtv_) context_->OMSetRenderTargets(1, rtv_.GetAddressOf(), dsv_.Get());
}

void Dx11Backend::setViewport(float x, float y, float width, float height) {
    D3D11_VIEWPORT vp{x, y, width, height, 0.0f, 1.0f};
    context_->RSSetViewports(1, &vp);
}

void Dx11Backend::clearColor(float r, float g, float b, float a) {
    if (!rtv_) return;
    float color[] = {r, g, b, a};
    context_->ClearRenderTargetView(rtv_.Get(), color);
}

void Dx11Backend::clearDepth(float depth) {
    if (dsv_) context_->ClearDepthStencilView(dsv_.Get(), D3D11_CLEAR_DEPTH, depth, 0);
}

BackendCapabilities Dx11Backend::capabilities() const {
    BackendCapabilities caps;
    caps.backend = BackendKind::Dx11;
    caps.featureLevelMajor = 11;
    caps.featureLevelMinor = featureLevel_ >= D3D_FEATURE_LEVEL_11_1 ? 1 : 0;
    caps.maxTextureSize = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
    caps.presentVSync = true;
    caps.presentImmediate = true;
    caps.presentTearing = allowTearing_;
    return caps;
}

RenderDiagnostics Dx11Backend::diagnostics() const {
    RenderDiagnostics diagnostics;
    diagnostics.lastResizeResult = static_cast<int>(lastResizeResult_);
    diagnostics.lastPresentResult = static_cast<int>(lastPresentResult_);
    diagnostics.deviceRemovedReason = static_cast<int>(deviceRemovedReason_);
    return diagnostics;
}

void Dx11Backend::setPresentMode(PresentMode mode) {
    presentMode_ = mode;
    swapChainOptions_.presentMode = mode;
}

void Dx11Backend::endFrame() {
    if (!swapChain_) return;
    const bool immediate = presentMode_ == PresentMode::Immediate;
    const UINT syncInterval = immediate ? 0U : 1U;
    const UINT flags = immediate && (swapChainFlags_ & DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING) ? DXGI_PRESENT_ALLOW_TEARING : 0U;
    lastPresentResult_ = swapChain_->Present(syncInterval, flags);
    if (FAILED(lastPresentResult_) && device_) {
        deviceRemovedReason_ = device_->GetDeviceRemovedReason();
    }
}

std::int64_t Dx11Backend::createTexture(int width, int height, const std::uint8_t* rgba, int byteCount) {
    if (!device_ || !rgba || width <= 0 || height <= 0 || byteCount < width * height * 4) return 0;
    D3D11_TEXTURE2D_DESC td{};
    td.Width = width;
    td.Height = height;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA sd{};
    sd.pSysMem = rgba;
    sd.SysMemPitch = width * 4;

    ID3D11Texture2D* texture = nullptr;
    if (FAILED(device_->CreateTexture2D(&td, &sd, &texture))) return 0;
    ID3D11ShaderResourceView* srv = nullptr;
    HRESULT hr = device_->CreateShaderResourceView(texture, nullptr, &srv);
    texture->Release();
    return SUCCEEDED(hr) ? reinterpret_cast<std::int64_t>(srv) : 0;
}

bool Dx11Backend::updateTextureRegion(std::int64_t texture, int x, int y, int width, int height, const std::uint8_t* rgba, int byteCount) {
    if (!context_ || !rgba || texture == 0 || x < 0 || y < 0 || width <= 0 || height <= 0 || byteCount < width * height * 4) return false;
    auto* srv = reinterpret_cast<ID3D11ShaderResourceView*>(texture);
    Microsoft::WRL::ComPtr<ID3D11Resource> resource;
    srv->GetResource(resource.GetAddressOf());
    Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
    if (FAILED(resource.As(&tex))) return false;
    D3D11_TEXTURE2D_DESC desc{};
    tex->GetDesc(&desc);
    if (x + width > static_cast<int>(desc.Width) || y + height > static_cast<int>(desc.Height)) return false;
    D3D11_BOX box{};
    box.left = static_cast<UINT>(x);
    box.top = static_cast<UINT>(y);
    box.front = 0;
    box.right = static_cast<UINT>(x + width);
    box.bottom = static_cast<UINT>(y + height);
    box.back = 1;
    context_->UpdateSubresource(tex.Get(), 0, &box, rgba, static_cast<UINT>(width * 4), 0);
    return true;
}

void Dx11Backend::destroyTexture(std::int64_t texture) {
    auto* srv = reinterpret_cast<ID3D11ShaderResourceView*>(texture);
    if (srv) srv->Release();
}

void Dx11Backend::guiBegin(int width, int height) { gui_.begin(width, height); }
void Dx11Backend::guiDrawQuad(float x, float y, float w, float h, float u0, float v0, float u1, float v1, float r, float g, float b, float a) {
    gui_.drawQuad(x, y, w, h, u0, v0, u1, v1, r, g, b, a);
}
void Dx11Backend::guiDrawGradientQuad(float x, float y, float w, float h, const float* c) {
    if (!c) return;
    gui_.drawGradientQuad(x, y, w, h, c[0], c[1], c[2], c[3], c[4], c[5], c[6], c[7], c[8], c[9], c[10], c[11], c[12], c[13], c[14], c[15]);
}
void Dx11Backend::guiSetTexture(std::int64_t texture) { gui_.setTexture(reinterpret_cast<ID3D11ShaderResourceView*>(texture)); }
void Dx11Backend::guiSetScissor(float x, float y, float w, float h) { gui_.setScissor(x, y, w, h); }
void Dx11Backend::guiClearScissor() { gui_.clearScissor(); }
void Dx11Backend::guiSetSdfMode(bool enabled) { gui_.setSdfMode(enabled); }
void Dx11Backend::guiSetSdfParams(float edge, float softness, float weightBias) { gui_.setSdfParams(edge, softness, weightBias); }
void Dx11Backend::guiBlurRect(float x, float y, float w, float h, int passes) { gui_.blurRect(x, y, w, h, passes, rtv_.Get()); }
void Dx11Backend::guiEnd() { gui_.end(); }

void Dx11Backend::cubemapRender(const std::int64_t* faces, float yawRadians, int width, int height) {
    if (!faces) return;
    ID3D11ShaderResourceView* srv[6] = {
        reinterpret_cast<ID3D11ShaderResourceView*>(faces[0]),
        reinterpret_cast<ID3D11ShaderResourceView*>(faces[1]),
        reinterpret_cast<ID3D11ShaderResourceView*>(faces[2]),
        reinterpret_cast<ID3D11ShaderResourceView*>(faces[3]),
        reinterpret_cast<ID3D11ShaderResourceView*>(faces[4]),
        reinterpret_cast<ID3D11ShaderResourceView*>(faces[5]),
    };
    gui_.flush();
    cubemap_.render(srv, yawRadians, width, height, rtv_.Get());
}

void Dx11Backend::worldUploadMesh(const float* vertices, int floatCount) { world_.uploadMesh(vertices, floatCount); }
void Dx11Backend::worldUploadSection(std::int64_t sectionId, int x, int y, int z, const float* vertices, int floatCount) {
    world_.uploadSection(sectionId, x, y, z, vertices, floatCount);
}
void Dx11Backend::worldUploadSectionLayer(std::int64_t sectionId, int x, int y, int z, int layer, const float* vertices, int floatCount) {
    world_.uploadSectionLayer(sectionId, x, y, z, layer, vertices, floatCount);
}
void Dx11Backend::worldUploadSectionLayerTextured(std::int64_t sectionId, int x, int y, int z, int layer, std::int64_t texture, const float* vertices, int floatCount) {
    world_.uploadSectionLayerTextured(sectionId, x, y, z, layer, reinterpret_cast<ID3D11ShaderResourceView*>(texture), vertices, floatCount);
}
void Dx11Backend::worldRemoveSection(std::int64_t sectionId) { world_.removeSection(sectionId); }
void Dx11Backend::worldClearSections() { world_.clearSections(); }
void Dx11Backend::worldSetTexture(std::int64_t texture) { world_.setTexture(reinterpret_cast<ID3D11ShaderResourceView*>(texture)); }
void Dx11Backend::worldSetTextureFilter(TextureFilter filter) { world_.setTextureFilter(filter); }
void Dx11Backend::worldSetSkyColor(float r, float g, float b) { world_.setSkyColor(r, g, b); }
void Dx11Backend::worldSetFog(float r, float g, float b, float start, float end) { world_.setFog(r, g, b, start, end); }
void Dx11Backend::worldRender(float cameraX, float cameraY, float cameraZ, float dirX, float dirY, float dirZ, float fovYDegrees, float farPlane, int width, int height) {
    gui_.flush();
    world_.render(cameraX, cameraY, cameraZ, dirX, dirY, dirZ, fovYDegrees, farPlane, width, height, rtv_.Get(), dsv_.Get());
}

std::int64_t Dx11Backend::sdfFontCreate(const std::uint8_t* ttfData, int byteCount, float fontSize) {
    if (!ttfData || byteCount <= 0) return 0;
    auto* font = new SdfFontRenderer();
    if (!font->init(device_.Get(), context_.Get(), ttfData, byteCount, fontSize)) {
        delete font;
        return 0;
    }
    return reinterpret_cast<std::int64_t>(font);
}
void Dx11Backend::sdfFontDestroy(std::int64_t font) {
    auto* renderer = reinterpret_cast<SdfFontRenderer*>(font);
    if (!renderer) return;
    renderer->shutdown();
    delete renderer;
}
std::int64_t Dx11Backend::sdfFontAtlas(std::int64_t font) {
    auto* renderer = reinterpret_cast<SdfFontRenderer*>(font);
    return renderer ? reinterpret_cast<std::int64_t>(renderer->getAtlasSRV()) : 0;
}
bool Dx11Backend::sdfFontMetrics(std::int64_t font, float* out3) {
    auto* renderer = reinterpret_cast<SdfFontRenderer*>(font);
    if (!renderer || !out3) return false;
    out3[0] = renderer->getLineHeight();
    out3[1] = renderer->getAscent();
    out3[2] = renderer->getDescent();
    return true;
}
bool Dx11Backend::sdfFontGlyph(std::int64_t font, int codepoint, float* out9) {
    auto* renderer = reinterpret_cast<SdfFontRenderer*>(font);
    const GlyphInfo* glyph = renderer ? renderer->getGlyph(codepoint) : nullptr;
    if (!glyph || !out9) return false;
    out9[0] = glyph->u0; out9[1] = glyph->v0; out9[2] = glyph->u1; out9[3] = glyph->v1;
    out9[4] = glyph->xoff; out9[5] = glyph->yoff; out9[6] = glyph->width; out9[7] = glyph->height; out9[8] = glyph->advance;
    return true;
}
float Dx11Backend::sdfFontKerning(std::int64_t font, int leftCodepoint, int rightCodepoint) {
    auto* renderer = reinterpret_cast<SdfFontRenderer*>(font);
    return renderer ? renderer->getKerning(leftCodepoint, rightCodepoint) : 0.0f;
}

float Dx11Backend::sdfFontTextWidth(std::int64_t font, const char* utf8, int byteCount, float scale) {
    auto* renderer = reinterpret_cast<SdfFontRenderer*>(font);
    if (!renderer || !utf8 || byteCount <= 0 || renderer->getFontSize() <= 0.0f) return 0.0f;
    const float s = scale / renderer->getFontSize();
    const char* p = utf8;
    const char* end = utf8 + byteCount;
    float width = 0.0f;
    int previous = 0;
    while (p < end) {
        int cp = nextCodepoint(p, end);
        if (previous != 0) width += renderer->getKerning(previous, cp) * s;
        if (const GlyphInfo* glyph = renderer->getGlyph(cp)) width += glyph->advance * s;
        else width += renderer->getFontSize() * s * 0.5f;
        previous = cp;
    }
    return width;
}

float Dx11Backend::sdfFontLineHeight(std::int64_t font, float scale) {
    auto* renderer = reinterpret_cast<SdfFontRenderer*>(font);
    return renderer && renderer->getFontSize() > 0.0f ? renderer->getLineHeight() * scale / renderer->getFontSize() : 0.0f;
}

void Dx11Backend::sdfFontDrawText(std::int64_t font, const char* utf8, int byteCount, float x, float y, float scale, float r, float g, float b, float a, float weight, bool shadow) {
    auto* renderer = reinterpret_cast<SdfFontRenderer*>(font);
    if (!renderer || !utf8 || byteCount <= 0 || renderer->getFontSize() <= 0.0f) return;
    auto draw = [&](float ox, float oy, float cr, float cg, float cb) {
        const float s = scale / renderer->getFontSize();
        const char* p = utf8;
        const char* end = utf8 + byteCount;
        float cx = std::floor(x + ox);
        const float baselineY = std::floor(y + oy);
        int previous = 0;
        while (p < end) {
            int cp = nextCodepoint(p, end);
            if (previous != 0) cx += renderer->getKerning(previous, cp) * s;
            const GlyphInfo* glyph = renderer->getGlyph(cp);
            if (!glyph) {
                cx += renderer->getFontSize() * s * 0.5f;
                previous = cp;
                continue;
            }
            if (glyph->width > 0.0f && glyph->height > 0.0f) {
                gui_.drawQuad(
                    cx + glyph->xoff * s,
                    baselineY + (renderer->getAscent() + glyph->yoff) * s,
                    glyph->width * s,
                    glyph->height * s,
                    glyph->u0, glyph->v0, glyph->u1, glyph->v1,
                    cr, cg, cb, a);
            }
            cx += glyph->advance * s;
            previous = cp;
        }
    };
    const float weightBias = std::clamp((weight - 400.0f) / 500.0f * 0.018f, -0.018f, 0.018f);
    gui_.setSdfMode(true);
    gui_.setSdfParams(SDF_EDGE, SDF_SOFTNESS, weightBias);
    gui_.setTexture(renderer->getAtlasSRV());
    if (shadow) {
        const float offset = scale * 0.04f;
        draw(offset, offset, r * 0.25f, g * 0.25f, b * 0.25f);
    }
    draw(0.0f, 0.0f, r, g, b);
    gui_.setTexture(nullptr);
    gui_.setSdfParams(SDF_EDGE, SDF_SOFTNESS, 0.0f);
    gui_.setSdfMode(false);
}

void Dx11Backend::createRenderTarget() {
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    if (FAILED(swapChain_->GetBuffer(0, IID_PPV_ARGS(&backBuffer)))) return;
    device_->CreateRenderTargetView(backBuffer.Get(), nullptr, rtv_.GetAddressOf());

    D3D11_TEXTURE2D_DESC depthDesc{};
    depthDesc.Width = width_;
    depthDesc.Height = height_;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    if (SUCCEEDED(device_->CreateTexture2D(&depthDesc, nullptr, depthBuffer_.GetAddressOf()))) {
        device_->CreateDepthStencilView(depthBuffer_.Get(), nullptr, dsv_.GetAddressOf());
    }
}

void Dx11Backend::updateTearingSupport() {
    allowTearing_ = false;
    Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
    Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
    Microsoft::WRL::ComPtr<IDXGIFactory2> factory;
    Microsoft::WRL::ComPtr<IDXGIFactory5> factory5;
    if (FAILED(device_.As(&dxgiDevice))) return;
    if (FAILED(dxgiDevice->GetAdapter(&adapter))) return;
    if (FAILED(adapter->GetParent(IID_PPV_ARGS(&factory)))) return;
    if (FAILED(factory.As(&factory5))) return;
    BOOL supported = FALSE;
    if (SUCCEEDED(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &supported, sizeof(supported)))) {
        allowTearing_ = supported == TRUE;
    }
}

} // namespace yorgl

#endif
