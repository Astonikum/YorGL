#include "api.h"
#include "renderer.hpp"

#include <cmath>
#include <cstdint>
#include <limits>
#include <string>

using yorgl::BackendKind;
using yorgl::PresentMode;
using yorgl::Renderer;
using yorgl::SwapChainOptions;
using yorgl::TextureFilter;

thread_local YorGLResult g_lastError = YORGL_RESULT_OK;

static bool fail(YorGLResult error) {
    g_lastError = error;
    return false;
}

static void succeed() {
    g_lastError = YORGL_RESULT_OK;
}

struct YorGLRenderer {
    Renderer renderer;
};

static bool toBackend(YorGLBackendKind backend, BackendKind& result) {
    switch (backend) {
        case YORGL_BACKEND_DX11: result = BackendKind::Dx11; return true;
        case YORGL_BACKEND_NULL: result = BackendKind::Null; return true;
        default: return false;
    }
}

static Renderer* native(YorGLRenderer* renderer) {
    if (!renderer) {
        fail(YORGL_RESULT_INVALID_HANDLE);
        return nullptr;
    }
    if (!renderer->renderer.valid()) {
        fail(YORGL_RESULT_NOT_READY);
        return nullptr;
    }
    return &renderer->renderer;
}

static yorgl::Backend* backend(YorGLRenderer* renderer) {
    auto* instance = native(renderer);
    return instance ? &instance->backend() : nullptr;
}

static bool validDimensions(int width, int height) {
    return width > 0 && height > 0;
}

static bool validPixelData(int width, int height, const std::uint8_t* rgba, int byteCount) {
    if (!validDimensions(width, height) || !rgba || byteCount < 0) return false;
    const auto required = static_cast<std::uint64_t>(width) * static_cast<std::uint64_t>(height) * 4U;
    return required <= static_cast<std::uint64_t>(std::numeric_limits<int>::max()) &&
        static_cast<std::uint64_t>(byteCount) >= required;
}

static bool validFloatData(const float* data, int floatCount) {
    constexpr int WorldVertexStride = 9;
    return data && floatCount > 0 && floatCount % WorldVertexStride == 0;
}

static bool validFinite(float value) {
    return std::isfinite(value);
}

static bool validPresentMode(YorGLPresentMode mode) {
    return mode == YORGL_PRESENT_VSYNC || mode == YORGL_PRESENT_IMMEDIATE;
}

static bool requireTexture(const YorGLRenderer* renderer, std::int64_t texture, bool allowNull = false) {
    if (allowNull && texture == 0) return true;
    if (!renderer || !renderer->renderer.ownsTexture(texture)) {
        return fail(YORGL_RESULT_INVALID_HANDLE);
    }
    return true;
}

static bool requireStandaloneTexture(const YorGLRenderer* renderer, std::int64_t texture) {
    if (!renderer || !renderer->renderer.ownsStandaloneTexture(texture)) {
        return fail(YORGL_RESULT_INVALID_HANDLE);
    }
    return true;
}

static bool requireFont(const YorGLRenderer* renderer, std::int64_t font) {
    if (!renderer || !renderer->renderer.ownsFont(font)) {
        return fail(YORGL_RESULT_INVALID_HANDLE);
    }
    return true;
}

static PresentMode toPresentMode(YorGLPresentMode mode) {
    switch (mode) {
        case YORGL_PRESENT_IMMEDIATE: return PresentMode::Immediate;
        case YORGL_PRESENT_VSYNC:
        default: return PresentMode::VSync;
    }
}

static TextureFilter toTextureFilter(YorGLTextureFilter filter) {
    switch (filter) {
        case YORGL_TEXTURE_FILTER_LINEAR: return TextureFilter::Linear;
        case YORGL_TEXTURE_FILTER_NEAREST:
        default: return TextureFilter::Nearest;
    }
}

