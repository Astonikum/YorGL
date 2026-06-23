#include "world_renderer.h"
#include "yorgl_log.hpp"
#include <d3dcompiler.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

static const char* WORLD_VS = R"(
struct VSInput {
    float3 pos : POSITION;
    float4 col : COLOR;
    float2 uv : TEXCOORD0;
};
struct PSInput {
    float4 pos : SV_POSITION;
    float3 worldPos : TEXCOORD1;
    float4 col : COLOR;
    float2 uv : TEXCOORD0;
};
cbuffer CameraBuffer : register(b0) {
    row_major float4x4 mvp;
    float useTexture;
    float3 cameraPos;
    float4 skyColor;
    float4 fogParams;
};
PSInput VSMain(VSInput i) {
    PSInput o;
    o.pos = mul(float4(i.pos, 1.0), mvp);
    o.worldPos = i.pos;
    o.col = i.col;
    o.uv = i.uv;
    return o;
}
)";

static const char* WORLD_PS = R"(
Texture2D atlasTex : register(t0);
SamplerState atlasSampler : register(s0);
struct PSInput {
    float4 pos : SV_POSITION;
    float3 worldPos : TEXCOORD1;
    float4 col : COLOR;
    float2 uv : TEXCOORD0;
};
cbuffer CameraBuffer : register(b0) {
    row_major float4x4 mvp;
    float useTexture;
    float3 cameraPos;
    float4 skyColor;
    float4 fogParams;
};
float4 PSMain(PSInput i) : SV_TARGET {
    float4 c;
    if (useTexture > 0.5) {
        c = atlasTex.Sample(atlasSampler, i.uv) * i.col;
        clip(c.a - 0.1);
    } else {
        c = i.col;
    }
    float fog = saturate((distance(i.worldPos, cameraPos) - fogParams.x) / max(1.0, fogParams.y - fogParams.x));
    c.rgb = lerp(c.rgb, skyColor.rgb, fog);
    return c;
}
)";

struct WorldVertex {
    float x, y, z;
    float r, g, b, a;
    float u, v;
};

struct CameraConstants {
    float mvp[16];
    float useTexture;
    float cameraPos[3];
    float skyColor[4];
    float fogParams[4];
};

struct Vec3 {
    float x, y, z;
};

static Vec3 cross(Vec3 a, Vec3 b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

static float dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static Vec3 normalize(Vec3 v) {
    float len = sqrtf(dot(v, v));
    if (len <= 0.0001f) return {0.0f, 0.0f, 1.0f};
    return {v.x / len, v.y / len, v.z / len};
}

static void multiply(const float* a, const float* b, float* out) {
    float r[16];
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            r[row * 4 + col] =
                a[row * 4 + 0] * b[0 * 4 + col] +
                a[row * 4 + 1] * b[1 * 4 + col] +
                a[row * 4 + 2] * b[2 * 4 + col] +
                a[row * 4 + 3] * b[3 * 4 + col];
        }
    }
    memcpy(out, r, sizeof(r));
}

static void lookToLH(Vec3 eye, Vec3 dir, Vec3 up, float* m) {
    Vec3 z = normalize(dir);
    Vec3 x = normalize(cross(up, z));
    Vec3 y = cross(z, x);

    m[0] = x.x; m[1] = y.x; m[2] = z.x; m[3] = 0.0f;
    m[4] = x.y; m[5] = y.y; m[6] = z.y; m[7] = 0.0f;
    m[8] = x.z; m[9] = y.z; m[10] = z.z; m[11] = 0.0f;
    m[12] = -dot(x, eye); m[13] = -dot(y, eye); m[14] = -dot(z, eye); m[15] = 1.0f;
}

static void perspectiveFovLH(float fovY, float aspect, float zn, float zf, float* m) {
    memset(m, 0, sizeof(float) * 16);
    float yScale = 1.0f / tanf(fovY * 0.5f);
    float xScale = yScale / aspect;
    m[0] = xScale;
    m[5] = yScale;
    m[10] = zf / (zf - zn);
    m[11] = 1.0f;
    m[14] = -zn * zf / (zf - zn);
}

static void transformPoint(const float* m, float x, float y, float z, float* out) {
    out[0] = x * m[0] + y * m[4] + z * m[8] + m[12];
    out[1] = x * m[1] + y * m[5] + z * m[9] + m[13];
    out[2] = x * m[2] + y * m[6] + z * m[10] + m[14];
    out[3] = x * m[3] + y * m[7] + z * m[11] + m[15];
}

bool WorldRenderer::init(ID3D11Device* device, ID3D11DeviceContext* ctx) {
    device_ = device;
    ctx_ = ctx;
    LOG_INFO("WorldRenderer: initializing...");
    createShaders();
    ensureVertexCapacity(4096);
    createStates();
    initialized_ = true;
    LOG_INFO("WorldRenderer: ready");
    return true;
}

