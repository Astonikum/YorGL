#pragma once

#include <cstdint>
#include <string_view>

namespace yorgl {

enum class BackendKind {
    Null = 0,
    Dx11 = 1,
};

enum class PresentMode {
    VSync = 0,
    Immediate = 1,
};

struct BackendCapabilities {
    BackendKind backend = BackendKind::Null;
    int featureLevelMajor = 0;
    int featureLevelMinor = 0;
    int maxTextureSize = 0;
    bool presentVSync = false;
    bool presentImmediate = false;
    bool presentTearing = false;
};

struct SwapChainOptions {
    int width = 0;
    int height = 0;
    int bufferCount = 2;
    PresentMode presentMode = PresentMode::VSync;
    bool allowTearing = true;
};

struct RenderDiagnostics {
    int lastResizeResult = 0;
    int lastPresentResult = 0;
    int deviceRemovedReason = 0;
};

class Backend {
public:
    virtual ~Backend() = default;
    virtual std::string_view name() const = 0;
    virtual bool init() = 0;
    virtual void shutdown() = 0;
    virtual bool createSwapChain(std::int64_t windowHandle, const SwapChainOptions& options) = 0;
    bool createSwapChain(std::int64_t windowHandle, int width, int height) {
        SwapChainOptions options;
        options.width = width;
        options.height = height;
        return createSwapChain(windowHandle, options);
    }
    virtual void resize(int width, int height) = 0;
    virtual void beginFrame() = 0;
    virtual void setViewport(float x, float y, float width, float height) = 0;
    virtual void clearColor(float r, float g, float b, float a) = 0;
    virtual void clearDepth(float depth) = 0;
    virtual BackendCapabilities capabilities() const = 0;
    virtual RenderDiagnostics diagnostics() const { return {}; }
    virtual void setPresentMode(PresentMode) {}
    virtual void endFrame() = 0;

    virtual std::int64_t createTexture(int, int, const std::uint8_t*, int) { return 0; }
    virtual void destroyTexture(std::int64_t) {}

    virtual void guiBegin(int, int) {}
    virtual void guiDrawQuad(float, float, float, float, float, float, float, float, float, float, float, float) {}
    virtual void guiDrawGradientQuad(float, float, float, float, const float*) {}
    virtual void guiSetTexture(std::int64_t) {}
    virtual void guiSetScissor(float, float, float, float) {}
    virtual void guiClearScissor() {}
    virtual void guiSetSdfMode(bool) {}
    virtual void guiSetSdfParams(float, float, float) {}
    virtual void guiBlurRect(float, float, float, float, int) {}
    virtual void guiEnd() {}

    virtual void panoramaRender(const std::int64_t*, float, int, int) {}

    virtual void worldUploadMesh(const float*, int) {}
    virtual void worldUploadSection(std::int64_t, int, int, int, const float*, int) {}
    virtual void worldUploadSectionLayer(std::int64_t, int, int, int, int, const float*, int) {}
    virtual void worldRemoveSection(std::int64_t) {}
    virtual void worldClearSections() {}
    virtual void worldSetTexture(std::int64_t) {}
    virtual void worldSetSkyColor(float, float, float) {}
    virtual void worldRender(float, float, float, float, float, float, float, float, int, int) {}

    virtual std::int64_t sdfFontCreate(const std::uint8_t*, int, float) { return 0; }
    virtual void sdfFontDestroy(std::int64_t) {}
    virtual std::int64_t sdfFontAtlas(std::int64_t) { return 0; }
    virtual bool sdfFontMetrics(std::int64_t, float*) { return false; }
    virtual bool sdfFontGlyph(std::int64_t, int, float*) { return false; }
    virtual float sdfFontKerning(std::int64_t, int, int) { return 0.0f; }
    virtual float sdfFontTextWidth(std::int64_t, const char*, int, float) { return 0.0f; }
    virtual float sdfFontLineHeight(std::int64_t, float) { return 0.0f; }
    virtual void sdfFontDrawText(std::int64_t, const char*, int, float, float, float, float, float, float, float, float, bool) {}
};

} // namespace yorgl
