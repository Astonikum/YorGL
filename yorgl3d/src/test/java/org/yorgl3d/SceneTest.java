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

        SceneObject parent = scene.createObject();
        parent.transform().position(10.0f, 0.0f, 0.0f);
        SceneObject child = scene.createObject(parent);
        child.transform().position(1.0f, 0.0f, 0.0f);
        Material material = new Material().name("debug").color(0.5f, 0.25f, 1.0f, 1.0f);
        MeshComponent childMesh = new MeshComponent(new float[] {
            2, 0, 0, 1, 1, 1, 1, 0, 0
        }).material(material);
        child.add(childMesh);
        baked = scene.bakeWorldVertices();
        assertClose(13.0f, baked[baked.length - 9]);
        if (child.parent().orElseThrow() != parent || !parent.children().contains(child)) {
            throw new AssertionError("scene object parent/child link missing");
        }
        if (childMesh.material() != material) {
            throw new AssertionError("mesh material reference missing");
        }
        if (!parent.removeChild(child) || child.parent().isPresent()) {
            throw new AssertionError("scene object child removal failed");
        }
        scene.removeObject(child);
        scene.removeObject(parent);

        Camera camera = new Camera()
            .aspectRatio(4.0f / 3.0f)
            .nearPlane(0.1f)
            .farPlane(1024.0f)
            .fovYDegrees(75.0f);
        Light light = new Light(Light.Kind.SPOT)
            .color(0.8f, 0.9f, 1.0f)
            .intensity(2.0f)
            .range(32.0f)
            .cone(15.0f, 45.0f);
        SceneObject cameraObject = scene.createObject(camera);
        SceneObject lightObject = scene.createObject(light);
        if (scene.activeCamera().orElseThrow() != camera || camera.object().orElseThrow() != cameraObject) {
            throw new AssertionError("scene camera registration failed");
        }
        if (!scene.lights().contains(light) || light.object().orElseThrow() != lightObject) {
            throw new AssertionError("scene light registration failed");
        }
        if (Math.abs(light.rgba()[2] - 2.0f) > 0.0001f || light.outerConeDegrees() != 45.0f) {
            throw new AssertionError("scene light data failed");
        }
        if (cameraObject.componentOrNull(Camera.class) != camera || cameraObject.requireComponent(Camera.class) != camera) {
            throw new AssertionError("kotlin-friendly component lookup failed");
        }
        scene.removeObject(cameraObject);
        scene.removeObject(lightObject);

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
