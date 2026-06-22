#pragma once

#include "../../yorgl/backend.hpp"

namespace yorgl {

class NullBackend final : public Backend {
public:
    std::string_view name() const override { return "null"; }
    bool init() override { return true; }
    void shutdown() override {}
    bool createSwapChain(std::int64_t, int width, int height) override;
    void resize(int width, int height) override;
    void beginFrame() override {}
    void clearColor(float r, float g, float b, float a) override;
    void endFrame() override {}

private:
    int width_ = 0;
    int height_ = 0;
    float clear_[4] = {0.0f, 0.0f, 0.0f, 1.0f};
};

} // namespace yorgl
