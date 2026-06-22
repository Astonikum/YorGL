package org.yorgl

enum class BackendKind(internal val id: Int) {
    Null(0),
    Dx11(1),
}

class YorGL private constructor(private var handle: Long) : AutoCloseable {
    val valid: Boolean get() = handle != 0L && YorGLNative.isValid(handle)
    val backendName: String get() = YorGLNative.backendName(handle)

    fun createSwapChain(windowHandle: Long, width: Int, height: Int): Boolean =
        YorGLNative.createSwapChain(handle, windowHandle, width, height)

    fun resize(width: Int, height: Int) = YorGLNative.resize(handle, width, height)
    fun beginFrame() = YorGLNative.beginFrame(handle)
    fun clearColor(r: Float, g: Float, b: Float, a: Float) = YorGLNative.clearColor(handle, r, g, b, a)
    fun endFrame() = YorGLNative.endFrame(handle)

    override fun close() {
        if (handle != 0L) {
            YorGLNative.destroy(handle)
            handle = 0
        }
    }

    companion object {
        fun load(libraryName: String = "yorgl") = System.loadLibrary(libraryName)

        fun create(backend: BackendKind = BackendKind.Null): YorGL {
            val ptr = YorGLNative.create(backend.id)
            check(ptr != 0L) { "YorGL failed to create ${backend.name} backend" }
            return YorGL(ptr)
        }
    }
}
