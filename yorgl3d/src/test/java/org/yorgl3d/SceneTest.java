package org.yorgl3d;

public final class SceneTest {
    public static void main(String[] args) {
        Scene scene = new Scene();
        scene.createObject()
            .transform()
            .position(10.0f, 2.0f, -3.0f)
            .scale(2.0f, 2.0f, 2.0f);
        scene.objects().get(0).add(new MeshComponent(new float[] {
            1, 1, 1, 1, 1, 1, 1, 0, 0
        }));

        float[] baked = scene.bakeWorldVertices();
        assertClose(12.0f, baked[0]);
        assertClose(4.0f, baked[1]);
        assertClose(-1.0f, baked[2]);
        assertClose(1.0f, baked[3]);
    }

    private static void assertClose(float expected, float actual) {
        if (Math.abs(expected - actual) > 0.0001f) {
            throw new AssertionError("expected " + expected + " got " + actual);
        }
    }
}
