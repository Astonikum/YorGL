#pragma once

#include "../../yorgl/backend.hpp"

#if defined(_WIN32)
#include <d3d11.h>
#include <dxgi1_5.h>
#include <wrl/client.h>
#include "modules/cubemap_renderer.h"
#include "modules/gui_renderer.h"
#include "modules/sdf_font.h"
#include "modules/world_renderer.h"

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
    void setViewport(float x, float y, float width, float height) override;
    void clearColor(float r, float g, float b, float a) override;
    void clearDepth(float depth) override;
    BackendCapabilities capabilities() const override;
    void setPresentMode(PresentMode mode) override;
    void endFrame() override;

    std::int64_t createTexture(int width, int height, const std::uint8_t* rgba, int byteCount) override;
    void destroyTexture(std::int64_t texture) override;

    void guiBegin(int width, int height) override;
    void guiDrawQuad(float x, float y, float w, float h, float u0, float v0, float u1, float v1, float r, float g, float b, float a) override;
    void guiDrawGradientQuad(float x, float y, float w, float h, const float* rgba16) override;
    void guiSetTexture(std::int64_t texture) override;
    void guiSetScissor(float x, float y, float w, float h) override;
    void guiClearScissor() override;
    void guiSetSdfMode(bool enabled) override;
    void guiSetSdfParams(float edge, float softness, float weightBias) override;
    void guiBlurRect(float x, float y, float w, float h, int passes) override;
    void guiEnd() override;

    void panoramaRender(const std::int64_t* faces, float angle, int width, int height) override;

    void worldUploadMesh(const float* vertices, int floatCount) override;
    void worldUploadSection(std::int64_t sectionId, int x, int y, int z, const float* vertices, int floatCount) override;
    void worldUploadSectionLayer(std::int64_t sectionId, int x, int y, int z, int layer, const float* vertices, int floatCount) override;
    void worldRemoveSection(std::int64_t sectionId) override;
    void worldClearSections() override;
    void worldSetTexture(std::int64_t texture) override;
    void worldSetSkyColor(float r, float g, float b) override;
    void worldRender(float cameraX, float cameraY, float cameraZ, float dirX, float dirY, float dirZ, float fovYDegrees, float farPlane, int width, int height) override;

    std::int64_t sdfFontCreate(const std::uint8_t* ttfData, int byteCount, float fontSize) override;
    void sdfFontDestroy(std::int64_t font) override;
    std::int64_t sdfFontAtlas(std::int64_t font) override;
    bool sdfFontMetrics(std::int64_t font, float* out3) override;
    bool sdfFontGlyph(std::int64_t font, int codepoint, float* out9) override;
    float sdfFontKerning(std::int64_t font, int leftCodepoint, int rightCodepoint) override;
    float sdfFontTextWidth(std::int64_t font, const char* utf8, int byteCount, float scale) override;
    float sdfFontLineHeight(std::int64_t font, float scale) override;
    void sdfFontDrawText(std::int64_t font, const char* utf8, int byteCount, float x, float y, float scale, float r, float g, float b, float a, float weight, bool shadow) override;

private:
    void updateTearingSupport();
    void createRenderTarget();

    Microsoft::WRL::ComPtr<ID3D11Device> device_;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain_;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv_;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> dsv_;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> depthBuffer_;
    GuiRenderer gui_;
    CubemapRenderer panorama_;
    WorldRenderer world_;
    int width_ = 0;
    int height_ = 0;
    D3D_FEATURE_LEVEL featureLevel_ = D3D_FEATURE_LEVEL_11_0;
    PresentMode presentMode_ = PresentMode::VSync;
    bool allowTearing_ = false;
};

} // namespace yorgl
#endif
