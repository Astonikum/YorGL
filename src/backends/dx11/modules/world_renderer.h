#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <unordered_map>
#include "../../../yorgl/backend.hpp"

using Microsoft::WRL::ComPtr;

class WorldRenderer {
public:
    bool init(ID3D11Device* device, ID3D11DeviceContext* ctx);
    void shutdown();
    void uploadMesh(const float* data, int floatCount);
    void uploadSection(long long sectionId, int sectionX, int sectionY, int sectionZ, const float* data, int floatCount);
    void uploadSectionLayer(long long sectionId, int sectionX, int sectionY, int sectionZ, int layer, const float* data, int floatCount);
    void uploadSectionLayerTextured(long long sectionId, int sectionX, int sectionY, int sectionZ, int layer, ID3D11ShaderResourceView* texture, const float* data, int floatCount);
    void removeSection(long long sectionId);
    void clearSections();
    void setTexture(ID3D11ShaderResourceView* texture);
    void setTextureFilter(yorgl::TextureFilter filter);
    void setSkyColor(float r, float g, float b);
    void render(float cameraX, float cameraY, float cameraZ,
                float dirX, float dirY, float dirZ, float fovYDegrees, float farPlane, int screenW, int screenH,
                ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv);

private:
    void createShaders();
    void ensureVertexCapacity(int vertexCount);
    void createStates();
    struct CameraConstants {
        float mvp[16];
        float useTexture;
        float cameraPos[3];
        float skyColor[4];
        float fogParams[4];
    };
    void drawBuffer(ID3D11Buffer* buffer, int vertexCount, ID3D11ShaderResourceView* texture, bool textureOverride, CameraConstants& constants, bool& textureEnabled);

    struct SectionMesh {
        ComPtr<ID3D11Buffer> opaqueBuffer;
        ComPtr<ID3D11Buffer> translucentBuffer;
        ID3D11ShaderResourceView* opaqueTexture = nullptr;
        ID3D11ShaderResourceView* translucentTexture = nullptr;
        bool opaqueTextureOverride = false;
        bool translucentTextureOverride = false;
        int opaqueVertexCount = 0;
        int translucentVertexCount = 0;
        float centerX = 0.0f;
        float centerY = 0.0f;
        float centerZ = 0.0f;
    };
    bool sectionVisible(const SectionMesh& mesh, const float* mvp) const;

    ID3D11Device* device_ = nullptr;
    ID3D11DeviceContext* ctx_ = nullptr;

    ComPtr<ID3D11VertexShader> vs_;
    ComPtr<ID3D11PixelShader> ps_;
    ComPtr<ID3D11InputLayout> inputLayout_;
    ComPtr<ID3D11Buffer> vertexBuffer_;
    ComPtr<ID3D11Buffer> constantBuffer_;
    ComPtr<ID3D11RasterizerState> rasterState_;
    ComPtr<ID3D11DepthStencilState> depthOn_;
    ComPtr<ID3D11DepthStencilState> depthRead_;
    ComPtr<ID3D11DepthStencilState> depthOff_;
    ComPtr<ID3D11BlendState> blendOff_;
    ComPtr<ID3D11BlendState> blendOn_;
    ComPtr<ID3D11SamplerState> samplerPoint_;
    ComPtr<ID3D11SamplerState> samplerLinear_;

    int vertexCapacity_ = 0;
    int vertexCount_ = 0;
    std::unordered_map<long long, SectionMesh> sections_;
    float skyColor_[4] = {0.10f, 0.13f, 0.16f, 1.0f};
    ID3D11ShaderResourceView* texture_ = nullptr;
    yorgl::TextureFilter textureFilter_ = yorgl::TextureFilter::Nearest;
    bool initialized_ = false;
};
