#include "api.h"
#include "renderer.hpp"

using yorgl::BackendKind;
using yorgl::PresentMode;
using yorgl::Renderer;

struct YorGLRenderer {
    Renderer renderer;
};

static BackendKind toBackend(YorGLBackendKind backend) {
    switch (backend) {
        case YORGL_BACKEND_DX11: return BackendKind::Dx11;
        case YORGL_BACKEND_NULL:
        default: return BackendKind::Null;
    }
}

static yorgl::Backend* backend(YorGLRenderer* renderer) {
    if (!renderer || !renderer->renderer.valid()) return nullptr;
    return &renderer->renderer.backend();
}

static PresentMode toPresentMode(YorGLPresentMode mode) {
    switch (mode) {
        case YORGL_PRESENT_IMMEDIATE: return PresentMode::Immediate;
        case YORGL_PRESENT_VSYNC:
        default: return PresentMode::VSync;
    }
}

YorGLRenderer* yorglCreate(YorGLBackendKind backend) {
    auto native = yorgl::createBackend(toBackend(backend));
    if (!native) return nullptr;
    return new YorGLRenderer{Renderer(std::move(native))};
}

void yorglDestroy(YorGLRenderer* renderer) {
    delete renderer;
}

int yorglIsValid(YorGLRenderer* renderer) {
    return renderer && renderer->renderer.valid();
}

const char* yorglBackendName(YorGLRenderer* renderer) {
    if (!renderer) return "invalid";
    auto name = renderer->renderer.backend().name();
    return name.data();
}

int yorglCreateSwapChain(YorGLRenderer* renderer, int64_t windowHandle, int width, int height) {
    auto* b = backend(renderer);
    return b ? b->createSwapChain(windowHandle, width, height) : 0;
}

void yorglResize(YorGLRenderer* renderer, int width, int height) {
    if (auto* b = backend(renderer)) b->resize(width, height);
}

void yorglBeginFrame(YorGLRenderer* renderer) {
    if (auto* b = backend(renderer)) b->beginFrame();
}

void yorglSetViewport(YorGLRenderer* renderer, float x, float y, float width, float height) {
    if (auto* b = backend(renderer)) b->setViewport(x, y, width, height);
}

void yorglClearColor(YorGLRenderer* renderer, float r, float g, float b, float a) {
    if (auto* backend = ::backend(renderer)) backend->clearColor(r, g, b, a);
}

void yorglClearDepth(YorGLRenderer* renderer, float depth) {
    if (auto* b = backend(renderer)) b->clearDepth(depth);
}

void yorglSetPresentMode(YorGLRenderer* renderer, YorGLPresentMode mode) {
    if (auto* b = backend(renderer)) b->setPresentMode(toPresentMode(mode));
}

void yorglEndFrame(YorGLRenderer* renderer) {
    if (auto* b = backend(renderer)) b->endFrame();
}

int64_t yorglCreateTexture(YorGLRenderer* renderer, int width, int height, const uint8_t* rgba, int byteCount) {
    auto* b = backend(renderer);
    return b ? b->createTexture(width, height, rgba, byteCount) : 0;
}

void yorglDestroyTexture(YorGLRenderer* renderer, int64_t texture) {
    if (auto* b = backend(renderer)) b->destroyTexture(texture);
}

void yorglGuiBegin(YorGLRenderer* renderer, int width, int height) {
    if (auto* b = backend(renderer)) b->guiBegin(width, height);
}

void yorglGuiDrawQuad(YorGLRenderer* renderer, float x, float y, float w, float h, float u0, float v0, float u1, float v1, float r, float g, float b, float a) {
    if (auto* backend = ::backend(renderer)) backend->guiDrawQuad(x, y, w, h, u0, v0, u1, v1, r, g, b, a);
}

void yorglGuiDrawGradientQuad(YorGLRenderer* renderer, float x, float y, float w, float h, const float* rgba16) {
    if (auto* b = backend(renderer)) b->guiDrawGradientQuad(x, y, w, h, rgba16);
}

void yorglGuiSetTexture(YorGLRenderer* renderer, int64_t texture) {
    if (auto* b = backend(renderer)) b->guiSetTexture(texture);
}

void yorglGuiSetScissor(YorGLRenderer* renderer, float x, float y, float w, float h) {
    if (auto* b = backend(renderer)) b->guiSetScissor(x, y, w, h);
}

void yorglGuiClearScissor(YorGLRenderer* renderer) {
    if (auto* b = backend(renderer)) b->guiClearScissor();
}

void yorglGuiSetSdfMode(YorGLRenderer* renderer, int enabled) {
    if (auto* b = backend(renderer)) b->guiSetSdfMode(enabled != 0);
}