static SwapChainOptions toSwapChainOptions(const YorGLSwapChainOptions* source) {
    SwapChainOptions options;
    if (!source) return options;
    options.width = source->width;
    options.height = source->height;
    options.bufferCount = source->bufferCount;
    options.presentMode = toPresentMode(source->presentMode);
    options.allowTearing = source->allowTearing != 0;
    return options;
}

extern "C" YorGLResult yorglGetLastError(void) {
    return g_lastError;
}

extern "C" void yorglClearLastError(void) {
    succeed();
}

YorGLRenderer* yorglCreate(YorGLBackendKind backend) {
    BackendKind kind{};
    if (!toBackend(backend, kind)) {
        fail(YORGL_RESULT_INVALID_ARGUMENT);
        return nullptr;
    }
    auto nativeBackend = yorgl::createBackend(kind);
    if (!nativeBackend) {
        fail(YORGL_RESULT_BACKEND_FAILURE);
        return nullptr;
    }
    auto* result = new YorGLRenderer{Renderer(std::move(nativeBackend))};
    if (!result->renderer.valid()) {
        delete result;
        fail(YORGL_RESULT_BACKEND_FAILURE);
        return nullptr;
    }
    succeed();
    return result;
}

void yorglDestroy(YorGLRenderer* renderer) {
    delete renderer;
    succeed();
}

int yorglIsValid(YorGLRenderer* renderer) {
    return renderer && renderer->renderer.valid();
}

const char* yorglBackendName(YorGLRenderer* renderer) {
    auto* b = backend(renderer);
    if (!b) return "invalid";
    static thread_local std::string name;
    name.assign(b->name());
    succeed();
    return name.c_str();
}

int yorglGetCapabilities(YorGLRenderer* renderer, YorGLCapabilities* outCapabilities) {
    auto* b = backend(renderer);
    if (!b || !outCapabilities) {
        if (b) fail(YORGL_RESULT_INVALID_ARGUMENT);
        return 0;
    }
    const auto caps = b->capabilities();
    outCapabilities->backend = static_cast<int>(caps.backend);
    outCapabilities->featureLevelMajor = caps.featureLevelMajor;
    outCapabilities->featureLevelMinor = caps.featureLevelMinor;
    outCapabilities->maxTextureSize = caps.maxTextureSize;
    outCapabilities->presentVSync = caps.presentVSync ? 1 : 0;
    outCapabilities->presentImmediate = caps.presentImmediate ? 1 : 0;
    outCapabilities->presentTearing = caps.presentTearing ? 1 : 0;
    succeed();
    return 1;
}

int yorglGetDiagnostics(YorGLRenderer* renderer, YorGLRenderDiagnostics* outDiagnostics) {
    auto* b = backend(renderer);
    if (!b || !outDiagnostics) {
        if (b) fail(YORGL_RESULT_INVALID_ARGUMENT);
        return 0;
    }
    const auto diagnostics = b->diagnostics();
    outDiagnostics->lastResizeResult = diagnostics.lastResizeResult;
    outDiagnostics->lastPresentResult = diagnostics.lastPresentResult;
    outDiagnostics->deviceRemovedReason = diagnostics.deviceRemovedReason;
    succeed();
    return 1;
}

int yorglCreateSwapChain(YorGLRenderer* renderer, int64_t windowHandle, int width, int height) {
    auto* b = backend(renderer);
    if (!b) return 0;
    if (!validDimensions(width, height)) {
        fail(YORGL_RESULT_INVALID_ARGUMENT);
        return 0;
    }
    const bool created = b->createSwapChain(windowHandle, width, height);
    if (created) succeed(); else fail(YORGL_RESULT_BACKEND_FAILURE);
    return created ? 1 : 0;
}

