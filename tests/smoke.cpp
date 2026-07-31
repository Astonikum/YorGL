#include "yorgl/api.h"
#include <cassert>
#include <cstdint>
#include <cstring>
#include <limits>

int main() {
    assert(yorglCreate(static_cast<YorGLBackendKind>(99)) == nullptr);
    assert(yorglGetLastError() == YORGL_RESULT_INVALID_ARGUMENT);
    YorGLCapabilities invalidCapabilities{};
    assert(yorglGetCapabilities(nullptr, &invalidCapabilities) == 0);
    assert(yorglGetLastError() == YORGL_RESULT_INVALID_HANDLE);
    yorglClearLastError();

    YorGLRenderer* renderer = yorglCreate(YORGL_BACKEND_NULL);
    assert(renderer);
    assert(yorglIsValid(renderer));
    assert(std::strcmp(yorglBackendName(renderer), "null") == 0);
    assert(yorglCreateSwapChain(renderer, 0, 0, 16) == 0);
    assert(yorglGetLastError() == YORGL_RESULT_INVALID_ARGUMENT);
    assert(yorglCreateSwapChain(renderer, 0, 16, 16));
    assert(yorglGetLastError() == YORGL_RESULT_OK);
    YorGLSwapChainOptions invalidOptions{16, 16, 1, YORGL_PRESENT_VSYNC, 0};
    assert(yorglCreateSwapChainWithOptions(renderer, 0, &invalidOptions) == 0);
    assert(yorglGetLastError() == YORGL_RESULT_INVALID_ARGUMENT);
    yorglSetViewport(renderer, 0.0f, 0.0f, 0.0f, 16.0f);
    assert(yorglGetLastError() == YORGL_RESULT_INVALID_ARGUMENT);
    yorglClearColor(renderer, std::numeric_limits<float>::quiet_NaN(), 0.2f, 0.3f, 1.0f);
    assert(yorglGetLastError() == YORGL_RESULT_INVALID_ARGUMENT);
    assert(yorglCreateTexture(renderer, 1, 1, nullptr, 4) == 0);
    assert(yorglGetLastError() == YORGL_RESULT_INVALID_ARGUMENT);
    const std::uint8_t pixel[4] = {255, 128, 64, 255};
    const auto texture = yorglCreateTexture(renderer, 1, 1, pixel, sizeof(pixel));
    assert(texture != 0);
    assert(yorglGetLastError() == YORGL_RESULT_OK);
    assert(yorglUpdateTextureRegion(renderer, texture, 0, 0, 1, 1, pixel, sizeof(pixel)));
    assert(yorglUpdateTextureRegion(renderer, 777, 0, 0, 1, 1, pixel, sizeof(pixel)) == 0);
    assert(yorglGetLastError() == YORGL_RESULT_INVALID_HANDLE);
    yorglGuiSetTexture(renderer, texture);
    assert(yorglGetLastError() == YORGL_RESULT_OK);
    yorglWorldSetTexture(renderer, texture);
    assert(yorglGetLastError() == YORGL_RESULT_OK);
    yorglDestroyTexture(renderer, texture);
    assert(yorglGetLastError() == YORGL_RESULT_OK);
    yorglDestroyTexture(renderer, texture);
    assert(yorglGetLastError() == YORGL_RESULT_INVALID_HANDLE);
    const std::uint8_t fakeTtf[4] = {0, 1, 2, 3};
    const auto font = yorglSdfFontCreate(renderer, fakeTtf, sizeof(fakeTtf), 16.0f);
    assert(font != 0);
    const auto atlas = yorglSdfFontAtlas(renderer, font);
    assert(atlas != 0);
    yorglGuiSetTexture(renderer, atlas);
    assert(yorglGetLastError() == YORGL_RESULT_OK);
    yorglDestroyTexture(renderer, atlas);
    assert(yorglGetLastError() == YORGL_RESULT_INVALID_HANDLE);
    yorglSdfFontDestroy(renderer, font);
    assert(yorglGetLastError() == YORGL_RESULT_OK);
    yorglGuiSetTexture(renderer, atlas);
    assert(yorglGetLastError() == YORGL_RESULT_INVALID_HANDLE);
    yorglDestroyTexture(renderer, 0);
    assert(yorglGetLastError() == YORGL_RESULT_INVALID_HANDLE);
    yorglBeginFrame(renderer);
    yorglClearColor(renderer, 0.1f, 0.2f, 0.3f, 1.0f);
    const float vertex[9] = {};
    yorglWorldUploadMesh(renderer, vertex, 8);
    assert(yorglGetLastError() == YORGL_RESULT_INVALID_ARGUMENT);
    yorglWorldUploadMesh(renderer, vertex, 9);
    assert(yorglGetLastError() == YORGL_RESULT_OK);
    yorglWorldUploadSectionLayerTextured(renderer, 1, 0, 0, 0, 1, 0, vertex, 9);
    assert(yorglGetLastError() == YORGL_RESULT_INVALID_HANDLE);
    yorglEndFrame(renderer);
    yorglDestroy(renderer);
    return 0;
}
