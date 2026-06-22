#pragma once

#include <cstdint>

// Abstract render backend interface for multi-API support
class IRenderBackend {
public:
    virtual ~IRenderBackend() = default;

    virtual bool init() = 0;
    virtual void shutdown() = 0;
    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;

    virtual void createSwapChain(int64_t windowHandle, int width, int height) = 0;
    virtual void resizeSwapChain(int width, int height) = 0;

    virtual int64_t createShader(const uint8_t* vsSrc, int vsLen, const uint8_t* psSrc, int psLen) = 0;
    virtual void destroyShader(int64_t handle) = 0;

    virtual int64_t createBuffer(int type, const uint8_t* data, int dataLen, int usage) = 0;
    virtual void destroyBuffer(int64_t handle) = 0;

    virtual void draw(int vertexCount, int startVertex) = 0;
    virtual void drawIndexed(int indexCount, int startIndex, int baseVertex) = 0;

    virtual void setViewport(float x, float y, float width, float height) = 0;
    virtual void clearColor(float r, float g, float b, float a) = 0;
    virtual void clearDepth(float depth) = 0;
};
