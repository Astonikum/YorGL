#include "dx11_backend.hpp"

#if defined(_WIN32)

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
    return SUCCEEDED(D3D11CreateDevice(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
        levels, 2, D3D11_SDK_VERSION,
        device_.GetAddressOf(), &selected, context_.GetAddressOf()));
}

void Dx11Backend::shutdown() {
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

    if (FAILED(factory->CreateSwapChainForHwnd(
            device_.Get(), reinterpret_cast<HWND>(windowHandle), &desc, nullptr, nullptr, swapChain_.GetAddressOf()))) {
        return false;
    }
    createRenderTarget();
    return rtv_ != nullptr;
}

void Dx11Backend::resize(int width, int height) {
    if (!swapChain_) return;
    width_ = width;
    height_ = height;
    rtv_.Reset();
    if (SUCCEEDED(swapChain_->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0))) {
        createRenderTarget();
    }
}

void Dx11Backend::beginFrame() {
    if (rtv_) context_->OMSetRenderTargets(1, rtv_.GetAddressOf(), nullptr);
}

void Dx11Backend::clearColor(float r, float g, float b, float a) {
    if (!rtv_) return;
    float color[] = {r, g, b, a};
    context_->ClearRenderTargetView(rtv_.Get(), color);
}

void Dx11Backend::endFrame() {
    if (swapChain_) swapChain_->Present(1, 0);
}

void Dx11Backend::createRenderTarget() {
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    if (FAILED(swapChain_->GetBuffer(0, IID_PPV_ARGS(&backBuffer)))) return;
    device_->CreateRenderTargetView(backBuffer.Get(), nullptr, rtv_.GetAddressOf());
}

} // namespace yorgl

#endif
