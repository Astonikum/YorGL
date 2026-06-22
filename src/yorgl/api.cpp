#include "api.h"
#include "renderer.hpp"

using yorgl::BackendKind;
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
    if (!renderer || !renderer->renderer.valid()) return 0;
    return renderer->renderer.backend().createSwapChain(windowHandle, width, height);
}

void yorglResize(YorGLRenderer* renderer, int width, int height) {
    if (renderer && renderer->renderer.valid()) renderer->renderer.backend().resize(width, height);
}

void yorglBeginFrame(YorGLRenderer* renderer) {
    if (renderer && renderer->renderer.valid()) renderer->renderer.backend().beginFrame();
}

void yorglClearColor(YorGLRenderer* renderer, float r, float g, float b, float a) {
    if (renderer && renderer->renderer.valid()) renderer->renderer.backend().clearColor(r, g, b, a);
}

void yorglEndFrame(YorGLRenderer* renderer) {
    if (renderer && renderer->renderer.valid()) renderer->renderer.backend().endFrame();
}