int yorglCreateSwapChainWithOptions(YorGLRenderer* renderer, int64_t windowHandle, const YorGLSwapChainOptions* options) {
    auto* b = backend(renderer);
    if (!b) return 0;
    if (!options || !validDimensions(options->width, options->height) ||
        options->bufferCount < 2 || options->bufferCount > 3 || !validPresentMode(options->presentMode)) {
        fail(YORGL_RESULT_INVALID_ARGUMENT);
        return 0;
    }
    if (b->capabilities().backend == BackendKind::Dx11 && windowHandle == 0) {
        fail(YORGL_RESULT_INVALID_ARGUMENT);
        return 0;
    }
    const bool created = b->createSwapChain(windowHandle, toSwapChainOptions(options));
    if (created) succeed(); else fail(YORGL_RESULT_BACKEND_FAILURE);
    return created ? 1 : 0;
}

void yorglResize(YorGLRenderer* renderer, int width, int height) {
    if (auto* b = backend(renderer)) {
        if (!validDimensions(width, height)) {
            fail(YORGL_RESULT_INVALID_ARGUMENT);
            return;
        }
        b->resize(width, height);
        succeed();
    }
}

void yorglBeginFrame(YorGLRenderer* renderer) {
    if (auto* b = backend(renderer)) {
        b->beginFrame();
        succeed();
    }
}

void yorglSetViewport(YorGLRenderer* renderer, float x, float y, float width, float height) {
    if (auto* b = backend(renderer)) {
        if (!validFinite(x) || !validFinite(y) || !validFinite(width) || !validFinite(height) || width <= 0.0f || height <= 0.0f) {
            fail(YORGL_RESULT_INVALID_ARGUMENT);
            return;
        }
        b->setViewport(x, y, width, height);
        succeed();
    }
}

void yorglClearColor(YorGLRenderer* renderer, float r, float g, float b, float a) {
    if (auto* nativeBackend = ::backend(renderer)) {
        if (!validFinite(r) || !validFinite(g) || !validFinite(b) || !validFinite(a)) {
            fail(YORGL_RESULT_INVALID_ARGUMENT);
            return;
        }
        nativeBackend->clearColor(r, g, b, a);
        succeed();
    }
}

void yorglClearDepth(YorGLRenderer* renderer, float depth) {
    if (auto* b = backend(renderer)) {
        if (!validFinite(depth) || depth < 0.0f || depth > 1.0f) {
            fail(YORGL_RESULT_INVALID_ARGUMENT);
            return;
        }
        b->clearDepth(depth);
        succeed();
    }
}

void yorglSetPresentMode(YorGLRenderer* renderer, YorGLPresentMode mode) {
    if (auto* b = backend(renderer)) {
        if (!validPresentMode(mode)) {
            fail(YORGL_RESULT_INVALID_ARGUMENT);
            return;
        }
        b->setPresentMode(toPresentMode(mode));
        succeed();
    }
}

void yorglEndFrame(YorGLRenderer* renderer) {
    if (auto* b = backend(renderer)) {
        b->endFrame();
        succeed();
    }
}

int64_t yorglCreateTexture(YorGLRenderer* renderer, int width, int height, const uint8_t* rgba, int byteCount) {
    auto* b = backend(renderer);
    if (!b) return 0;
    if (!validPixelData(width, height, rgba, byteCount)) {
        fail(YORGL_RESULT_INVALID_ARGUMENT);
        return 0;
    }
    const auto texture = b->createTexture(width, height, rgba, byteCount);
    if (texture != 0 && renderer->renderer.trackTexture(texture)) {
        succeed();
    } else {
        if (texture != 0) b->destroyTexture(texture);
        fail(YORGL_RESULT_BACKEND_FAILURE);
        return 0;
    }
    return texture;
}

int yorglUpdateTextureRegion(YorGLRenderer* renderer, int64_t texture, int x, int y, int width, int height, const uint8_t* rgba, int byteCount) {
    auto* b = backend(renderer);
    if (!b) return 0;
    if (x < 0 || y < 0 || !validPixelData(width, height, rgba, byteCount)) {
        fail(YORGL_RESULT_INVALID_ARGUMENT);
        return 0;
    }
    if (!requireTexture(renderer, texture)) return 0;
    const bool updated = b->updateTextureRegion(texture, x, y, width, height, rgba, byteCount);
    if (updated) succeed(); else fail(YORGL_RESULT_BACKEND_FAILURE);
    return updated ? 1 : 0;
}

