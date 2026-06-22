package org.yorgl

import java.nio.file.Files

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
    fun createTexture(width: Int, height: Int, pixels: ByteArray): Long =
        YorGLNative.createTexture(handle, width, height, pixels)
    fun destroyTexture(texture: Long) = YorGLNative.destroyTexture(handle, texture)

    fun guiBegin(width: Int, height: Int) = YorGLNative.guiBegin(handle, width, height)
    fun guiDrawQuad(x: Float, y: Float, w: Float, h: Float, u0: Float, v0: Float, u1: Float, v1: Float, r: Float, g: Float, b: Float, a: Float) =
        YorGLNative.guiDrawQuad(handle, x, y, w, h, u0, v0, u1, v1, r, g, b, a)
    fun guiDrawGradientQuad(x: Float, y: Float, w: Float, h: Float, rgba16: FloatArray) =
        YorGLNative.guiDrawGradientQuad(handle, x, y, w, h, rgba16)
    fun guiSetTexture(texture: Long) = YorGLNative.guiSetTexture(handle, texture)
    fun guiSetScissor(x: Float, y: Float, w: Float, h: Float) = YorGLNative.guiSetScissor(handle, x, y, w, h)
    fun guiClearScissor() = YorGLNative.guiClearScissor(handle)
    fun guiSetSdfMode(enabled: Boolean) = YorGLNative.guiSetSdfMode(handle, enabled)
    fun guiSetSdfParams(edge: Float, softness: Float, weightBias: Float) =
        YorGLNative.guiSetSdfParams(handle, edge, softness, weightBias)
    fun guiBlurRect(x: Float, y: Float, w: Float, h: Float, passes: Int) =
        YorGLNative.guiBlurRect(handle, x, y, w, h, passes)
    fun guiEnd() = YorGLNative.guiEnd(handle)

    fun panoramaRender(faces: LongArray, angle: Float, width: Int, height: Int) =
        YorGLNative.panoramaRender(handle, faces, angle, width, height)

    fun worldUploadMesh(vertices: FloatArray, floatCount: Int) =
        YorGLNative.worldUploadMesh(handle, vertices, floatCount)
    fun worldUploadSection(sectionId: Long, x: Int, y: Int, z: Int, vertices: FloatArray, floatCount: Int) =
        YorGLNative.worldUploadSection(handle, sectionId, x, y, z, vertices, floatCount)
    fun worldUploadSectionLayer(sectionId: Long, x: Int, y: Int, z: Int, layer: Int, vertices: FloatArray, floatCount: Int) =
        YorGLNative.worldUploadSectionLayer(handle, sectionId, x, y, z, layer, vertices, floatCount)
    fun worldRemoveSection(sectionId: Long) = YorGLNative.worldRemoveSection(handle, sectionId)
    fun worldClearSections() = YorGLNative.worldClearSections(handle)
    fun worldSetTexture(texture: Long) = YorGLNative.worldSetTexture(handle, texture)
    fun worldSetSkyColor(r: Float, g: Float, b: Float) = YorGLNative.worldSetSkyColor(handle, r, g, b)
    fun worldRender(cameraX: Float, cameraY: Float, cameraZ: Float, dirX: Float, dirY: Float, dirZ: Float, fovYDegrees: Float, farPlane: Float, width: Int, height: Int) =
        YorGLNative.worldRender(handle, cameraX, cameraY, cameraZ, dirX, dirY, dirZ, fovYDegrees, farPlane, width, height)

    fun sdfFontCreate(ttfData: ByteArray, fontSize: Float): Long = YorGLNative.sdfFontCreate(handle, ttfData, fontSize)
    fun sdfFontDestroy(font: Long) = YorGLNative.sdfFontDestroy(handle, font)
    fun sdfFontAtlas(font: Long): Long = YorGLNative.sdfFontAtlas(handle, font)
    fun sdfFontMetrics(font: Long): FloatArray? = YorGLNative.sdfFontMetrics(handle, font)
    fun sdfFontGlyph(font: Long, codepoint: Int): FloatArray? = YorGLNative.sdfFontGlyph(handle, font, codepoint)
    fun sdfFontKerning(font: Long, leftCodepoint: Int, rightCodepoint: Int): Float =
        YorGLNative.sdfFontKerning(handle, font, leftCodepoint, rightCodepoint)

    override fun close() {
        if (handle != 0L) {
            YorGLNative.destroy(handle)
            handle = 0
        }
    }

    companion object {
        fun load(libraryName: String = "yorgl") {
            runCatching { System.loadLibrary(libraryName) }.getOrElse { original ->
                val resource = nativeResourceName(libraryName)
                val stream = YorGL::class.java.getResourceAsStream(resource) ?: throw original
                val suffix = resource.substringAfterLast('.', ".dll")
                val file = Files.createTempFile("yorgl-", suffix)
                file.toFile().deleteOnExit()
                stream.use { input -> Files.copy(input, file, java.nio.file.StandardCopyOption.REPLACE_EXISTING) }
                System.load(file.toAbsolutePath().toString())
            }
        }

        fun create(backend: BackendKind = BackendKind.Null): YorGL {
            val ptr = YorGLNative.create(backend.id)
            check(ptr != 0L) { "YorGL failed to create ${backend.name} backend" }
            return YorGL(ptr)
        }

        private fun nativeResourceName(libraryName: String): String {
            val os = System.getProperty("os.name").lowercase()
            val arch = System.getProperty("os.arch").lowercase()
            val platform = when {
                os.contains("win") && (arch.contains("64") || arch.contains("amd64")) -> "windows-x64"
                else -> error("No bundled YorGL native for os=$os arch=$arch")
            }
            val file = if (os.contains("win")) "$libraryName.dll" else "lib$libraryName.so"
            return "/org/yorgl/native/$platform/$file"
        }
    }
}
