#pragma once

#include "../../yorgl/backend.hpp"
#include <unordered_map>
#include <utility>

namespace yorgl {

class NullBackend final : public Backend {
public:
    std::string_view name() const override { return "null"; }
    bool init() override { return true; }
    void shutdown() override {}
    bool createSwapChain(std::int64_t, const SwapChainOptions& options) override;
    void resize(int width, int height) override;
    void beginFrame() override {}
    void setViewport(float, float, float, float) override {}
    void clearColor(float r, float g, float b, float a) override;
    void clearDepth(float) override {}
    BackendCapabilities capabilities() const override;
    void setPresentMode(PresentMode mode) override { presentMode_ = mode; }
    void endFrame() override {}

    std::int64_t createTexture(int width, int height, const std::uint8_t* rgba, int byteCount) override;
    bool updateTextureRegion(std::int64_t texture, int x, int y, int width, int height, const std::uint8_t* rgba, int byteCount) override;
    void destroyTexture(std::int64_t texture) override;

    std::int64_t sdfFontCreate(const std::uint8_t* ttfData, int byteCount, float fontSize) override;
    void sdfFontDestroy(std::int64_t font) override;
    std::int64_t sdfFontAtlas(std::int64_t font) override;

private:
    int width_ = 0;
    int height_ = 0;
    float clear_[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    PresentMode presentMode_ = PresentMode::VSync;
    std::int64_t nextHandle_ = 1;
    std::unordered_map<std::int64_t, std::pair<int, int>> textures_;
    std::unordered_map<std::int64_t, std::int64_t> fonts_;
};

} // namespace yorgl