void yorglDestroyTexture(YorGLRenderer* renderer, int64_t texture) {
    if (auto* b = backend(renderer)) {
        if (!requireStandaloneTexture(renderer, texture)) return;
        b->destroyTexture(texture);
        renderer->renderer.releaseTexture(texture);
        succeed();
    }
}

void yorglGuiBegin(YorGLRenderer* renderer, int width, int height) {
    if (auto* b = backend(renderer)) {
        if (!validDimensions(width, height)) {
            fail(YORGL_RESULT_INVALID_ARGUMENT);
            return;
        }
        b->guiBegin(width, height);
        succeed();
    }
}

void yorglGuiDrawQuad(YorGLRenderer* renderer, float x, float y, float w, float h, float u0, float v0, float u1, float v1, float r, float g, float b, float a) {
    if (auto* nativeBackend = ::backend(renderer)) {
        if (!validFinite(x) || !validFinite(y) || !validFinite(w) || !validFinite(h) || w <= 0.0f || h <= 0.0f ||
            !validFinite(u0) || !validFinite(v0) || !validFinite(u1) || !validFinite(v1) ||
            !validFinite(r) || !validFinite(g) || !validFinite(b) || !validFinite(a)) {
            fail(YORGL_RESULT_INVALID_ARGUMENT);
            return;
        }
        nativeBackend->guiDrawQuad(x, y, w, h, u0, v0, u1, v1, r, g, b, a);
        succeed();
    }
}

void yorglGuiDrawGradientQuad(YorGLRenderer* renderer, float x, float y, float w, float h, const float* rgba16) {
    if (auto* b = backend(renderer)) {
        if (!validFinite(x) || !validFinite(y) || !validFinite(w) || !validFinite(h) || w <= 0.0f || h <= 0.0f || !rgba16) {
            fail(YORGL_RESULT_INVALID_ARGUMENT);
            return;
        }
        b->guiDrawGradientQuad(x, y, w, h, rgba16);
        succeed();
    }
}

void yorglGuiSetTexture(YorGLRenderer* renderer, int64_t texture) {
    if (auto* b = backend(renderer)) {
        if (!requireTexture(renderer, texture, true)) return;
        b->guiSetTexture(texture);
        succeed();
    }
}

void yorglGuiSetScissor(YorGLRenderer* renderer, float x, float y, float w, float h) {
    if (auto* b = backend(renderer)) {
        if (!validFinite(x) || !validFinite(y) || !validFinite(w) || !validFinite(h) || w <= 0.0f || h <= 0.0f) {
            fail(YORGL_RESULT_INVALID_ARGUMENT);
            return;
        }
        b->guiSetScissor(x, y, w, h);
        succeed();
    }
}

void yorglGuiClearScissor(YorGLRenderer* renderer) {
    if (auto* b = backend(renderer)) {
        b->guiClearScissor();
        succeed();
    }
}

void yorglGuiSetSdfMode(YorGLRenderer* renderer, int enabled) {
    if (auto* b = backend(renderer)) {
        if (enabled != 0 && enabled != 1) {
            fail(YORGL_RESULT_INVALID_ARGUMENT);
            return;
        }
        b->guiSetSdfMode(enabled != 0);
        succeed();
    }
}

void yorglGuiSetSdfParams(YorGLRenderer* renderer, float edge, float softness, float weightBias) {
    if (auto* b = backend(renderer)) {
        if (!validFinite(edge) || !validFinite(softness) || !validFinite(weightBias)) {
            fail(YORGL_RESULT_INVALID_ARGUMENT);
            return;
        }
        b->guiSetSdfParams(edge, softness, weightBias);
        succeed();
    }
}

