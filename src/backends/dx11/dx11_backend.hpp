#pragma once

#include "../../yorgl/backend.hpp"

#if defined(_WIN32)
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

namespace yorgl {

class Dx11Backend final : public Backend {
public:
    ~Dx11Backend() override;

    std::string_view name() const override { return "dx11"; }
    bool init() override;
    void shutdown() override;
    bool createSwapChain(std::int64_t windowHandle, int width, int height) override;
    void resize(int width, int height) override;
    void beginFrame() override;
    void clearColor(float r, float g, float b, float a) override;
    void endFrame() override;

private:
    void createRenderTarget();

    Microsoft::WRL::ComPtr<ID3D11Device> device_;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain_;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv_;
    int width_ = 0;
    int height_ = 0;
};

} // namespace yorgl
#endif
