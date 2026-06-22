#include "dx11_backend.hpp"

#if defined(_WIN32)

#include "modules/yorgl_log.hpp"

namespace yorgl {

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
    return true;
}

void Dx11Backend::shutdown() {
    gui_.shutdown();
    panorama_.shutdown();
    world_.shutdown();
    dsv_.Reset();
    depthBuffer_.Reset();
    rtv_.Reset();
    swapChain_.Reset();
    context_.Reset();
    device_.Reset();
}

bool Dx11Backend::createSwapChain(std::int64_t windowHandle, int width, int height) {
    width_ = width;
    height_ = height;

    Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
    Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
    Microsoft::WRL::ComPtr<IDXGIFactory2> factory;
    if (FAILED(device_.As(&dxgiDevice))) return false;
    if (FAILED(dxgiDevice->GetAdapter(&adapter))) return false;
    if (FAILED(adapter->GetParent(IID_PPV_ARGS(&factory)))) return false;

    DXGI_SWAP_CHAIN_DESC1 desc{};
    desc.Width = width;
    desc.Height = height;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 2;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    HRESULT hr = factory->CreateSwapChainForHwnd(
        device_.Get(), reinterpret_cast<HWND>(windowHandle), &desc, nullptr, nullptr, swapChain_.GetAddressOf());
    if (FAILED(hr)) return false;

    createRenderTarget();
    gui_.init(device_.Get(), context_.Get());
    panorama_.init(device_.Get(), context_.Get());
    world_.init(device_.Get(), context_.Get());
    return rtv_ != nullptr && dsv_ != nullptr;
}

void Dx11Backend::resize(int width, int height) {
    if (!swapChain_) return;
    width_ = width;
    height_ = height;
    rtv_.Reset();
    dsv_.Reset();
    depthBuffer_.Reset();
    if (SUCCEEDED(swapChain_->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0))) {
        createRenderTarget();
    }
}

void Dx11Backend::beginFrame() {
    if (rtv_) context_->OMSetRenderTargets(1, rtv_.GetAddressOf(), dsv_.Get());
}

void Dx11Backend::clearColor(float r, float g, float b, float a) {
    if (!rtv_) return;
    float color[] = {r, g, b, a};
    context_->ClearRenderTargetView(rtv_.Get(), color);
}

void Dx11Backend::endFrame() {
    if (swapChain_) swapChain_->Present(1, 0);
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
    td.Usage = D3D11_USAGE_IMMUTABLE;
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

void Dx11Backend::panoramaRender(const std::int64_t* faces, float angle, int width, int height) {
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
    panorama_.render(srv, angle, width, height, rtv_.Get());
}

void Dx11Backend::worldUploadMesh(const float* vertices, int floatCount) { world_.uploadMesh(vertices, floatCount); }
void Dx11Backend::worldUploadSection(std::int64_t sectionId, int x, int y, int z, const float* vertices, int floatCount) {
    world_.uploadSection(sectionId, x, y, z, vertices, floatCount);
}
void Dx11Backend::worldUploadSectionLayer(std::int64_t sectionId, int x, int y, int z, int layer, const float* vertices, int floatCount) {
    world_.uploadSectionLayer(sectionId, x, y, z, layer, vertices, floatCount);
}
void Dx11Backend::worldRemoveSection(std::int64_t sectionId) { world_.removeSection(sectionId); }
void Dx11Backend::worldClearSections() { world_.clearSections(); }
void Dx11Backend::worldSetTexture(std::int64_t texture) { world_.setTexture(reinterpret_cast<ID3D11ShaderResourceView*>(texture)); }
void Dx11Backend::worldSetSkyColor(float r, float g, float b) { world_.setSkyColor(r, g, b); }
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

} // namespace yorgl

#endif