void yorglGuiBlurRect(YorGLRenderer* renderer, float x, float y, float w, float h, int passes) {
    if (auto* b = backend(renderer)) {
        if (!validFinite(x) || !validFinite(y) || !validFinite(w) || !validFinite(h) || w <= 0.0f || h <= 0.0f || passes < 0) {
            fail(YORGL_RESULT_INVALID_ARGUMENT);
            return;
        }
        b->guiBlurRect(x, y, w, h, passes);
        succeed();
    }
}

void yorglGuiEnd(YorGLRenderer* renderer) {
    if (auto* b = backend(renderer)) {
        b->guiEnd();
        succeed();
    }
}

void yorglCubemapRender(YorGLRenderer* renderer, const int64_t* faces6, float yawRadians, int width, int height) {
    if (auto* b = backend(renderer)) {
        if (!faces6 || !validFinite(yawRadians) || !validDimensions(width, height)) {
            fail(YORGL_RESULT_INVALID_ARGUMENT);
            return;
        }
        for (int index = 0; index < 6; ++index) {
            if (!requireTexture(renderer, faces6[index])) return;
        }
        b->cubemapRender(faces6, yawRadians, width, height);
        succeed();
    }
}

void yorglPanoramaRender(YorGLRenderer* renderer, const int64_t* faces6, float angle, int width, int height) {
    yorglCubemapRender(renderer, faces6, angle, width, height);
}

void yorglWorldUploadMesh(YorGLRenderer* renderer, const float* vertices, int floatCount) {
    if (auto* b = backend(renderer)) {
        if (!validFloatData(vertices, floatCount)) {
            fail(YORGL_RESULT_INVALID_ARGUMENT);
            return;
        }
        b->worldUploadMesh(vertices, floatCount);
        succeed();
    }
}

void yorglWorldUploadSection(YorGLRenderer* renderer, int64_t sectionId, int x, int y, int z, const float* vertices, int floatCount) {
    if (auto* b = backend(renderer)) {
        if (!validFloatData(vertices, floatCount)) {
            fail(YORGL_RESULT_INVALID_ARGUMENT);
            return;
        }
        b->worldUploadSection(sectionId, x, y, z, vertices, floatCount);
        succeed();
    }
}

void yorglWorldUploadSectionLayer(YorGLRenderer* renderer, int64_t sectionId, int x, int y, int z, int layer, const float* vertices, int floatCount) {
    if (auto* b = backend(renderer)) {
        if (layer < 0 || layer > 3 || !validFloatData(vertices, floatCount)) {
            fail(YORGL_RESULT_INVALID_ARGUMENT);
            return;
        }
        b->worldUploadSectionLayer(sectionId, x, y, z, layer, vertices, floatCount);
        succeed();
    }
}

void yorglWorldUploadSectionLayerTextured(YorGLRenderer* renderer, int64_t sectionId, int x, int y, int z, int layer, int64_t texture, const float* vertices, int floatCount) {
    if (auto* b = backend(renderer)) {
        if (layer < 0 || layer > 3 || !validFloatData(vertices, floatCount)) {
            fail(YORGL_RESULT_INVALID_ARGUMENT);
            return;
        }
        if (!requireTexture(renderer, texture)) return;
        b->worldUploadSectionLayerTextured(sectionId, x, y, z, layer, texture, vertices, floatCount);
        succeed();
    }
}

void yorglWorldRemoveSection(YorGLRenderer* renderer, int64_t sectionId) {
    if (auto* b = backend(renderer)) {
        b->worldRemoveSection(sectionId);
        succeed();
    }
}

void yorglWorldClearSections(YorGLRenderer* renderer) {
    if (auto* b = backend(renderer)) {
        b->worldClearSections();
        succeed();
    }
}

