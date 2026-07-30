#include "yorgl/api.h"
#include <cassert>
#include <cstring>

int main() {
    YorGLRenderer* renderer = yorglCreate(YORGL_BACKEND_NULL);
    assert(renderer);
    assert(yorglIsValid(renderer));
    assert(std::strcmp(yorglBackendName(renderer), "null") == 0);
    assert(yorglCreateSwapChain(renderer, 0, 16, 16));
    yorglBeginFrame(renderer);
    yorglClearColor(renderer, 0.1f, 0.2f, 0.3f, 1.0f);
    yorglWorldUploadSectionLayerTextured(renderer, 1, 0, 0, 0, 1, 0, nullptr, 0);
    yorglEndFrame(renderer);
    yorglDestroy(renderer);
    return 0;
}
