package org.yorgl;

import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardCopyOption;
import java.util.Locale;

public final class YorGL implements AutoCloseable {
    private long handle;

    private YorGL(long handle) {
        this.handle = handle;
    }

    public static void load() {
        load("yorgl");
    }

    public static void load(String libraryName) {
        try {
            System.loadLibrary(libraryName);
            return;
        } catch (UnsatisfiedLinkError original) {
            loadBundledNative(libraryName, original);
        }
    }

    public static YorGL create() {
        return create(BackendKind.Null);
    }

    public static YorGL create(BackendKind backend) {
        long ptr = YorGLNative.create(backend.id);
        if (ptr == 0L) {
            throw new IllegalStateException("YorGL failed to create " + backend.name() + " backend");
        }
        return new YorGL(ptr);
    }

    public boolean isValid() {
        return handle != 0L && YorGLNative.isValid(handle);
    }

    public String getBackendName() {
        return YorGLNative.backendName(handle);
    }

    public boolean createSwapChain(long windowHandle, int width, int height) {
        return YorGLNative.createSwapChain(handle, windowHandle, width, height);
    }

    public void resize(int width, int height) {
        YorGLNative.resize(handle, width, height);
    }

    public void beginFrame() {
        YorGLNative.beginFrame(handle);
    }

    public void setViewport(float x, float y, float width, float height) {
        YorGLNative.setViewport(handle, x, y, width, height);
    }

    public void clearColor(float r, float g, float b, float a) {
        YorGLNative.clearColor(handle, r, g, b, a);
    }

    public void clearDepth(float depth) {
        YorGLNative.clearDepth(handle, depth);
    }

    public void endFrame() {
        YorGLNative.endFrame(handle);
    }

    public long createTexture(int width, int height, byte[] pixels) {
        return YorGLNative.createTexture(handle, width, height, pixels);
    }

    public void destroyTexture(long texture) {
        YorGLNative.destroyTexture(handle, texture);
    }

    public void guiBegin(int width, int height) {
        YorGLNative.guiBegin(handle, width, height);
    }

    public void guiDrawQuad(float x, float y, float w, float h, float u0, float v0, float u1, float v1, float r, float g, float b, float a) {
        YorGLNative.guiDrawQuad(handle, x, y, w, h, u0, v0, u1, v1, r, g, b, a);
    }

    public void guiDrawGradientQuad(float x, float y, float w, float h, float[] rgba16) {
        YorGLNative.guiDrawGradientQuad(handle, x, y, w, h, rgba16);
    }

    public void guiSetTexture(long texture) {
        YorGLNative.guiSetTexture(handle, texture);
    }

    public void guiSetScissor(float x, float y, float w, float h) {
        YorGLNative.guiSetScissor(handle, x, y, w, h);
    }

    public void guiClearScissor() {
        YorGLNative.guiClearScissor(handle);
    }

    public void guiSetSdfMode(boolean enabled) {
        YorGLNative.guiSetSdfMode(handle, enabled);
    }

    public void guiSetSdfParams(float edge, float softness, float weightBias) {
        YorGLNative.guiSetSdfParams(handle, edge, softness, weightBias);
    }

    public void guiBlurRect(float x, float y, float w, float h, int passes) {
        YorGLNative.guiBlurRect(handle, x, y, w, h, passes);
    }

    public void guiEnd() {
        YorGLNative.guiEnd(handle);
    }

    public void panoramaRender(long[] faces, float angle, int width, int height) {
        YorGLNative.panoramaRender(handle, faces, angle, width, height);
    }

    public void worldUploadMesh(float[] vertices, int floatCount) {
        YorGLNative.worldUploadMesh(handle, vertices, floatCount);
    }

    public void worldUploadSection(long sectionId, int x, int y, int z, float[] vertices, int floatCount) {
        YorGLNative.worldUploadSection(handle, sectionId, x, y, z, vertices, floatCount);
    }

    public void worldUploadSectionLayer(long sectionId, int x, int y, int z, int layer, float[] vertices, int floatCount) {
        YorGLNative.worldUploadSectionLayer(handle, sectionId, x, y, z, layer, vertices, floatCount);
    }

    public void worldRemoveSection(long sectionId) {
        YorGLNative.worldRemoveSection(handle, sectionId);
    }

    public void worldClearSections() {
        YorGLNative.worldClearSections(handle);
    }

    public void worldSetTexture(long texture) {
        YorGLNative.worldSetTexture(handle, texture);
    }

    public void worldSetSkyColor(float r, float g, float b) {
        YorGLNative.worldSetSkyColor(handle, r, g, b);
    }

    public void worldRender(float cameraX, float cameraY, float cameraZ, float dirX, float dirY, float dirZ, float fovYDegrees, float farPlane, int width, int height) {
        YorGLNative.worldRender(handle, cameraX, cameraY, cameraZ, dirX, dirY, dirZ, fovYDegrees, farPlane, width, height);
    }

    public long sdfFontCreate(byte[] ttfData, float fontSize) {
        return YorGLNative.sdfFontCreate(handle, ttfData, fontSize);
    }

    public void sdfFontDestroy(long font) {
        YorGLNative.sdfFontDestroy(handle, font);
    }

    public long sdfFontAtlas(long font) {
        return YorGLNative.sdfFontAtlas(handle, font);
    }

    public float[] sdfFontMetrics(long font) {
        return YorGLNative.sdfFontMetrics(handle, font);
    }

    public float[] sdfFontGlyph(long font, int codepoint) {
        return YorGLNative.sdfFontGlyph(handle, font, codepoint);
    }

    public float sdfFontKerning(long font, int leftCodepoint, int rightCodepoint) {
        return YorGLNative.sdfFontKerning(handle, font, leftCodepoint, rightCodepoint);
    }

    @Override
    public void close() {
        if (handle != 0L) {
            YorGLNative.destroy(handle);
            handle = 0L;
        }
    }

    private static void loadBundledNative(String libraryName, UnsatisfiedLinkError original) {
        String resource = nativeResourceName(libraryName);
        try (InputStream input = YorGL.class.getResourceAsStream(resource)) {
            if (input == null) {
                throw original;
            }
            String suffix = resource.substring(resource.lastIndexOf('.'));
            Path file = Files.createTempFile("yorgl-", suffix);
            file.toFile().deleteOnExit();
            Files.copy(input, file, StandardCopyOption.REPLACE_EXISTING);
            System.load(file.toAbsolutePath().toString());
        } catch (IOException e) {
            original.addSuppressed(e);
            throw original;
        }
    }

    private static String nativeResourceName(String libraryName) {
        String os = System.getProperty("os.name").toLowerCase(Locale.ROOT);
        String arch = System.getProperty("os.arch").toLowerCase(Locale.ROOT);
        String platform;
        if (os.contains("win") && (arch.contains("64") || arch.contains("amd64"))) {
            platform = "windows-x64";
        } else {
            throw new IllegalStateException("No bundled YorGL native for os=" + os + " arch=" + arch);
        }
        String file = os.contains("win") ? libraryName + ".dll" : "lib" + libraryName + ".so";
        return "/org/yorgl/native/" + platform + "/" + file;
    }
}
