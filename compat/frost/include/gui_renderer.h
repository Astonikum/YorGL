#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <vector>
#include <cstdint>

using Microsoft::WRL::ComPtr;

struct GuiVertex {
    float x, y;
    float u, v;
    float r, g, b, a;
};

class GuiRenderer {
public:
    bool init(ID3D11Device* device, ID3D11DeviceContext* ctx);
    void shutdown();

    void begin(int screenWidth, int screenHeight);
    void drawQuad(float x, float y, float w, float h,
                  float u0, float v0, float u1, float v1,
                  float r, float g, float b, float a);
    void drawGradientQuad(float x, float y, float w, float h,
                          float tlR, float tlG, float tlB, float tlA,
                          float trR, float trG, float trB, float trA,
                          float brR, float brG, float brB, float brA,
                          float blR, float blG, float blB, float blA);
    void drawQuadTextured(float x, float y, float w, float h,
                          float u0, float v0, float u1, float v1,
                          float r, float g, float b, float a,
                          ID3D11ShaderResourceView* srv);
    void flush();
    void end();

    void setTexture(ID3D11ShaderResourceView* srv);
    void setSdfMode(bool sdf);
    void setSdfParams(float edge, float softness, float weightBias);
    void setScissor(float x, float y, float w, float h);
    void clearScissor();

    // Blur: capture current backbuffer region, blur it, draw back as quad
    void blurRect(float x, float y, float w, float h, int passes, ID3D11RenderTargetView* mainRTV);

private:
    void createShaders();
    void createBuffers();
    void createStates();
    void createBlurResources();
    void updateProjection(int w, int h);

    ID3D11Device* device_ = nullptr;
    ID3D11DeviceContext* ctx_ = nullptr;

    ComPtr<ID3D11VertexShader> vs_;
    ComPtr<ID3D11PixelShader> ps_;
    ComPtr<ID3D11PixelShader> psSdf_;
    ComPtr<ID3D11SamplerState> samplerLinear_;
    ComPtr<ID3D11InputLayout> inputLayout_;
    ComPtr<ID3D11Buffer> vertexBuffer_;
    ComPtr<ID3D11Buffer> constantBuffer_;
    ComPtr<ID3D11Buffer> sdfConstantBuffer_;
    ComPtr<ID3D11BlendState> blendState_;
    ComPtr<ID3D11SamplerState> sampler_;
    ComPtr<ID3D11RasterizerState> rasterState_;
    ComPtr<ID3D11RasterizerState> rasterStateScissor_;
    ComPtr<ID3D11DepthStencilState> depthOff_;
    ComPtr<ID3D11ShaderResourceView> whiteTexSRV_;
    ComPtr<ID3D11Texture2D> whiteTex_;

    ID3D11ShaderResourceView* currentSRV_ = nullptr;
    bool sdfMode_ = false;
    float sdfEdge_ = 180.0f / 255.0f;
    float sdfSoftness_ = 0.85f;
    float sdfWeightBias_ = 0.0f;

    // Blur resources
    ComPtr<ID3D11PixelShader> psBlurH_;
    ComPtr<ID3D11PixelShader> psBlurV_;
    ComPtr<ID3D11VertexShader> vsFullscreen_;
    ComPtr<ID3D11Buffer> blurCB_;
    ComPtr<ID3D11Texture2D> blurTexA_;
    ComPtr<ID3D11Texture2D> blurTexB_;
    ComPtr<ID3D11RenderTargetView> blurRtvA_;
    ComPtr<ID3D11RenderTargetView> blurRtvB_;
    ComPtr<ID3D11ShaderResourceView> blurSrvA_;
    ComPtr<ID3D11ShaderResourceView> blurSrvB_;
    int blurW_ = 0, blurH_ = 0;

    static constexpr int MAX_QUADS = 4096;
    static constexpr int MAX_VERTICES = MAX_QUADS * 6;
    std::vector<GuiVertex> vertices_;
    int screenW_ = 0, screenH_ = 0;
};
