#include "dx11_backend.h"
#include "frost_log.h"
#include <d3dcompiler.h>

DX11Backend::~DX11Backend() {
    if (initialized_) shutdown();
}

bool DX11Backend::init() {
    LOG_INFO("Creating D3D11 device...");

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };

    UINT flags = 0;
#ifdef _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
    LOG_DEBUG("D3D11 debug layer enabled");
#endif

    D3D_FEATURE_LEVEL achievedLevel;
    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        flags,
        featureLevels, 2,
        D3D11_SDK_VERSION,
        device_.GetAddressOf(),
        &achievedLevel,
        context_.GetAddressOf()
    );

    if (FAILED(hr)) {
        LOG_ERROR("D3D11CreateDevice failed: 0x%08X", hr);
        return false;
    }

    LOG_INFO("D3D11 device created successfully (feature level: 0x%X)", achievedLevel);
    initialized_ = true;
    return true;
}

void DX11Backend::shutdown() {
    LOG_INFO("Shutting down DX11 backend...");
    gui_.shutdown();
    panorama_.shutdown();
    world_.shutdown();
    dsv_.Reset();
    depthBuffer_.Reset();
    rtv_.Reset();
    swapChain_.Reset();
    context_.Reset();
    device_.Reset();
    initialized_ = false;
    LOG_INFO("DX11 backend shutdown complete");
}

void DX11Backend::beginFrame() {
    if (rtv_) {
        context_->OMSetRenderTargets(1, rtv_.GetAddressOf(), dsv_.Get());
    }
}

void DX11Backend::endFrame() {
    if (swapChain_) {
        HRESULT hr = swapChain_->Present(1, 0);
        if (FAILED(hr)) {
            LOG_ERROR("Present failed: 0x%08X", hr);
        }
    }
}

void DX11Backend::createSwapChain(int64_t windowHandle, int width, int height) {
    LOG_INFO("Creating swap chain (%dx%d, HWND=0x%llX)", width, height, windowHandle);
    width_ = width;
    height_ = height;

    ComPtr<IDXGIDevice> dxgiDevice;
    device_.As(&dxgiDevice);
    ComPtr<IDXGIAdapter> adapter;
    dxgiDevice->GetAdapter(&adapter);
    ComPtr<IDXGIFactory2> factory;
    adapter->GetParent(IID_PPV_ARGS(&factory));

    DXGI_SWAP_CHAIN_DESC1 desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 2;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    HRESULT hr = factory->CreateSwapChainForHwnd(
        device_.Get(), (HWND)windowHandle, &desc, nullptr, nullptr,
        swapChain_.GetAddressOf()
    );

    if (FAILED(hr)) {
        LOG_ERROR("CreateSwapChain failed: 0x%08X", hr);
        return;
    }

    createRenderTargetView();
    gui_.init(device_.Get(), context_.Get());
    panorama_.init(device_.Get(), context_.Get());
    world_.init(device_.Get(), context_.Get());
    LOG_INFO("Swap chain created successfully");
}

void DX11Backend::resizeSwapChain(int width, int height) {
    LOG_INFO("Resizing swap chain to %dx%d", width, height);
    width_ = width;
    height_ = height;
    rtv_.Reset();
    dsv_.Reset();
    depthBuffer_.Reset();

    HRESULT hr = swapChain_->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(hr)) {
        LOG_ERROR("ResizeBuffers failed: 0x%08X", hr);
        return;
    }
    createRenderTargetView();
    LOG_INFO("Swap chain resized successfully");
}

void DX11Backend::createRenderTargetView() {
    ComPtr<ID3D11Texture2D> backBuffer;
    swapChain_->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    device_->CreateRenderTargetView(backBuffer.Get(), nullptr, rtv_.GetAddressOf());

    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = width_;
    depthDesc.Height = height_;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    device_->CreateTexture2D(&depthDesc, nullptr, depthBuffer_.GetAddressOf());
    device_->CreateDepthStencilView(depthBuffer_.Get(), nullptr, dsv_.GetAddressOf());
    LOG_DEBUG("Render target and depth buffer created (%dx%d)", width_, height_);
}

int64_t DX11Backend::createShader(const uint8_t* vsSrc, int vsLen, const uint8_t* psSrc, int psLen) {
    LOG_DEBUG("Compiling shader (VS: %d bytes, PS: %d bytes)", vsLen, psLen);
    // TODO: compile from HLSL bytecode, store in handle map
    return 0;
}

void DX11Backend::destroyShader(int64_t handle) {
    LOG_DEBUG("Destroying shader handle %lld", handle);
}

int64_t DX11Backend::createBuffer(int type, const uint8_t* data, int dataLen, int usage) {
    LOG_DEBUG("Creating buffer (type=%d, size=%d, usage=%d)", type, dataLen, usage);
    // TODO: implement buffer creation
    return 0;
}

void DX11Backend::destroyBuffer(int64_t handle) {
    LOG_DEBUG("Destroying buffer handle %lld", handle);
}

void DX11Backend::draw(int vertexCount, int startVertex) {
    context_->Draw(vertexCount, startVertex);
}

void DX11Backend::drawIndexed(int indexCount, int startIndex, int baseVertex) {
    context_->DrawIndexed(indexCount, startIndex, baseVertex);
}

void DX11Backend::setViewport(float x, float y, float width, float height) {
    D3D11_VIEWPORT vp = { x, y, width, height, 0.0f, 1.0f };
    context_->RSSetViewports(1, &vp);
}

void DX11Backend::clearColor(float r, float g, float b, float a) {
    float color[] = { r, g, b, a };
    if (rtv_) context_->ClearRenderTargetView(rtv_.Get(), color);
}

void DX11Backend::clearDepth(float depth) {
    if (dsv_) context_->ClearDepthStencilView(dsv_.Get(), D3D11_CLEAR_DEPTH, depth, 0);
}
