#pragma once

#include <cstdint>
#include <string_view>

namespace yorgl {

enum class BackendKind {
    Null = 0,
    Dx11 = 1,
};

class Backend {
public:
    virtual ~Backend() = default;
    virtual std::string_view name() const = 0;
    virtual bool init() = 0;
    virtual void shutdown() = 0;
    virtual bool createSwapChain(std::int64_t windowHandle, int width, int height) = 0;
    virtual void resize(int width, int height) = 0;
    virtual void beginFrame() = 0;
    virtual void clearColor(float r, float g, float b, float a) = 0;
    virtual void endFrame() = 0;
};

} // namespace yorgl
