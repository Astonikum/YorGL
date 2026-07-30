package org.yorengine;

public final class NativeSceneTest {
    public static void main(String[] args) {
        try (NativeScene scene = new NativeScene()) {
            NativeScene.EntityId parent = scene.createEntity();
            NativeScene.EntityId child = scene.createEntity();
            scene.setParent(child, parent);
            scene.setTransform(parent, new NativeScene.Transform(
                new NativeScene.Vec3(10.0f, 2.0f, -3.0f),
                new NativeScene.Quaternion(0.0f, 0.0f, 0.0f, 1.0f),
                new NativeScene.Vec3(1.0f, 1.0f, 1.0f)));
            scene.setTransform(child, new NativeScene.Transform(
                new NativeScene.Vec3(1.0f, 0.0f, 0.0f),
                new NativeScene.Quaternion(0.0f, 0.0f, 0.0f, 1.0f),
                new NativeScene.Vec3(2.0f, 2.0f, 2.0f)));

            float[] matrix = scene.worldMatrix(child).values();
            checkNear(11.0f, matrix[12]);
            checkNear(2.0f, matrix[13]);
            checkNear(-3.0f, matrix[14]);

            scene.addMesh(child);
            scene.addCamera(parent);
            scene.setCamera(parent, new NativeScene.CameraState(70.0f, 16.0f / 9.0f, 0.1f, 1024.0f));
            scene.addLight(parent, 2);
            scene.setLight(parent, new NativeScene.LightState(2, 0.8f, 0.9f, 1.0f, 2.0f, 32.0f, 15.0f, 45.0f));

            if (!scene.isAlive(child)) throw new AssertionError("native child entity is not alive");
            scene.destroyEntity(parent);
            if (scene.isAlive(parent) || scene.isAlive(child)) throw new AssertionError("native cascade destroy failed");
        }
    }

    private static void checkNear(float expected, float actual) {
        if (Math.abs(expected - actual) > 0.0001f) {
            throw new AssertionError("expected " + expected + " got " + actual);
        }
    }
}
