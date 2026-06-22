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
YORGL_API void yorglClearColor(YorGLRenderer* renderer, float r, float g, float b, float a);
YORGL_API void yorglEndFrame(YorGLRenderer* renderer);

#ifdef __cplusplus
}
#endif
