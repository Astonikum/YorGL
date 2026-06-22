#pragma once

#include "render_backend.h"
#include "gui_renderer.h"
#include "cubemap_renderer.h"
#include "world_renderer.h"
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class DX11Backend : public IRenderBackend {
public:
    DX11Backend() = default;
    ~DX11Backend() override;

    bool init() override;
    void shutdown() override;
    void beginFrame() override;
    void endFrame() override;

    void createSwapChain(int64_t windowHandle, int width, int height) override;
    void resizeSwapChain(int width, int height) override;

    int64_t createShader(const uint8_t* vsSrc, int vsLen, const uint8_t* psSrc, int psLen) override;
    void destroyShader(int64_t handle) override;

    int64_t createBuffer(int type, const uint8_t* data, int dataLen, int usage) override;
    void destroyBuffer(int64_t handle) override;

    void draw(int vertexCount, int startVertex) override;
    void drawIndexed(int indexCount, int startIndex, int baseVertex) override;

    void setViewport(float x, float y, float width, float height) override;
    void clearColor(float r, float g, float b, float a) override;
    void clearDepth(float depth) override;

    GuiRenderer& gui() { return gui_; }
    CubemapRenderer& panorama() { return panorama_; }
    WorldRenderer& world() { return world_; }
    ID3D11Device* getDevice() { return device_.Get(); }
    ID3D11DeviceContext* getContext() { return context_.Get(); }
    ID3D11RenderTargetView* getRTV() { return rtv_.Get(); }
    ID3D11DepthStencilView* getDSV() { return dsv_.Get(); }

private:
    void createRenderTargetView();

    ComPtr<ID3D11Device> device_;
    ComPtr<ID3D11DeviceContext> context_;
    ComPtr<IDXGISwapChain1> swapChain_;
    ComPtr<ID3D11RenderTargetView> rtv_;
    ComPtr<ID3D11DepthStencilView> dsv_;
    ComPtr<ID3D11Texture2D> depthBuffer_;

    GuiRenderer gui_;
    CubemapRenderer panorama_;
    WorldRenderer world_;
    int width_ = 0;
    int height_ = 0;
    bool initialized_ = false;
};
