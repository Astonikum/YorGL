#include "gui_renderer.h"
#include "yorgl_log.hpp"
#include <d3dcompiler.h>
#include <cstring>

static const char* GUI_VS_SRC = R"(
cbuffer PerFrame : register(b0) {
    float4x4 projection;
};
struct VSInput {
    float2 pos : POSITION;
    float2 uv  : TEXCOORD;
    float4 col : COLOR;
};
struct PSInput {
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD;
    float4 col : COLOR;
};
PSInput VSMain(VSInput input) {
    PSInput o;
    o.pos = mul(projection, float4(input.pos, 0.0, 1.0));
    o.uv = input.uv;
    o.col = input.col;
    return o;
}
)";

static const char* GUI_PS_SRC = R"(
Texture2D tex : register(t0);
SamplerState samp : register(s0);
struct PSInput {
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD;
    float4 col : COLOR;
};
float4 PSMain(PSInput input) : SV_TARGET {
    float4 tc = tex.Sample(samp, input.uv);
    return tc * input.col;
}
)";

static const char* GUI_SDF_PS_SRC = R"(
Texture2D tex : register(t0);
SamplerState samp : register(s0);
cbuffer SdfParams : register(b1) {
    float sdfEdge;
    float sdfSoftness;
    float sdfWeightBias;
    float sdfGamma;
};
struct PSInput {
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD;
    float4 col : COLOR;
};
float4 PSMain(PSInput input) : SV_TARGET {
    float d = tex.Sample(samp, input.uv).a + sdfWeightBias;
    float w = max(fwidth(d) * sdfSoftness, 0.0025);
    float a = smoothstep(sdfEdge - w, sdfEdge + w, d);
    a = pow(saturate(a), sdfGamma);
    return float4(input.col.rgb, input.col.a * a);
}
)";

bool GuiRenderer::init(ID3D11Device* device, ID3D11DeviceContext* ctx) {
    device_ = device;
    ctx_ = ctx;
    LOG_INFO("GuiRenderer: initializing...");
    createShaders();
    createBuffers();
    createStates();
    createBlurResources();
    vertices_.reserve(MAX_VERTICES);
    LOG_INFO("GuiRenderer: ready (max %d quads per batch)", MAX_QUADS);
    return true;
}

void GuiRenderer::shutdown() {
    LOG_INFO("GuiRenderer: shutdown");
    vs_.Reset(); ps_.Reset(); psSdf_.Reset(); inputLayout_.Reset();
    vertexBuffer_.Reset(); constantBuffer_.Reset(); sdfConstantBuffer_.Reset();
    blendState_.Reset(); sampler_.Reset(); samplerLinear_.Reset();
    rasterState_.Reset(); depthOff_.Reset();
    whiteTexSRV_.Reset(); whiteTex_.Reset();
}

void GuiRenderer::createShaders() {
    ComPtr<ID3DBlob> vsBlob, psBlob, errBlob;

    HRESULT hr = D3DCompile(GUI_VS_SRC, strlen(GUI_VS_SRC), "gui_vs",
        nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vsBlob, &errBlob);
    if (FAILED(hr)) {
        LOG_ERROR("GUI VS compile failed: %s", (char*)errBlob->GetBufferPointer());
        return;
    }

    hr = D3DCompile(GUI_PS_SRC, strlen(GUI_PS_SRC), "gui_ps",
        nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &psBlob, &errBlob);
    if (FAILED(hr)) {
        LOG_ERROR("GUI PS compile failed: %s", (char*)errBlob->GetBufferPointer());
        return;
    }

    device_->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vs_);
    device_->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &ps_);

    ComPtr<ID3DBlob> sdfBlob;
    hr = D3DCompile(GUI_SDF_PS_SRC, strlen(GUI_SDF_PS_SRC), "gui_sdf_ps",
        nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &sdfBlob, &errBlob);
    if (SUCCEEDED(hr)) {
        device_->CreatePixelShader(sdfBlob->GetBufferPointer(), sdfBlob->GetBufferSize(), nullptr, &psSdf_);
    } else {
        LOG_ERROR("GUI SDF PS compile failed: %s", (char*)errBlob->GetBufferPointer());
    }

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    device_->CreateInputLayout(layout, 3, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout_);
    LOG_DEBUG("GuiRenderer: shaders compiled, input layout created");
}

