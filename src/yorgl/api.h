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

typedef struct YorGLRenderer YorGLRenderer;

YORGL_API YorGLRenderer* yorglCreate(YorGLBackendKind backend);
YORGL_API void yorglDestroy(YorGLRenderer* renderer);
YORGL_API int yorglIsValid(YorGLRenderer* renderer);
YORGL_API const char* yorglBackendName(YorGLRenderer* renderer);
YORGL_API int yorglCreateSwapChain(YorGLRenderer* renderer, int64_t windowHandle, int width, int height);
YORGL_API void yorglResize(YorGLRenderer* renderer, int width, int height);
YORGL_API void yorglBeginFrame(YorGLRenderer* renderer);
YORGL_API void yorglSetViewport(YorGLRenderer* renderer, float x, float y, float width, float height);
YORGL_API void yorglClearColor(YorGLRenderer* renderer, float r, float g, float b, float a);
YORGL_API void yorglClearDepth(YorGLRenderer* renderer, float depth);
YORGL_API void yorglEndFrame(YorGLRenderer* renderer);

YORGL_API int64_t yorglCreateTexture(YorGLRenderer* renderer, int width, int height, const uint8_t* rgba, int byteCount);
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

YORGL_API void yorglPanoramaRender(YorGLRenderer* renderer, const int64_t* faces6, float angle, int width, int height);

YORGL_API void yorglWorldUploadMesh(YorGLRenderer* renderer, const float* vertices, int floatCount);
YORGL_API void yorglWorldUploadSection(YorGLRenderer* renderer, int64_t sectionId, int x, int y, int z, const float* vertices, int floatCount);
YORGL_API void yorglWorldUploadSectionLayer(YorGLRenderer* renderer, int64_t sectionId, int x, int y, int z, int layer, const float* vertices, int floatCount);
YORGL_API void yorglWorldRemoveSection(YorGLRenderer* renderer, int64_t sectionId);
YORGL_API void yorglWorldClearSections(YorGLRenderer* renderer);
YORGL_API void yorglWorldSetTexture(YorGLRenderer* renderer, int64_t texture);
YORGL_API void yorglWorldSetSkyColor(YorGLRenderer* renderer, float r, float g, float b);
YORGL_API void yorglWorldRender(YorGLRenderer* renderer, float cameraX, float cameraY, float cameraZ, float dirX, float dirY, float dirZ, float fovYDegrees, float farPlane, int width, int height);

YORGL_API int64_t yorglSdfFontCreate(YorGLRenderer* renderer, const uint8_t* ttfData, int byteCount, float fontSize);
YORGL_API void yorglSdfFontDestroy(YorGLRenderer* renderer, int64_t font);
YORGL_API int64_t yorglSdfFontAtlas(YorGLRenderer* renderer, int64_t font);
YORGL_API int yorglSdfFontMetrics(YorGLRenderer* renderer, int64_t font, float* out3);
YORGL_API int yorglSdfFontGlyph(YorGLRenderer* renderer, int64_t font, int codepoint, float* out9);
YORGL_API float yorglSdfFontKerning(YorGLRenderer* renderer, int64_t font, int leftCodepoint, int rightCodepoint);

#ifdef __cplusplus
}
#endif
