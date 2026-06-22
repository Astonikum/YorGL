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
    external fun createTexture(ptr: Long, width: Int, height: Int, pixels: ByteArray): Long
    external fun destroyTexture(ptr: Long, texture: Long)
    external fun guiBegin(ptr: Long, width: Int, height: Int)
    external fun guiDrawQuad(ptr: Long, x: Float, y: Float, w: Float, h: Float, u0: Float, v0: Float, u1: Float, v1: Float, r: Float, g: Float, b: Float, a: Float)
    external fun guiDrawGradientQuad(ptr: Long, x: Float, y: Float, w: Float, h: Float, rgba16: FloatArray)
    external fun guiSetTexture(ptr: Long, texture: Long)
    external fun guiSetScissor(ptr: Long, x: Float, y: Float, w: Float, h: Float)
    external fun guiClearScissor(ptr: Long)
    external fun guiSetSdfMode(ptr: Long, enabled: Boolean)
    external fun guiSetSdfParams(ptr: Long, edge: Float, softness: Float, weightBias: Float)
    external fun guiBlurRect(ptr: Long, x: Float, y: Float, w: Float, h: Float, passes: Int)
    external fun guiEnd(ptr: Long)
    external fun panoramaRender(ptr: Long, faces: LongArray, angle: Float, width: Int, height: Int)
    external fun worldUploadMesh(ptr: Long, vertices: FloatArray, floatCount: Int)
    external fun worldUploadSection(ptr: Long, sectionId: Long, x: Int, y: Int, z: Int, vertices: FloatArray, floatCount: Int)
    external fun worldUploadSectionLayer(ptr: Long, sectionId: Long, x: Int, y: Int, z: Int, layer: Int, vertices: FloatArray, floatCount: Int)
    external fun worldRemoveSection(ptr: Long, sectionId: Long)
    external fun worldClearSections(ptr: Long)
    external fun worldSetTexture(ptr: Long, texture: Long)
    external fun worldSetSkyColor(ptr: Long, r: Float, g: Float, b: Float)
    external fun worldRender(ptr: Long, cameraX: Float, cameraY: Float, cameraZ: Float, dirX: Float, dirY: Float, dirZ: Float, fovYDegrees: Float, farPlane: Float, width: Int, height: Int)
    external fun sdfFontCreate(ptr: Long, ttfData: ByteArray, fontSize: Float): Long
    external fun sdfFontDestroy(ptr: Long, font: Long)
    external fun sdfFontAtlas(ptr: Long, font: Long): Long
    external fun sdfFontMetrics(ptr: Long, font: Long): FloatArray?
    external fun sdfFontGlyph(ptr: Long, font: Long, codepoint: Int): FloatArray?
    external fun sdfFontKerning(ptr: Long, font: Long, leftCodepoint: Int, rightCodepoint: Int): Float
}
