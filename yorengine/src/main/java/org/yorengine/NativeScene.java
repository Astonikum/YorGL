package org.yorengine;

import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.Path;

/**
 * Thin JVM adapter over the native C++ YorEngine scene API.
 *
 * <p>This class owns only the native handle and value conversion. Entity,
 * component, transform, and lifetime state remain in the C++ scene.</p>
 */
public final class NativeScene implements AutoCloseable {
    private static final String NATIVE_RESOURCE = "/org/yorengine/native/windows-x64/yorengine_api.dll";

    static {
        loadNativeLibrary();
    }

    private long handle;

    public NativeScene() {
        handle = nativeCreate();
        if (handle == 0L) {
            throw new NativeSceneException(7, "Native YorEngine scene allocation failed");
        }
    }

    public EntityId createEntity() {
        return EntityId.fromPacked(nativeCreateEntity(requireHandle()));
    }

    public void destroyEntity(EntityId entity) {
        nativeDestroyEntity(requireHandle(), entity.packed());
    }

    public boolean isAlive(EntityId entity) {
        return nativeIsAlive(requireHandle(), entity.packed());
    }

    public void setParent(EntityId child, EntityId parent) {
        nativeSetParent(requireHandle(), child.packed(), parent.packed());
    }

    public void setTransform(EntityId entity, Transform transform) {
        nativeSetTransform(requireHandle(), entity.packed(), transform.toArray());
    }

    public Matrix4 worldMatrix(EntityId entity) {
        return new Matrix4(nativeWorldMatrix(requireHandle(), entity.packed()));
    }

    public void addMesh(EntityId entity) {
        nativeAddMesh(requireHandle(), entity.packed());
    }

    public void addCamera(EntityId entity) {
        nativeAddCamera(requireHandle(), entity.packed());
    }

    public void setCamera(EntityId entity, CameraState camera) {
        nativeSetCamera(requireHandle(), entity.packed(), camera.fovYDegrees(), camera.aspectRatio(), camera.nearPlane(), camera.farPlane());
    }

    public void addLight(EntityId entity, int kind) {
        nativeAddLight(requireHandle(), entity.packed(), kind);
    }

    public void setLight(EntityId entity, LightState light) {
        nativeSetLight(requireHandle(), entity.packed(), light.kind(), light.red(), light.green(), light.blue(), light.intensity(), light.range(), light.innerConeDegrees(), light.outerConeDegrees());
    }

    @Override
    public void close() {
        if (handle != 0L) {
            nativeDestroy(handle);
            handle = 0L;
        }
    }

    private long requireHandle() {
        if (handle == 0L) throw new IllegalStateException("NativeScene is closed");
        return handle;
    }

    private static void loadNativeLibrary() {
        if (!System.getProperty("os.name", "").toLowerCase().contains("win")) {
            throw new UnsupportedOperationException("NativeScene currently ships a Windows x64 native artifact");
        }
        try (InputStream input = NativeScene.class.getResourceAsStream(NATIVE_RESOURCE)) {
            if (input == null) throw new IOException("Missing " + NATIVE_RESOURCE);
            Path extracted = Files.createTempFile("yorengine-api-", ".dll");
            Files.copy(input, extracted, java.nio.file.StandardCopyOption.REPLACE_EXISTING);
            extracted.toFile().deleteOnExit();
            System.load(extracted.toAbsolutePath().toString());
        } catch (IOException error) {
            throw new ExceptionInInitializerError(error);
        }
    }

    private static native long nativeCreate();
    private static native void nativeDestroy(long handle);
    private static native long nativeCreateEntity(long handle);
    private static native void nativeDestroyEntity(long handle, long entity);
    private static native boolean nativeIsAlive(long handle, long entity);
    private static native void nativeSetParent(long handle, long child, long parent);
    private static native void nativeSetTransform(long handle, long entity, float[] values);
    private static native float[] nativeWorldMatrix(long handle, long entity);
    private static native void nativeAddMesh(long handle, long entity);
    private static native void nativeAddCamera(long handle, long entity);
    private static native void nativeSetCamera(long handle, long entity, float fov, float aspect, float nearPlane, float farPlane);
    private static native void nativeAddLight(long handle, long entity, int kind);
    private static native void nativeSetLight(long handle, long entity, int kind, float red, float green, float blue, float intensity, float range, float innerCone, float outerCone);

    public record EntityId(int index, int generation) {
        private long packed() {
            return ((long) generation << 32) | (index & 0xffffffffL);
        }

        private static EntityId fromPacked(long packed) {
            return new EntityId((int) packed, (int) (packed >>> 32));
        }
    }

    public record Vec3(float x, float y, float z) {}

    public record Quaternion(float x, float y, float z, float w) {}

    public record Transform(Vec3 position, Quaternion rotation, Vec3 scale) {
        private float[] toArray() {
            return new float[] {
                position.x(), position.y(), position.z(),
                rotation.x(), rotation.y(), rotation.z(), rotation.w(),
                scale.x(), scale.y(), scale.z(),
            };
        }
    }

    public record Matrix4(float[] values) {
        public Matrix4 {
            if (values == null || values.length != 16) throw new IllegalArgumentException("Matrix4 requires 16 values");
            values = values.clone();
        }
    }

    public record CameraState(float fovYDegrees, float aspectRatio, float nearPlane, float farPlane) {}

    public record LightState(int kind, float red, float green, float blue, float intensity, float range, float innerConeDegrees, float outerConeDegrees) {}
}
