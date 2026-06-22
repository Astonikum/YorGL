#include "cubemap_renderer.h"
#include "yorgl_log.hpp"
#include <d3dcompiler.h>
#include <cstring>
#include <cmath>
#include <cstdint>

static const char* CUBEMAP_VS = R"(
cbuffer PerFrame : register(b0) {
    float4x4 viewProjection;
};
struct VSInput {
    float3 pos : POSITION;
    float2 uv  : TEXCOORD;
    uint faceId : BLENDINDICES;
};
struct PSInput {
    float4 pos    : SV_POSITION;
    float2 uv     : TEXCOORD;
    uint   faceId : BLENDINDICES;
};
PSInput VSMain(VSInput i) {
    PSInput o;
    o.pos = mul(viewProjection, float4(i.pos, 1.0));
    o.uv = i.uv;
    o.faceId = i.faceId;
    return o;
}
)";

static const char* CUBEMAP_PS = R"(
Texture2D face0 : register(t0);
Texture2D face1 : register(t1);
Texture2D face2 : register(t2);
Texture2D face3 : register(t3);
Texture2D face4 : register(t4);
Texture2D face5 : register(t5);
SamplerState samp : register(s0);
struct PSInput {
    float4 pos    : SV_POSITION;
    float2 uv     : TEXCOORD;
    uint   faceId : BLENDINDICES;
};
float4 PSMain(PSInput i) : SV_TARGET {
    float2 uv = i.uv;
    if (i.faceId == 0) return face0.Sample(samp, uv);
    if (i.faceId == 1) return face1.Sample(samp, uv);
    if (i.faceId == 2) return face2.Sample(samp, uv);
    if (i.faceId == 3) return face3.Sample(samp, uv);
    if (i.faceId == 4) return face4.Sample(samp, uv);
    return face5.Sample(samp, uv);
}
)";

struct CubeVertex {
    float x, y, z;
    float u, v;
    unsigned int faceId;
};

static void buildViewProjection(float out[16], float angle, float aspect) {
    float fov = 1.2217f; // ~70 degrees
    float nearZ = 0.05f, farZ = 10.0f;
    float f = 1.0f / tanf(fov * 0.5f);

    float proj[16] = {
        f / aspect, 0,  0,                              0,
        0,          f,  0,                              0,
        0,          0,  farZ / (farZ - nearZ),          1,
        0,          0, -nearZ * farZ / (farZ - nearZ),  0
    };

    float c = cosf(angle), s = sinf(angle);
    float view[16] = {
         c, 0, s, 0,
         0, 1, 0, 0,
        -s, 0, c, 0,
         0, 0, 0, 1
    };

    memset(out, 0, 64);
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            for (int k = 0; k < 4; k++)
                out[i * 4 + j] += view[i * 4 + k] * proj[k * 4 + j];
}

bool CubemapRenderer::init(ID3D11Device* device, ID3D11DeviceContext* ctx) {
    device_ = device;
    ctx_ = ctx;
    LOG_INFO("CubemapRenderer: initializing...");
    createShaders();
    createGeometry();
    createStates();
    initialized_ = true;
    LOG_INFO("CubemapRenderer: ready");
    return true;
}

void CubemapRenderer::shutdown() {
    vs_.Reset(); ps_.Reset(); inputLayout_.Reset();
    vertexBuffer_.Reset(); constantBuffer_.Reset();
    sampler_.Reset(); rasterState_.Reset();
    depthOff_.Reset(); blendOff_.Reset();
    initialized_ = false;
}

void CubemapRenderer::createShaders() {
    ComPtr<ID3DBlob> vsBlob, psBlob, errBlob;

    HRESULT hr = D3DCompile(CUBEMAP_VS, strlen(CUBEMAP_VS), "cubemap_vs",
        nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vsBlob, &errBlob);
    if (FAILED(hr)) {
        LOG_ERROR("Cubemap VS compile: %s", (char*)errBlob->GetBufferPointer());
        return;
    }

    hr = D3DCompile(CUBEMAP_PS, strlen(CUBEMAP_PS), "cubemap_ps",
        nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &psBlob, &errBlob);
    if (FAILED(hr)) {
        LOG_ERROR("Cubemap PS compile: %s", (char*)errBlob->GetBufferPointer());
        return;
    }

    device_->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vs_);
    device_->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &ps_);

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"BLENDINDICES", 0, DXGI_FORMAT_R32_UINT,        0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    device_->CreateInputLayout(layout, 3, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout_);
}

