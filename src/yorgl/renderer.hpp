#pragma once

#include "backend.hpp"
#include <memory>
#include <unordered_map>
#include <unordered_set>

namespace yorgl {

class Renderer {
public:
    explicit Renderer(std::unique_ptr<Backend> backend);
    ~Renderer();

    bool valid() const { return valid_; }
    Backend& backend() { return *backend_; }
    const Backend& backend() const { return *backend_; }

    bool trackTexture(std::int64_t handle);
    bool ownsTexture(std::int64_t handle) const;
    bool ownsStandaloneTexture(std::int64_t handle) const;
    void releaseTexture(std::int64_t handle);
    bool trackFont(std::int64_t handle);
    bool ownsFont(std::int64_t handle) const;
    bool trackFontAtlas(std::int64_t font, std::int64_t atlas);
    std::int64_t releaseFont(std::int64_t font);

private:
    std::unique_ptr<Backend> backend_;
    bool valid_ = false;
    std::unordered_set<std::int64_t> textures_;
    std::unordered_set<std::int64_t> standaloneTextures_;
    std::unordered_set<std::int64_t> fonts_;
    std::unordered_map<std::int64_t, std::int64_t> fontAtlases_;
};

std::unique_ptr<Backend> createBackend(BackendKind kind);

} // namespace yorgl