void GuiRenderer::createBuffers() {
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.ByteWidth = MAX_VERTICES * sizeof(GuiVertex);
    vbDesc.Usage = D3D11_USAGE_DYNAMIC;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    device_->CreateBuffer(&vbDesc, nullptr, &vertexBuffer_);

    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = 64; // float4x4
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    device_->CreateBuffer(&cbDesc, nullptr, &constantBuffer_);

    D3D11_BUFFER_DESC sdfDesc = {};
    sdfDesc.ByteWidth = 16; // float4: edge, softness, weight bias, gamma
    sdfDesc.Usage = D3D11_USAGE_DYNAMIC;
    sdfDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    sdfDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    device_->CreateBuffer(&sdfDesc, nullptr, &sdfConstantBuffer_);

    // 1x1 white texture for untextured quads
    D3D11_TEXTURE2D_DESC td = {};
    td.Width = 1; td.Height = 1; td.MipLevels = 1; td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_IMMUTABLE;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    uint32_t white = 0xFFFFFFFF;
    D3D11_SUBRESOURCE_DATA sd = { &white, 4, 0 };
    device_->CreateTexture2D(&td, &sd, &whiteTex_);
    device_->CreateShaderResourceView(whiteTex_.Get(), nullptr, &whiteTexSRV_);
}

void GuiRenderer::createStates() {
    D3D11_BLEND_DESC bd = {};
    bd.RenderTarget[0].BlendEnable = TRUE;
    bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    device_->CreateBlendState(&bd, &blendState_);

    D3D11_SAMPLER_DESC sDesc = {};
    sDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    sDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    device_->CreateSamplerState(&sDesc, &sampler_);

    D3D11_SAMPLER_DESC slDesc = {};
    slDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    slDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    slDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    slDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    device_->CreateSamplerState(&slDesc, &samplerLinear_);

    D3D11_RASTERIZER_DESC rd = {};
    rd.FillMode = D3D11_FILL_SOLID;
    rd.CullMode = D3D11_CULL_NONE;
    rd.ScissorEnable = FALSE;
    device_->CreateRasterizerState(&rd, &rasterState_);

    rd.ScissorEnable = TRUE;
    device_->CreateRasterizerState(&rd, &rasterStateScissor_);

    D3D11_DEPTH_STENCIL_DESC dsd = {};
    dsd.DepthEnable = FALSE;
    device_->CreateDepthStencilState(&dsd, &depthOff_);
}

void GuiRenderer::updateProjection(int w, int h) {
    // Orthographic: (0,0) top-left, (w,h) bottom-right, NDC [-1,1]
    float proj[16] = {
        2.0f / w,  0,          0, 0,
        0,        -2.0f / h,   0, 0,
        0,         0,          1, 0,
       -1.0f,      1.0f,       0, 1
    };
    D3D11_MAPPED_SUBRESOURCE mapped;
    ctx_->Map(constantBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, proj, 64);
    ctx_->Unmap(constantBuffer_.Get(), 0);
}

void GuiRenderer::begin(int screenWidth, int screenHeight) {
    screenW_ = screenWidth;
    screenH_ = screenHeight;
    vertices_.clear();
    currentSRV_ = whiteTexSRV_.Get();
    updateProjection(screenWidth, screenHeight);

    ctx_->IASetInputLayout(inputLayout_.Get());
    ctx_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx_->VSSetShader(vs_.Get(), nullptr, 0);
    ctx_->PSSetShader(ps_.Get(), nullptr, 0);
    ctx_->VSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
    ctx_->PSSetConstantBuffers(1, 1, sdfConstantBuffer_.GetAddressOf());
    ctx_->PSSetSamplers(0, 1, sampler_.GetAddressOf());
    ctx_->OMSetBlendState(blendState_.Get(), nullptr, 0xFFFFFFFF);
    ctx_->RSSetState(rasterState_.Get());
    ctx_->OMSetDepthStencilState(depthOff_.Get(), 0);
}

