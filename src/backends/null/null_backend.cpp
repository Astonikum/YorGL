#include "null_backend.hpp"
#include "../../yorgl/renderer.hpp"

#ifdef YORGL_HAS_DX11
#include "../dx11/dx11_backend.hpp"
#endif

namespace yorgl {

bool NullBackend::createSwapChain(std::int64_t, const SwapChainOptions& options) {
    width_ = options.width;
    height_ = options.height;
    presentMode_ = options.presentMode;
    return width_ > 0 && height_ > 0;
}

void NullBackend::resize(int width, int height) {
    width_ = width;
    height_ = height;
}

void NullBackend::clearColor(float r, float g, float b, float a) {
    clear_[0] = r;
    clear_[1] = g;
    clear_[2] = b;
    clear_[3] = a;
}

BackendCapabilities NullBackend::capabilities() const {
    BackendCapabilities caps;
    caps.backend = BackendKind::Null;
    caps.presentVSync = true;
    caps.presentImmediate = true;
    return caps;
}

std::unique_ptr<Backend> createBackend(BackendKind kind) {
    switch (kind) {
#ifdef YORGL_HAS_DX11
        case BackendKind::Dx11:
            return std::make_unique<Dx11Backend>();
#endif
        case BackendKind::Null:
        default:
            return std::make_unique<NullBackend>();
    }
}

} // namespace yorgl
