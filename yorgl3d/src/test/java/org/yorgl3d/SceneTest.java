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

        long version = scene.version();
        scene.objects().get(0).transform().position(0.0f, 0.0f, 0.0f);
        if (scene.version() <= version) {
            throw new AssertionError("scene version did not advance after transform change");
        }

        version = scene.version();
        SceneObject object = scene.objects().get(0).property("minecraft:id", "zombie");
        if (!"zombie".equals(object.property("minecraft:id").orElseThrow())) {
            throw new AssertionError("scene object property missing");
        }
        if (scene.version() <= version) {
            throw new AssertionError("scene version did not advance after property change");
        }

        MeshComponent mesh = object.component(MeshComponent.class).orElseThrow();
        version = scene.version();
        if (!object.remove(mesh) || object.component(MeshComponent.class).isPresent()) {
            throw new AssertionError("scene object component removal failed");
        }
        if (scene.version() <= version) {
            throw new AssertionError("scene version did not advance after component removal");
        }

        object.add(mesh);
        if (!object.remove(MeshComponent.class) || object.component(MeshComponent.class).isPresent()) {
            throw new AssertionError("scene object typed component removal failed");
        }

        Component selfRemoving = new Component() {
            @Override
            public void update(SceneObject updated, float deltaSeconds) {
                updated.remove(this);
            }
        };
        object.add(selfRemoving);
        scene.update(0.016f);
        if (object.component(selfRemoving.getClass()).isPresent()) {
            throw new AssertionError("scene object component self-removal failed");
        }

        version = scene.version();
        if (!scene.removeObject(object) || !scene.objects().isEmpty()) {
            throw new AssertionError("scene object removal failed");
        }
        if (scene.version() <= version) {
            throw new AssertionError("scene version did not advance after object removal");
        }
    }

    private static void assertClose(float expected, float actual) {
        if (Math.abs(expected - actual) > 0.0001f) {
            throw new AssertionError("expected " + expected + " got " + actual);
        }
    }
}