void GuiRenderer::setTexture(ID3D11ShaderResourceView* srv) {
    if (srv != currentSRV_) {
        flush();
        currentSRV_ = srv ? srv : whiteTexSRV_.Get();
    }
}

void GuiRenderer::drawQuad(float x, float y, float w, float h,
                           float u0, float v0, float u1, float v1,
                           float r, float g, float b, float a) {
    if (vertices_.size() + 6 > MAX_VERTICES) flush();

    GuiVertex tl = {x,     y,     u0, v0, r, g, b, a};
    GuiVertex tr = {x + w, y,     u1, v0, r, g, b, a};
    GuiVertex bl = {x,     y + h, u0, v1, r, g, b, a};
    GuiVertex br = {x + w, y + h, u1, v1, r, g, b, a};

    vertices_.push_back(tl);
    vertices_.push_back(tr);
    vertices_.push_back(bl);
    vertices_.push_back(tr);
    vertices_.push_back(br);
    vertices_.push_back(bl);
}

void GuiRenderer::drawGradientQuad(float x, float y, float w, float h,
                                   float tlR, float tlG, float tlB, float tlA,
                                   float trR, float trG, float trB, float trA,
                                   float brR, float brG, float brB, float brA,
                                   float blR, float blG, float blB, float blA) {
    if (vertices_.size() + 6 > MAX_VERTICES) flush();

    GuiVertex tl = {x,     y,     0, 0, tlR, tlG, tlB, tlA};
    GuiVertex tr = {x + w, y,     1, 0, trR, trG, trB, trA};
    GuiVertex bl = {x,     y + h, 0, 1, blR, blG, blB, blA};
    GuiVertex br = {x + w, y + h, 1, 1, brR, brG, brB, brA};

    vertices_.push_back(tl);
    vertices_.push_back(tr);
    vertices_.push_back(bl);
    vertices_.push_back(tr);
    vertices_.push_back(br);
    vertices_.push_back(bl);
}

void GuiRenderer::drawQuadTextured(float x, float y, float w, float h,
                                   float u0, float v0, float u1, float v1,
                                   float r, float g, float b, float a,
                                   ID3D11ShaderResourceView* srv) {
    setTexture(srv);
    drawQuad(x, y, w, h, u0, v0, u1, v1, r, g, b, a);
}

void GuiRenderer::flush() {
    if (vertices_.empty()) return;

    D3D11_MAPPED_SUBRESOURCE mapped;
    ctx_->Map(vertexBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, vertices_.data(), vertices_.size() * sizeof(GuiVertex));
    ctx_->Unmap(vertexBuffer_.Get(), 0);

    UINT stride = sizeof(GuiVertex), offset = 0;
    ctx_->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride, &offset);
    ctx_->PSSetShaderResources(0, 1, &currentSRV_);
    ctx_->Draw((UINT)vertices_.size(), 0);
    vertices_.clear();
}

void GuiRenderer::end() {
    flush();
}

void GuiRenderer::setSdfMode(bool sdf) {
    if (sdf != sdfMode_) {
        flush();
        sdfMode_ = sdf;
        if (sdf) {
            ctx_->PSSetShader(psSdf_.Get(), nullptr, 0);
            ctx_->PSSetSamplers(0, 1, samplerLinear_.GetAddressOf());
        } else {
            ctx_->PSSetShader(ps_.Get(), nullptr, 0);
            ctx_->PSSetSamplers(0, 1, sampler_.GetAddressOf());
        }
    }
}