void yorglWorldSetTexture(YorGLRenderer* renderer, int64_t texture) {
    if (auto* b = backend(renderer)) {
        if (!requireTexture(renderer, texture, true)) return;
        b->worldSetTexture(texture);
        succeed();
    }
}

void yorglWorldSetTextureFilter(YorGLRenderer* renderer, YorGLTextureFilter filter) {
    if (auto* b = backend(renderer)) {
        if (filter != YORGL_TEXTURE_FILTER_NEAREST && filter != YORGL_TEXTURE_FILTER_LINEAR) {
            fail(YORGL_RESULT_INVALID_ARGUMENT);
            return;
        }
        b->worldSetTextureFilter(toTextureFilter(filter));
        succeed();
    }
}

void yorglWorldSetSkyColor(YorGLRenderer* renderer, float r, float g, float b) {
    if (auto* nativeBackend = ::backend(renderer)) {
        if (!validFinite(r) || !validFinite(g) || !validFinite(b)) {
            fail(YORGL_RESULT_INVALID_ARGUMENT);
            return;
        }
        nativeBackend->worldSetSkyColor(r, g, b);
        succeed();
    }
}

void yorglWorldSetFog(YorGLRenderer* renderer, float r, float g, float b, float start, float end) {
    if (auto* nativeBackend = ::backend(renderer)) {
        if (!validFinite(r) || !validFinite(g) || !validFinite(b) || !validFinite(start) || !validFinite(end) || start < 0.0f || end <= start) {
            fail(YORGL_RESULT_INVALID_ARGUMENT);
            return;
        }
        nativeBackend->worldSetFog(r, g, b, start, end);
        succeed();
    }
}

void yorglWorldRender(YorGLRenderer* renderer, float cameraX, float cameraY, float cameraZ, float dirX, float dirY, float dirZ, float fovYDegrees, float farPlane, int width, int height) {
    if (auto* nativeBackend = ::backend(renderer)) {
        if (!validFinite(cameraX) || !validFinite(cameraY) || !validFinite(cameraZ) || !validFinite(dirX) || !validFinite(dirY) || !validFinite(dirZ) ||
            !validFinite(fovYDegrees) || fovYDegrees <= 0.0f || fovYDegrees >= 180.0f || !validFinite(farPlane) || farPlane <= 0.0f ||
            !validDimensions(width, height)) {
            fail(YORGL_RESULT_INVALID_ARGUMENT);
            return;
        }
        nativeBackend->worldRender(cameraX, cameraY, cameraZ, dirX, dirY, dirZ, fovYDegrees, farPlane, width, height);
        succeed();
    }
}

int64_t yorglSdfFontCreate(YorGLRenderer* renderer, const uint8_t* ttfData, int byteCount, float fontSize) {
    auto* b = backend(renderer);
    if (!b) return 0;
    if (!ttfData || byteCount <= 0 || !validFinite(fontSize) || fontSize <= 0.0f) {
        fail(YORGL_RESULT_INVALID_ARGUMENT);
        return 0;
    }
    const auto font = b->sdfFontCreate(ttfData, byteCount, fontSize);
    if (font != 0 && renderer->renderer.trackFont(font)) {
        succeed();
    } else {
        if (font != 0) b->sdfFontDestroy(font);
        fail(YORGL_RESULT_BACKEND_FAILURE);
        return 0;
    }
    return font;
}

void yorglSdfFontDestroy(YorGLRenderer* renderer, int64_t font) {
    if (auto* b = backend(renderer)) {
        if (!requireFont(renderer, font)) return;
        b->sdfFontDestroy(font);
        renderer->renderer.releaseFont(font);
        succeed();
    }
}

int64_t yorglSdfFontAtlas(YorGLRenderer* renderer, int64_t font) {
    auto* b = backend(renderer);
    if (!b) return 0;
    if (!requireFont(renderer, font)) return 0;
    const auto atlas = b->sdfFontAtlas(font);
    if (atlas != 0 && renderer->renderer.trackFontAtlas(font, atlas)) {
        succeed();
    } else {
        fail(YORGL_RESULT_BACKEND_FAILURE);
        return 0;
    }
    return atlas;
}