void CubemapRenderer::createGeometry() {
    // Inverted unit cube (camera inside looking out). 6 faces Г— 2 triangles Г— 3 verts = 36.
    // MC panorama face order: front(0), right(1), back(2), left(3), top(4), bottom(5)
    CubeVertex verts[36];
    int idx = 0;

    auto face = [&](unsigned int fid,
                    float x0, float y0, float z0,
                    float x1, float y1, float z1,
                    float x2, float y2, float z2,
                    float x3, float y3, float z3) {
        verts[idx++] = CubeVertex{x0, y0, z0, 0, 0, fid};
        verts[idx++] = CubeVertex{x1, y1, z1, 1, 0, fid};
        verts[idx++] = CubeVertex{x2, y2, z2, 1, 1, fid};
        verts[idx++] = CubeVertex{x0, y0, z0, 0, 0, fid};
        verts[idx++] = CubeVertex{x2, y2, z2, 1, 1, fid};
        verts[idx++] = CubeVertex{x3, y3, z3, 0, 1, fid};
    };

    // Front (face 0): -Z, looking from inside means verts wound CW from outside
    face(0,  1, 1,-1,  -1, 1,-1,  -1,-1,-1,   1,-1,-1);
    // Right (face 1): +X
    face(1,  1, 1, 1,   1, 1,-1,   1,-1,-1,   1,-1, 1);
    // Back (face 2): +Z
    face(2, -1, 1, 1,   1, 1, 1,   1,-1, 1,  -1,-1, 1);
    // Left (face 3): -X
    face(3, -1, 1,-1,  -1, 1, 1,  -1,-1, 1,  -1,-1,-1);
    // Top (face 4): +Y
    face(4, -1, 1, 1,  -1, 1,-1,   1, 1,-1,   1, 1, 1);
    // Bottom (face 5): -Y
    face(5, -1,-1,-1,  -1,-1, 1,   1,-1, 1,   1,-1,-1);

    D3D11_BUFFER_DESC bd = {};
    bd.ByteWidth = sizeof(verts);
    bd.Usage = D3D11_USAGE_IMMUTABLE;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA sd = {verts, 0, 0};
    device_->CreateBuffer(&bd, &sd, &vertexBuffer_);

    D3D11_BUFFER_DESC cb = {};
    cb.ByteWidth = 64;
    cb.Usage = D3D11_USAGE_DYNAMIC;
    cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    device_->CreateBuffer(&cb, nullptr, &constantBuffer_);
}

void CubemapRenderer::createStates() {
    D3D11_SAMPLER_DESC sd = {};
    sd.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    device_->CreateSamplerState(&sd, &sampler_);

    D3D11_RASTERIZER_DESC rd = {};
    rd.FillMode = D3D11_FILL_SOLID;
    rd.CullMode = D3D11_CULL_NONE;
    device_->CreateRasterizerState(&rd, &rasterState_);

    D3D11_DEPTH_STENCIL_DESC dsd = {};
    dsd.DepthEnable = FALSE;
    device_->CreateDepthStencilState(&dsd, &depthOff_);

    D3D11_BLEND_DESC bd = {};
    bd.RenderTarget[0].BlendEnable = FALSE;
    bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    device_->CreateBlendState(&bd, &blendOff_);
}

void CubemapRenderer::render(ID3D11ShaderResourceView* faces[6], float rotationAngle,
                             int screenW, int screenH, ID3D11RenderTargetView* rtv) {
    if (!initialized_ || !vs_ || !ps_) return;

    float aspect = (float)screenW / (float)screenH;
    float vp[16];
    buildViewProjection(vp, rotationAngle, aspect);

    D3D11_MAPPED_SUBRESOURCE mapped;
    ctx_->Map(constantBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, vp, 64);
    ctx_->Unmap(constantBuffer_.Get(), 0);

    D3D11_VIEWPORT viewport = {0, 0, (float)screenW, (float)screenH, 0, 1};
    ctx_->RSSetViewports(1, &viewport);
    ctx_->OMSetRenderTargets(1, &rtv, nullptr);
    ctx_->OMSetBlendState(blendOff_.Get(), nullptr, 0xFFFFFFFF);
    ctx_->OMSetDepthStencilState(depthOff_.Get(), 0);
    ctx_->RSSetState(rasterState_.Get());

    ctx_->IASetInputLayout(inputLayout_.Get());
    ctx_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    UINT stride = sizeof(CubeVertex), offset = 0;
    ctx_->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride, &offset);

    ctx_->VSSetShader(vs_.Get(), nullptr, 0);
    ctx_->PSSetShader(ps_.Get(), nullptr, 0);
    ctx_->VSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
    ctx_->PSSetSamplers(0, 1, sampler_.GetAddressOf());
    ctx_->PSSetShaderResources(0, 6, faces);

    ctx_->Draw(36, 0);

    ID3D11ShaderResourceView* nullSRVs[6] = {};
    ctx_->PSSetShaderResources(0, 6, nullSRVs);
}