void GuiRenderer::setSdfParams(float edge, float softness, float weightBias) {
    edge = edge < 0.05f ? 0.05f : (edge > 0.95f ? 0.95f : edge);
    softness = softness < 0.1f ? 0.1f : (softness > 2.0f ? 2.0f : softness);
    weightBias = weightBias < -0.08f ? -0.08f : (weightBias > 0.08f ? 0.08f : weightBias);
    flush();
    sdfEdge_ = edge;
    sdfSoftness_ = softness;
    sdfWeightBias_ = weightBias;

    float params[4] = {sdfEdge_, sdfSoftness_, sdfWeightBias_, 1.0f};
    D3D11_MAPPED_SUBRESOURCE mapped;
    ctx_->Map(sdfConstantBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, params, sizeof(params));
    ctx_->Unmap(sdfConstantBuffer_.Get(), 0);
}

void GuiRenderer::setScissor(float x, float y, float w, float h) {
    flush();
    D3D11_RECT rect = {
        (LONG)x, (LONG)y,
        (LONG)(x + w), (LONG)(y + h)
    };
    ctx_->RSSetScissorRects(1, &rect);
    ctx_->RSSetState(rasterStateScissor_.Get());
}

void GuiRenderer::clearScissor() {
    flush();
    ctx_->RSSetState(rasterState_.Get());
}

// в”Ђв”Ђ Blur implementation в”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђ

static const char* FULLSCREEN_VS = R"(
struct PSInput { float4 pos : SV_POSITION; float2 uv : TEXCOORD; };
PSInput VSMain(uint id : SV_VertexID) {
    PSInput o;
    o.uv = float2((id << 1) & 2, id & 2);
    o.pos = float4(o.uv * float2(2,-2) + float2(-1,1), 0, 1);
    return o;
}
)";

static const char* BLUR_PS = R"(
Texture2D tex : register(t0);
SamplerState samp : register(s0);
cbuffer BlurParams : register(b0) { float2 direction; float2 texelSize; };
struct PSInput { float4 pos : SV_POSITION; float2 uv : TEXCOORD; };
static const float weights[5] = {0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216};
float4 PSMain(PSInput i) : SV_TARGET {
    float2 offset = direction * texelSize;
    float4 result = tex.Sample(samp, i.uv) * weights[0];
    for (int j = 1; j < 5; j++) {
        result += tex.Sample(samp, i.uv + offset * j) * weights[j];
        result += tex.Sample(samp, i.uv - offset * j) * weights[j];
    }
    return result;
}
)";

void GuiRenderer::createBlurResources() {
    ComPtr<ID3DBlob> blob, err;
    HRESULT hr = D3DCompile(FULLSCREEN_VS, strlen(FULLSCREEN_VS), "fs_vs",
        nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &blob, &err);
    if (SUCCEEDED(hr))
        device_->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &vsFullscreen_);

    hr = D3DCompile(BLUR_PS, strlen(BLUR_PS), "blur_ps",
        nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &blob, &err);
    if (SUCCEEDED(hr)) {
        device_->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &psBlurH_);
        device_->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &psBlurV_);
    }

    D3D11_BUFFER_DESC cb = {};
    cb.ByteWidth = 16;
    cb.Usage = D3D11_USAGE_DYNAMIC;
    cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    device_->CreateBuffer(&cb, nullptr, &blurCB_);
}