void WorldRenderer::shutdown() {
    vs_.Reset(); ps_.Reset(); inputLayout_.Reset();
    vertexBuffer_.Reset(); constantBuffer_.Reset(); rasterState_.Reset();
    depthOn_.Reset(); depthRead_.Reset();
    depthOff_.Reset(); blendOff_.Reset(); blendOn_.Reset(); sampler_.Reset();
    vertexCapacity_ = 0;
    vertexCount_ = 0;
    sections_.clear();
    texture_ = nullptr;
    initialized_ = false;
}

void WorldRenderer::createShaders() {
    ComPtr<ID3DBlob> vsBlob, psBlob, errBlob;

    HRESULT hr = D3DCompile(WORLD_VS, strlen(WORLD_VS), "world_vs",
        nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vsBlob, &errBlob);
    if (FAILED(hr)) {
        LOG_ERROR("World VS compile: %s", errBlob ? (char*)errBlob->GetBufferPointer() : "unknown");
        return;
    }

    hr = D3DCompile(WORLD_PS, strlen(WORLD_PS), "world_ps",
        nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &psBlob, &errBlob);
    if (FAILED(hr)) {
        LOG_ERROR("World PS compile: %s", errBlob ? (char*)errBlob->GetBufferPointer() : "unknown");
        return;
    }

    device_->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vs_);
    device_->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &ps_);

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    device_->CreateInputLayout(layout, 3, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout_);

    D3D11_BUFFER_DESC cbd = {};
    cbd.ByteWidth = sizeof(CameraConstants);
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    device_->CreateBuffer(&cbd, nullptr, &constantBuffer_);
}

void WorldRenderer::ensureVertexCapacity(int vertexCount) {
    if (vertexCount <= vertexCapacity_ && vertexBuffer_) return;
    int nextCapacity = vertexCapacity_ > 0 ? vertexCapacity_ : 4096;
    while (nextCapacity < vertexCount) nextCapacity *= 2;

    D3D11_BUFFER_DESC bd = {};
    bd.ByteWidth = sizeof(WorldVertex) * nextCapacity;
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    device_->CreateBuffer(&bd, nullptr, &vertexBuffer_);
    vertexCapacity_ = nextCapacity;
}

void WorldRenderer::createStates() {
    D3D11_RASTERIZER_DESC rd = {};
    rd.FillMode = D3D11_FILL_SOLID;
    rd.CullMode = D3D11_CULL_NONE;
    rd.DepthClipEnable = TRUE;
    device_->CreateRasterizerState(&rd, &rasterState_);

    D3D11_DEPTH_STENCIL_DESC dsdOn = {};
    dsdOn.DepthEnable = TRUE;
    dsdOn.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsdOn.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    device_->CreateDepthStencilState(&dsdOn, &depthOn_);

    D3D11_DEPTH_STENCIL_DESC dsdRead = {};
    dsdRead.DepthEnable = TRUE;
    dsdRead.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsdRead.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    device_->CreateDepthStencilState(&dsdRead, &depthRead_);

    D3D11_DEPTH_STENCIL_DESC dsd = {};
    dsd.DepthEnable = FALSE;
    device_->CreateDepthStencilState(&dsd, &depthOff_);

    D3D11_BLEND_DESC bd = {};
    bd.RenderTarget[0].BlendEnable = FALSE;
    bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    device_->CreateBlendState(&bd, &blendOff_);

    D3D11_BLEND_DESC bdAlpha = {};
    bdAlpha.RenderTarget[0].BlendEnable = TRUE;
    bdAlpha.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    bdAlpha.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    bdAlpha.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    bdAlpha.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    bdAlpha.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    bdAlpha.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    bdAlpha.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    device_->CreateBlendState(&bdAlpha, &blendOn_);

    D3D11_SAMPLER_DESC sd = {};
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sd.MinLOD = 0;
    sd.MaxLOD = D3D11_FLOAT32_MAX;
    device_->CreateSamplerState(&sd, &sampler_);
}

