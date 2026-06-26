package org.yorgl;

final class YorGLNative {
    private YorGLNative() {
    }

    static native long create(int backend);
    static native void destroy(long ptr);
    static native boolean isValid(long ptr);
    static native String backendName(long ptr);
    static native int[] capabilities(long ptr);
    static native boolean createSwapChain(long ptr, long windowHandle, int width, int height);
    static native void resize(long ptr, int width, int height);
    static native void beginFrame(long ptr);
    static native void setViewport(long ptr, float x, float y, float width, float height);
    static native void clearColor(long ptr, float r, float g, float b, float a);
    static native void clearDepth(long ptr, float depth);
    static native void setPresentMode(long ptr, int mode);
    static native void endFrame(long ptr);
    static native long createTexture(long ptr, int width, int height, byte[] pixels);
    static native void destroyTexture(long ptr, long texture);
    static native void guiBegin(long ptr, int width, int height);
    static native void guiDrawQuad(long ptr, float x, float y, float w, float h, float u0, float v0, float u1, float v1, float r, float g, float b, float a);
    static native void guiDrawGradientQuad(long ptr, float x, float y, float w, float h, float[] rgba16);
    static native void guiSetTexture(long ptr, long texture);
    static native void guiSetScissor(long ptr, float x, float y, float w, float h);
    static native void guiClearScissor(long ptr);
    static native void guiSetSdfMode(long ptr, boolean enabled);
    static native void guiSetSdfParams(long ptr, float edge, float softness, float weightBias);
    static native void guiBlurRect(long ptr, float x, float y, float w, float h, int passes);
    static native void guiEnd(long ptr);
    static native void panoramaRender(long ptr, long[] faces, float angle, int width, int height);
    static native void worldUploadMesh(long ptr, float[] vertices, int floatCount);
    static native void worldUploadSection(long ptr, long sectionId, int x, int y, int z, float[] vertices, int floatCount);
    static native void worldUploadSectionLayer(long ptr, long sectionId, int x, int y, int z, int layer, float[] vertices, int floatCount);
    static native void worldRemoveSection(long ptr, long sectionId);
    static native void worldClearSections(long ptr);
    static native void worldSetTexture(long ptr, long texture);
    static native void worldSetSkyColor(long ptr, float r, float g, float b);
    static native void worldRender(long ptr, float cameraX, float cameraY, float cameraZ, float dirX, float dirY, float dirZ, float fovYDegrees, float farPlane, int width, int height);
    static native long sdfFontCreate(long ptr, byte[] ttfData, float fontSize);
    static native void sdfFontDestroy(long ptr, long font);
    static native long sdfFontAtlas(long ptr, long font);
    static native float[] sdfFontMetrics(long ptr, long font);
    static native float[] sdfFontGlyph(long ptr, long font, int codepoint);
    static native float sdfFontKerning(long ptr, long font, int leftCodepoint, int rightCodepoint);
    static native float sdfFontTextWidth(long ptr, long font, String text, float scale);
    static native float sdfFontLineHeight(long ptr, long font, float scale);
    static native void sdfFontDrawText(long ptr, long font, String text, float x, float y, float scale, float r, float g, float b, float a, float weight, boolean shadow);
}
