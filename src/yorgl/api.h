#pragma once

#include "export.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum YorGLBackendKind {
    YORGL_BACKEND_NULL = 0,
    YORGL_BACKEND_DX11 = 1
} YorGLBackendKind;

typedef enum YorGLPresentMode {
    YORGL_PRESENT_VSYNC = 0,
    YORGL_PRESENT_IMMEDIATE = 1
} YorGLPresentMode;

typedef enum YorGLTextureFilter {
    YORGL_TEXTURE_FILTER_NEAREST = 0,
    YORGL_TEXTURE_FILTER_LINEAR = 1
} YorGLTextureFilter;

typedef struct YorGLCapabilities {
    int backend;
    int featureLevelMajor;
    int featureLevelMinor;
    int maxTextureSize;
    int presentVSync;
    int presentImmediate;
    int presentTearing;
} YorGLCapabilities;

typedef struct YorGLSwapChainOptions {
    int width;
    int height;
    int bufferCount;
    YorGLPresentMode presentMode;
    int allowTearing;
} YorGLSwapChainOptions;

typedef struct YorGLRenderDiagnostics {
    int lastResizeResult;
    int lastPresentResult;
    int deviceRemovedReason;
} YorGLRenderDiagnostics;

typedef struct YorGLRenderer YorGLRenderer;

YORGL_API YorGLRenderer* yorglCreate(YorGLBackendKind backend);
YORGL_API void yorglDestroy(YorGLRenderer* renderer);
YORGL_API int yorglIsValid(YorGLRenderer* renderer);
YORGL_API const char* yorglBackendName(YorGLRenderer* renderer);
YORGL_API int yorglGetCapabilities(YorGLRenderer* renderer, YorGLCapabilities* outCapabilities);
YORGL_API int yorglGetDiagnostics(YorGLRenderer* renderer, YorGLRenderDiagnostics* outDiagnostics);
YORGL_API int yorglCreateSwapChain(YorGLRenderer* renderer, int64_t windowHandle, int width, int height);
YORGL_API int yorglCreateSwapChainWithOptions(YorGLRenderer* renderer, int64_t windowHandle, const YorGLSwapChainOptions* options);
YORGL_API void yorglResize(YorGLRenderer* renderer, int width, int height);
YORGL_API void yorglBeginFrame(YorGLRenderer* renderer);
YORGL_API void yorglSetViewport(YorGLRenderer* renderer, float x, float y, float width, float height);
YORGL_API void yorglClearColor(YorGLRenderer* renderer, float r, float g, float b, float a);
YORGL_API void yorglClearDepth(YorGLRenderer* renderer, float depth);
YORGL_API void yorglSetPresentMode(YorGLRenderer* renderer, YorGLPresentMode mode);
YORGL_API void yorglEndFrame(YorGLRenderer* renderer);

YORGL_API int64_t yorglCreateTexture(YorGLRenderer* renderer, int width, int height, const uint8_t* rgba, int byteCount);
YORGL_API int yorglUpdateTextureRegion(YorGLRenderer* renderer, int64_t texture, int x, int y, int width, int height, const uint8_t* rgba, int byteCount);
YORGL_API void yorglDestroyTexture(YorGLRenderer* renderer, int64_t texture);

YORGL_API void yorglGuiBegin(YorGLRenderer* renderer, int width, int height);
YORGL_API void yorglGuiDrawQuad(YorGLRenderer* renderer, float x, float y, float w, float h, float u0, float v0, float u1, float v1, float r, float g, float b, float a);
YORGL_API void yorglGuiDrawGradientQuad(YorGLRenderer* renderer, float x, float y, float w, float h, const float* rgba16);
YORGL_API void yorglGuiSetTexture(YorGLRenderer* renderer, int64_t texture);
YORGL_API void yorglGuiSetScissor(YorGLRenderer* renderer, float x, float y, float w, float h);
YORGL_API void yorglGuiClearScissor(YorGLRenderer* renderer);
YORGL_API void yorglGuiSetSdfMode(YorGLRenderer* renderer, int enabled);
YORGL_API void yorglGuiSetSdfParams(YorGLRenderer* renderer, float edge, float softness, float weightBias);
YORGL_API void yorglGuiBlurRect(YorGLRenderer* renderer, float x, float y, float w, float h, int passes);
YORGL_API void yorglGuiEnd(YorGLRenderer* renderer);

YORGL_API void yorglCubemapRender(YorGLRenderer* renderer, const int64_t* faces6, float yawRadians, int width, int height);
YORGL_API void yorglPanoramaRender(YorGLRenderer* renderer, const int64_t* faces6, float angle, int width, int height);

YORGL_API void yorglWorldUploadMesh(YorGLRenderer* renderer, const float* vertices, int floatCount);
YORGL_API void yorglWorldUploadSection(YorGLRenderer* renderer, int64_t sectionId, int x, int y, int z, const float* vertices, int floatCount);
YORGL_API void yorglWorldUploadSectionLayer(YorGLRenderer* renderer, int64_t sectionId, int x, int y, int z, int layer, const float* vertices, int floatCount);
YORGL_API void yorglWorldUploadSectionLayerTextured(YorGLRenderer* renderer, int64_t sectionId, int x, int y, int z, int layer, int64_t texture, const float* vertices, int floatCount);
YORGL_API void yorglWorldRemoveSection(YorGLRenderer* renderer, int64_t sectionId);
YORGL_API void yorglWorldClearSections(YorGLRenderer* renderer);
YORGL_API void yorglWorldSetTexture(YorGLRenderer* renderer, int64_t texture);
YORGL_API void yorglWorldSetTextureFilter(YorGLRenderer* renderer, YorGLTextureFilter filter);
YORGL_API void yorglWorldSetSkyColor(YorGLRenderer* renderer, float r, float g, float b);
YORGL_API void yorglWorldSetFog(YorGLRenderer* renderer, float r, float g, float b, float start, float end);
YORGL_API void yorglWorldRender(YorGLRenderer* renderer, float cameraX, float cameraY, float cameraZ, float dirX, float dirY, float dirZ, float fovYDegrees, float farPlane, int width, int height);

YORGL_API int64_t yorglSdfFontCreate(YorGLRenderer* renderer, const uint8_t* ttfData, int byteCount, float fontSize);
YORGL_API void yorglSdfFontDestroy(YorGLRenderer* renderer, int64_t font);
YORGL_API int64_t yorglSdfFontAtlas(YorGLRenderer* renderer, int64_t font);
YORGL_API int yorglSdfFontMetrics(YorGLRenderer* renderer, int64_t font, float* out3);
YORGL_API int yorglSdfFontGlyph(YorGLRenderer* renderer, int64_t font, int codepoint, float* out9);
YORGL_API float yorglSdfFontKerning(YorGLRenderer* renderer, int64_t font, int leftCodepoint, int rightCodepoint);
YORGL_API float yorglSdfFontTextWidth(YorGLRenderer* renderer, int64_t font, const char* utf8, int byteCount, float scale);
YORGL_API float yorglSdfFontLineHeight(YorGLRenderer* renderer, int64_t font, float scale);
YORGL_API void yorglSdfFontDrawText(YorGLRenderer* renderer, int64_t font, const char* utf8, int byteCount, float x, float y, float scale, float r, float g, float b, float a, float weight, int shadow);

#ifdef __cplusplus
}
#endif
