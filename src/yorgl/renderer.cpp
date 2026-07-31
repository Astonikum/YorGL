#include "renderer.hpp"

namespace yorgl {

Renderer::Renderer(std::unique_ptr<Backend> backend) : backend_(std::move(backend)) {
    valid_ = backend_ && backend_->init();
}

Renderer::~Renderer() {
    if (backend_) backend_->shutdown();
}

bool Renderer::trackTexture(std::int64_t handle) {
    if (handle == 0 || textures_.contains(handle)) return false;
    textures_.insert(handle);
    standaloneTextures_.insert(handle);
    return true;
}

bool Renderer::ownsTexture(std::int64_t handle) const {
    return textures_.contains(handle);
}

bool Renderer::ownsStandaloneTexture(std::int64_t handle) const {
    return standaloneTextures_.contains(handle);
}

void Renderer::releaseTexture(std::int64_t handle) {
    textures_.erase(handle);
    standaloneTextures_.erase(handle);
}

bool Renderer::trackFont(std::int64_t handle) {
    return handle != 0 && fonts_.insert(handle).second;
}

bool Renderer::ownsFont(std::int64_t handle) const {
    return fonts_.contains(handle);
}

bool Renderer::trackFontAtlas(std::int64_t font, std::int64_t atlas) {
    if (!ownsFont(font) || atlas == 0) return false;
    if (const auto iterator = fontAtlases_.find(font); iterator != fontAtlases_.end()) {
        return iterator->second == atlas;
    }
    if (textures_.contains(atlas)) return false;
    fontAtlases_[font] = atlas;
    textures_.insert(atlas);
    return true;
}

std::int64_t Renderer::releaseFont(std::int64_t font) {
    const auto iterator = fontAtlases_.find(font);
    const std::int64_t atlas = iterator == fontAtlases_.end() ? 0 : iterator->second;
    if (iterator != fontAtlases_.end()) {
        textures_.erase(iterator->second);
        fontAtlases_.erase(iterator);
    }
    fonts_.erase(font);
    return atlas;
}

} // namespace yorgl
