#pragma once

#include <cstdint>
#include <string_view>

namespace yorgl {

enum class BackendKind {
    Null = 0,
    Dx11 = 1,
};

class Backend {
public:
    virtual ~Backend() = default;
    virtual std::string_view name() const = 0;
    virtual bool init() = 0;
    virtual void shutdown() = 0;
    virtual bool createSwapChain(std::int64_t windowHandle, int width, int height) = 0;
    virtual void resize(int width, int height) = 0;
    virtual void beginFrame() = 0;
    virtual void clearColor(float r, float g, float b, float a) = 0;
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
};

} // namespace yorgl
