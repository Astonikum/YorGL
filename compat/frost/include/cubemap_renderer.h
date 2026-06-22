#pragma once

#include <d3d11.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class CubemapRenderer {
public:
    bool init(ID3D11Device* device, ID3D11DeviceContext* ctx);
    void shutdown();
    void render(ID3D11ShaderResourceView* faces[6], float rotationAngle,
                int screenW, int screenH, ID3D11RenderTargetView* rtv);

private:
    void createShaders();
    void createGeometry();
    void createStates();

    ID3D11Device* device_ = nullptr;
    ID3D11DeviceContext* ctx_ = nullptr;

    ComPtr<ID3D11VertexShader> vs_;
    ComPtr<ID3D11PixelShader> ps_;
    ComPtr<ID3D11InputLayout> inputLayout_;
    ComPtr<ID3D11Buffer> vertexBuffer_;
    ComPtr<ID3D11Buffer> constantBuffer_;
    ComPtr<ID3D11SamplerState> sampler_;
    ComPtr<ID3D11RasterizerState> rasterState_;
    ComPtr<ID3D11DepthStencilState> depthOff_;
    ComPtr<ID3D11BlendState> blendOff_;

    bool initialized_ = false;
};