void yorglGuiSetSdfParams(YorGLRenderer* renderer, float edge, float softness, float weightBias) {
    if (auto* b = backend(renderer)) b->guiSetSdfParams(edge, softness, weightBias);
}

void yorglGuiBlurRect(YorGLRenderer* renderer, float x, float y, float w, float h, int passes) {
    if (auto* b = backend(renderer)) b->guiBlurRect(x, y, w, h, passes);
}

void yorglGuiEnd(YorGLRenderer* renderer) {
    if (auto* b = backend(renderer)) b->guiEnd();
}

void yorglPanoramaRender(YorGLRenderer* renderer, const int64_t* faces6, float angle, int width, int height) {
    if (auto* b = backend(renderer)) b->panoramaRender(faces6, angle, width, height);
}

void yorglWorldUploadMesh(YorGLRenderer* renderer, const float* vertices, int floatCount) {
    if (auto* b = backend(renderer)) b->worldUploadMesh(vertices, floatCount);
}

void yorglWorldUploadSection(YorGLRenderer* renderer, int64_t sectionId, int x, int y, int z, const float* vertices, int floatCount) {
    if (auto* b = backend(renderer)) b->worldUploadSection(sectionId, x, y, z, vertices, floatCount);
}

void yorglWorldUploadSectionLayer(YorGLRenderer* renderer, int64_t sectionId, int x, int y, int z, int layer, const float* vertices, int floatCount) {
    if (auto* b = backend(renderer)) b->worldUploadSectionLayer(sectionId, x, y, z, layer, vertices, floatCount);
}

void yorglWorldRemoveSection(YorGLRenderer* renderer, int64_t sectionId) {
    if (auto* b = backend(renderer)) b->worldRemoveSection(sectionId);
}

void yorglWorldClearSections(YorGLRenderer* renderer) {
    if (auto* b = backend(renderer)) b->worldClearSections();
}

void yorglWorldSetTexture(YorGLRenderer* renderer, int64_t texture) {
    if (auto* b = backend(renderer)) b->worldSetTexture(texture);
}

void yorglWorldSetSkyColor(YorGLRenderer* renderer, float r, float g, float b) {
    if (auto* backend = ::backend(renderer)) backend->worldSetSkyColor(r, g, b);
}

void yorglWorldRender(YorGLRenderer* renderer, float cameraX, float cameraY, float cameraZ, float dirX, float dirY, float dirZ, float fovYDegrees, float farPlane, int width, int height) {
    if (auto* backend = ::backend(renderer)) backend->worldRender(cameraX, cameraY, cameraZ, dirX, dirY, dirZ, fovYDegrees, farPlane, width, height);
}

int64_t yorglSdfFontCreate(YorGLRenderer* renderer, const uint8_t* ttfData, int byteCount, float fontSize) {
    auto* b = backend(renderer);
    return b ? b->sdfFontCreate(ttfData, byteCount, fontSize) : 0;
}

void yorglSdfFontDestroy(YorGLRenderer* renderer, int64_t font) {
    if (auto* b = backend(renderer)) b->sdfFontDestroy(font);
}

int64_t yorglSdfFontAtlas(YorGLRenderer* renderer, int64_t font) {
    auto* b = backend(renderer);
    return b ? b->sdfFontAtlas(font) : 0;
}

int yorglSdfFontMetrics(YorGLRenderer* renderer, int64_t font, float* out3) {
    auto* b = backend(renderer);
    return b && b->sdfFontMetrics(font, out3);
}

int yorglSdfFontGlyph(YorGLRenderer* renderer, int64_t font, int codepoint, float* out9) {
    auto* b = backend(renderer);
    return b && b->sdfFontGlyph(font, codepoint, out9);
}

float yorglSdfFontKerning(YorGLRenderer* renderer, int64_t font, int leftCodepoint, int rightCodepoint) {
    auto* b = backend(renderer);
    return b ? b->sdfFontKerning(font, leftCodepoint, rightCodepoint) : 0.0f;
}

float yorglSdfFontTextWidth(YorGLRenderer* renderer, int64_t font, const char* utf8, int byteCount, float scale) {
    auto* b = backend(renderer);
    return b ? b->sdfFontTextWidth(font, utf8, byteCount, scale) : 0.0f;
}

float yorglSdfFontLineHeight(YorGLRenderer* renderer, int64_t font, float scale) {
    auto* b = backend(renderer);
    return b ? b->sdfFontLineHeight(font, scale) : 0.0f;
}

void yorglSdfFontDrawText(YorGLRenderer* renderer, int64_t font, const char* utf8, int byteCount, float x, float y, float scale, float r, float g, float b, float a, float weight, int shadow) {
    if (auto* native = backend(renderer)) native->sdfFontDrawText(font, utf8, byteCount, x, y, scale, r, g, b, a, weight, shadow != 0);
}