void WorldRenderer::uploadMesh(const float* data, int floatCount) {
    sections_.clear();
    if (!initialized_ || !data || floatCount <= 0) {
        vertexCount_ = 0;
        return;
    }
    int vertexCount = floatCount / 9;
    ensureVertexCapacity(vertexCount);

    D3D11_MAPPED_SUBRESOURCE mapped;
    HRESULT hr = ctx_->Map(vertexBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (FAILED(hr)) {
        vertexCount_ = 0;
        return;
    }
    memcpy(mapped.pData, data, vertexCount * sizeof(WorldVertex));
    ctx_->Unmap(vertexBuffer_.Get(), 0);
    vertexCount_ = vertexCount;
}

void WorldRenderer::uploadSection(long long sectionId, int sectionX, int sectionY, int sectionZ, const float* data, int floatCount) {
    uploadSectionLayer(sectionId, sectionX, sectionY, sectionZ, 0, data, floatCount);
}

void WorldRenderer::uploadSectionLayer(long long sectionId, int sectionX, int sectionY, int sectionZ, int layer, const float* data, int floatCount) {
    if (!initialized_ || !data || floatCount <= 0) {
        auto found = sections_.find(sectionId);
        if (found != sections_.end()) {
            if (layer == 1) {
                found->second.translucentBuffer.Reset();
                found->second.translucentVertexCount = 0;
            } else {
                found->second.opaqueBuffer.Reset();
                found->second.opaqueVertexCount = 0;
            }
            if (found->second.opaqueVertexCount <= 0 && found->second.translucentVertexCount <= 0) sections_.erase(found);
        }
        return;
    }
    int vertexCount = floatCount / 9;
    D3D11_BUFFER_DESC bd = {};
    bd.ByteWidth = sizeof(WorldVertex) * vertexCount;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initialData = {};
    initialData.pSysMem = data;
    ComPtr<ID3D11Buffer> buffer;
    if (FAILED(device_->CreateBuffer(&bd, &initialData, &buffer))) return;

    SectionMesh mesh = sections_[sectionId];
    mesh.centerX = sectionX * 16.0f + 8.0f;
    mesh.centerY = sectionY * 16.0f + 8.0f;
    mesh.centerZ = sectionZ * 16.0f + 8.0f;
    if (layer == 1) {
        mesh.translucentBuffer = buffer;
        mesh.translucentVertexCount = vertexCount;
    } else {
        mesh.opaqueBuffer = buffer;
        mesh.opaqueVertexCount = vertexCount;
    }
    sections_[sectionId] = mesh;
    vertexCount_ = 0;
}

void WorldRenderer::removeSection(long long sectionId) {
    sections_.erase(sectionId);
}

void WorldRenderer::clearSections() {
    sections_.clear();
    vertexCount_ = 0;
}

void WorldRenderer::setTexture(ID3D11ShaderResourceView* texture) {
    texture_ = texture;
}

void WorldRenderer::setSkyColor(float r, float g, float b) {
    skyColor_[0] = r;
    skyColor_[1] = g;
    skyColor_[2] = b;
    skyColor_[3] = 1.0f;
}

void WorldRenderer::render(float cameraX, float cameraY, float cameraZ,
                           float dirX, float dirY, float dirZ, float fovYDegrees, float farPlane, int screenW, int screenH,
                           ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv) {
    if (!initialized_ || !vs_ || !ps_ || !vertexBuffer_ || !constantBuffer_) return;

    ctx_->ClearRenderTargetView(rtv, skyColor_);
    if (dsv) ctx_->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
    if (vertexCount_ <= 0 && sections_.empty()) return;

    Vec3 eye = {cameraX, cameraY, cameraZ};
    Vec3 dir = {dirX, dirY, dirZ};
    Vec3 up = {0.0f, 1.0f, 0.0f};
    float aspect = screenH > 0 ? (float)screenW / (float)screenH : 1.777f;

    CameraConstants constants;
    float view[16];
    float proj[16];
    lookToLH(eye, dir, up, view);
    perspectiveFovLH(fovYDegrees * 0.01745329251994329577f, aspect, 0.05f, farPlane, proj);
    multiply(view, proj, constants.mvp);
    constants.useTexture = texture_ ? 1.0f : 0.0f;
    constants.cameraPos[0] = cameraX;
    constants.cameraPos[1] = cameraY;
    constants.cameraPos[2] = cameraZ;
    constants.skyColor[0] = skyColor_[0];
    constants.skyColor[1] = skyColor_[1];
    constants.skyColor[2] = skyColor_[2];
    constants.skyColor[3] = 1.0f;
    constants.fogParams[0] = farPlane * 0.72f;
    constants.fogParams[1] = farPlane;
    constants.fogParams[2] = constants.fogParams[3] = 0.0f;
    D3D11_MAPPED_SUBRESOURCE mapped;
    if (SUCCEEDED(ctx_->Map(constantBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        memcpy(mapped.pData, &constants, sizeof(constants));
        ctx_->Unmap(constantBuffer_.Get(), 0);
    }

    D3D11_VIEWPORT viewport = {0, 0, (float)screenW, (float)screenH, 0, 1};
    ctx_->RSSetViewports(1, &viewport);
    ctx_->OMSetRenderTargets(1, &rtv, dsv);
    ctx_->RSSetState(rasterState_.Get());

    ctx_->IASetInputLayout(inputLayout_.Get());
    ctx_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx_->VSSetShader(vs_.Get(), nullptr, 0);
    ctx_->VSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
    ctx_->PSSetShader(ps_.Get(), nullptr, 0);
    ctx_->PSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
    ctx_->PSSetSamplers(0, 1, sampler_.GetAddressOf());
    ctx_->PSSetShaderResources(0, 1, &texture_);
    if (!sections_.empty()) {
        std::vector<SectionMesh*> opaqueList;
        std::vector<SectionMesh*> translucentList;
        opaqueList.reserve(sections_.size());
        translucentList.reserve(sections_.size());
        Vec3 renderDir = normalize({dirX, 0.0f, dirZ});
        float maxDist = farPlane + 32.0f;
        float maxDistSq = maxDist * maxDist;
        for (auto& it : sections_) {
            auto& mesh = it.second;
            float dx = mesh.centerX - cameraX;
            float dy = mesh.centerY - cameraY;
            float dz = mesh.centerZ - cameraZ;
            float distSq = dx * dx + dy * dy + dz * dz;
            if (distSq > maxDistSq) continue;
            if (!sectionVisible(mesh, constants.mvp)) continue;
            float horizontalSq = dx * dx + dz * dz;
            if (horizontalSq > 4096.0f) {
                float invLen = 1.0f / sqrtf(horizontalSq);
                float facing = (dx * invLen) * renderDir.x + (dz * invLen) * renderDir.z;
                if (facing < -0.35f) continue;
            }
            if (mesh.opaqueVertexCount > 0) opaqueList.push_back(&mesh);
            if (mesh.translucentVertexCount > 0) translucentList.push_back(&mesh);
        }
        auto farToNear = [cameraX, cameraY, cameraZ](const SectionMesh* a, const SectionMesh* b) {
            float ax = a->centerX - cameraX, ay = a->centerY - cameraY, az = a->centerZ - cameraZ;
            float bx = b->centerX - cameraX, by = b->centerY - cameraY, bz = b->centerZ - cameraZ;
            return ax * ax + ay * ay + az * az > bx * bx + by * by + bz * bz;
        };
        std::sort(opaqueList.begin(), opaqueList.end(), farToNear);
        std::sort(translucentList.begin(), translucentList.end(), farToNear);

        ctx_->OMSetBlendState(blendOff_.Get(), nullptr, 0xFFFFFFFF);
        ctx_->OMSetDepthStencilState(depthOn_.Get(), 0);
        for (auto* mesh : opaqueList) {
            drawBuffer(mesh->opaqueBuffer.Get(), mesh->opaqueVertexCount);
        }

        ctx_->OMSetBlendState(blendOn_.Get(), nullptr, 0xFFFFFFFF);
        ctx_->OMSetDepthStencilState(depthRead_.Get(), 0);
        for (auto* mesh : translucentList) {
            drawBuffer(mesh->translucentBuffer.Get(), mesh->translucentVertexCount);
        }
    } else {
        ctx_->OMSetBlendState(blendOff_.Get(), nullptr, 0xFFFFFFFF);
        ctx_->OMSetDepthStencilState(depthOn_.Get(), 0);
        drawBuffer(vertexBuffer_.Get(), vertexCount_);
    }
    ID3D11ShaderResourceView* nullSrv = nullptr;
    ctx_->PSSetShaderResources(0, 1, &nullSrv);
}

void WorldRenderer::drawBuffer(ID3D11Buffer* buffer, int vertexCount) {
    if (!buffer || vertexCount <= 0) return;
    UINT stride = sizeof(WorldVertex), offset = 0;
    ctx_->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
    ctx_->Draw((UINT)vertexCount, 0);
}

bool WorldRenderer::sectionVisible(const SectionMesh& mesh, const float* mvp) const {
    int outsideLeft = 0, outsideRight = 0, outsideBottom = 0, outsideTop = 0, outsideNear = 0, outsideFar = 0;
    for (int ix = -1; ix <= 1; ix += 2) {
        for (int iy = -1; iy <= 1; iy += 2) {
            for (int iz = -1; iz <= 1; iz += 2) {
                float c[4];
                transformPoint(mvp, mesh.centerX + ix * 8.0f, mesh.centerY + iy * 8.0f, mesh.centerZ + iz * 8.0f, c);
                if (c[0] < -c[3]) ++outsideLeft;
                if (c[0] > c[3]) ++outsideRight;
                if (c[1] < -c[3]) ++outsideBottom;
                if (c[1] > c[3]) ++outsideTop;
                if (c[2] < 0.0f) ++outsideNear;
                if (c[2] > c[3]) ++outsideFar;
            }
        }
    }
    return outsideLeft < 8 && outsideRight < 8 && outsideBottom < 8 && outsideTop < 8 && outsideNear < 8 && outsideFar < 8;
}
