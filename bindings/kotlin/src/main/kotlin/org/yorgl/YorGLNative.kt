package org.yorgl

internal object YorGLNative {
    external fun create(backend: Int): Long
    external fun destroy(ptr: Long)
    external fun isValid(ptr: Long): Boolean
    external fun backendName(ptr: Long): String
    external fun createSwapChain(ptr: Long, windowHandle: Long, width: Int, height: Int): Boolean
    external fun resize(ptr: Long, width: Int, height: Int)
    external fun beginFrame(ptr: Long)
    external fun clearColor(ptr: Long, r: Float, g: Float, b: Float, a: Float)
    external fun endFrame(ptr: Long)
}