void GuiRenderer::blurRect(float x, float y, float w, float h, int passes,
                           ID3D11RenderTargetView* mainRTV) {
    flush();
    if (!vsFullscreen_ || !psBlurH_) return;

    if (blurW_ != screenW_ || blurH_ != screenH_) {
        blurTexA_.Reset(); blurTexB_.Reset();
        blurRtvA_.Reset(); blurRtvB_.Reset();
        blurSrvA_.Reset(); blurSrvB_.Reset();
        blurW_ = screenW_; blurH_ = screenH_;

        D3D11_TEXTURE2D_DESC td = {};
        td.Width = blurW_; td.Height = blurH_;
        td.MipLevels = 1; td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        device_->CreateTexture2D(&td, nullptr, &blurTexA_);
        device_->CreateTexture2D(&td, nullptr, &blurTexB_);
        device_->CreateRenderTargetView(blurTexA_.Get(), nullptr, &blurRtvA_);
        device_->CreateRenderTargetView(blurTexB_.Get(), nullptr, &blurRtvB_);
        device_->CreateShaderResourceView(blurTexA_.Get(), nullptr, &blurSrvA_);
        device_->CreateShaderResourceView(blurTexB_.Get(), nullptr, &blurSrvB_);
    }

    // Copy backbuffer to blurTexA
    ComPtr<ID3D11Resource> bbRes;
    mainRTV->GetResource(&bbRes);
    ctx_->CopyResource(blurTexA_.Get(), bbRes.Get());

    ctx_->IASetInputLayout(nullptr);
    ctx_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx_->VSSetShader(vsFullscreen_.Get(), nullptr, 0);
    ctx_->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
    ctx_->PSSetSamplers(0, 1, samplerLinear_.GetAddressOf());

    D3D11_VIEWPORT vp = {0, 0, (float)blurW_, (float)blurH_, 0, 1};
    ctx_->RSSetViewports(1, &vp);

    float texel[4] = {1.0f / blurW_, 1.0f / blurH_, 0, 0};

    for (int p = 0; p < passes; p++) {
        // Horizontal: A -> B
        float dirH[4] = {1, 0, texel[0], texel[1]};
        D3D11_MAPPED_SUBRESOURCE m;
        ctx_->Map(blurCB_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &m);
        memcpy(m.pData, dirH, 16);
        ctx_->Unmap(blurCB_.Get(), 0);

        ctx_->OMSetRenderTargets(1, blurRtvB_.GetAddressOf(), nullptr);
        ctx_->PSSetShader(psBlurH_.Get(), nullptr, 0);
        ctx_->PSSetConstantBuffers(0, 1, blurCB_.GetAddressOf());
        ctx_->PSSetShaderResources(0, 1, blurSrvA_.GetAddressOf());
        ctx_->Draw(3, 0);

        // Vertical: B -> A
        float dirV[4] = {0, 1, texel[0], texel[1]};
        ctx_->Map(blurCB_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &m);
        memcpy(m.pData, dirV, 16);
        ctx_->Unmap(blurCB_.Get(), 0);

        ctx_->OMSetRenderTargets(1, blurRtvA_.GetAddressOf(), nullptr);
        ctx_->PSSetShaderResources(0, 1, blurSrvB_.GetAddressOf());
        ctx_->Draw(3, 0);
    }

    // Restore main RTV and draw blurred region as quad
    ctx_->OMSetRenderTargets(1, &mainRTV, nullptr);
    ID3D11ShaderResourceView* null_srv = nullptr;
    ctx_->PSSetShaderResources(0, 1, &null_srv);

    // Restore GUI state
    ctx_->IASetInputLayout(inputLayout_.Get());
    ctx_->VSSetShader(vs_.Get(), nullptr, 0);
    ctx_->PSSetShader(ps_.Get(), nullptr, 0);
    ctx_->VSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
    ctx_->PSSetSamplers(0, 1, sampler_.GetAddressOf());
    ctx_->OMSetBlendState(blendState_.Get(), nullptr, 0xFFFFFFFF);
    ctx_->OMSetDepthStencilState(depthOff_.Get(), 0);

    // Draw the blurred texture as a quad in the specified rect
    float u0 = x / screenW_, v0 = y / screenH_;
    float u1 = (x + w) / screenW_, v1 = (y + h) / screenH_;
    setTexture(blurSrvA_.Get());
    drawQuad(x, y, w, h, u0, v0, u1, v1, 1, 1, 1, 1);
    flush();
    setTexture(whiteTexSRV_.Get());
}