int yorglSdfFontMetrics(YorGLRenderer* renderer, int64_t font, float* out3) {
    auto* b = backend(renderer);
    if (!b) return 0;
    if (!requireFont(renderer, font)) return 0;
    if (!out3) {
        fail(YORGL_RESULT_INVALID_ARGUMENT);
        return 0;
    }
    const bool result = b->sdfFontMetrics(font, out3);
    if (result) succeed(); else fail(YORGL_RESULT_BACKEND_FAILURE);
    return result ? 1 : 0;
}

int yorglSdfFontGlyph(YorGLRenderer* renderer, int64_t font, int codepoint, float* out9) {
    auto* b = backend(renderer);
    if (!b) return 0;
    if (!requireFont(renderer, font)) return 0;
    if (!out9 || codepoint < 0 || codepoint > 0x10FFFF) {
        fail(YORGL_RESULT_INVALID_ARGUMENT);
        return 0;
    }
    const bool result = b->sdfFontGlyph(font, codepoint, out9);
    if (result) succeed(); else fail(YORGL_RESULT_BACKEND_FAILURE);
    return result ? 1 : 0;
}

float yorglSdfFontKerning(YorGLRenderer* renderer, int64_t font, int leftCodepoint, int rightCodepoint) {
    auto* b = backend(renderer);
    if (!b) return 0.0f;
    if (!requireFont(renderer, font)) return 0.0f;
    if (leftCodepoint < 0 || leftCodepoint > 0x10FFFF || rightCodepoint < 0 || rightCodepoint > 0x10FFFF) {
        fail(YORGL_RESULT_INVALID_ARGUMENT);
        return 0.0f;
    }
    const float result = b->sdfFontKerning(font, leftCodepoint, rightCodepoint);
    succeed();
    return result;
}

float yorglSdfFontTextWidth(YorGLRenderer* renderer, int64_t font, const char* utf8, int byteCount, float scale) {
    auto* b = backend(renderer);
    if (!b) return 0.0f;
    if (!requireFont(renderer, font)) return 0.0f;
    if (!utf8 || byteCount <= 0 || !validFinite(scale) || scale <= 0.0f) {
        fail(YORGL_RESULT_INVALID_ARGUMENT);
        return 0.0f;
    }
    const float result = b->sdfFontTextWidth(font, utf8, byteCount, scale);
    succeed();
    return result;
}

float yorglSdfFontLineHeight(YorGLRenderer* renderer, int64_t font, float scale) {
    auto* b = backend(renderer);
    if (!b) return 0.0f;
    if (!requireFont(renderer, font)) return 0.0f;
    if (!validFinite(scale) || scale <= 0.0f) {
        fail(YORGL_RESULT_INVALID_ARGUMENT);
        return 0.0f;
    }
    const float result = b->sdfFontLineHeight(font, scale);
    succeed();
    return result;
}

void yorglSdfFontDrawText(YorGLRenderer* renderer, int64_t font, const char* utf8, int byteCount, float x, float y, float scale, float r, float g, float b, float a, float weight, int shadow) {
    if (auto* native = backend(renderer)) {
        if (!requireFont(renderer, font)) return;
        if (!utf8 || byteCount <= 0 || !validFinite(x) || !validFinite(y) || !validFinite(scale) || scale <= 0.0f ||
            !validFinite(r) || !validFinite(g) || !validFinite(b) || !validFinite(a) || !validFinite(weight) ||
            (shadow != 0 && shadow != 1)) {
            fail(YORGL_RESULT_INVALID_ARGUMENT);
            return;
        }
        native->sdfFontDrawText(font, utf8, byteCount, x, y, scale, r, g, b, a, weight, shadow != 0);
        succeed();
    }
}
