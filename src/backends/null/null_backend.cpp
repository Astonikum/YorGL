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

std::int64_t NullBackend::createTexture(int width, int height, const std::uint8_t* rgba, int byteCount) {
    if (width <= 0 || height <= 0 || !rgba || byteCount <= 0) return 0;
    const auto handle = nextHandle_++;
    textures_.emplace(handle, std::make_pair(width, height));
    return handle;
}

bool NullBackend::updateTextureRegion(std::int64_t texture, int x, int y, int width, int height, const std::uint8_t* rgba, int byteCount) {
    const auto iterator = textures_.find(texture);
    if (iterator == textures_.end() || x < 0 || y < 0 || width <= 0 || height <= 0 || !rgba || byteCount <= 0) return false;
    return static_cast<std::int64_t>(x) + width <= iterator->second.first &&
        static_cast<std::int64_t>(y) + height <= iterator->second.second;
}

void NullBackend::destroyTexture(std::int64_t texture) {
    textures_.erase(texture);
}

std::int64_t NullBackend::sdfFontCreate(const std::uint8_t* ttfData, int byteCount, float fontSize) {
    if (!ttfData || byteCount <= 0 || fontSize <= 0.0f) return 0;
    const auto handle = nextHandle_++;
    fonts_.emplace(handle, 0);
    return handle;
}

void NullBackend::sdfFontDestroy(std::int64_t font) {
    const auto iterator = fonts_.find(font);
    if (iterator == fonts_.end()) return;
    if (iterator->second != 0) textures_.erase(iterator->second);
    fonts_.erase(iterator);
}

std::int64_t NullBackend::sdfFontAtlas(std::int64_t font) {
    const auto iterator = fonts_.find(font);
    if (iterator == fonts_.end()) return 0;
    if (iterator->second == 0) {
        iterator->second = nextHandle_++;
        textures_.emplace(iterator->second, std::make_pair(1, 1));
    }
    return iterator->second;
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
