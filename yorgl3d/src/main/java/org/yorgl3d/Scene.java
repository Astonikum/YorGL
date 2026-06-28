package org.yorgl3d;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public final class Scene {
    private final List<SceneObject> objects = new ArrayList<>();

    public SceneObject createObject() {
        SceneObject object = new SceneObject();
        objects.add(object);
        return object;
    }

    public List<SceneObject> objects() {
        return Collections.unmodifiableList(objects);
    }

    public void update(float deltaSeconds) {
        for (SceneObject object : objects) {
            object.update(deltaSeconds);
        }
    }

    public float[] bakeWorldVertices() {
        FloatList out = new FloatList();
        for (SceneObject object : objects) {
            if (!object.active()) continue;
            object.component(MeshComponent.class).ifPresent(mesh -> mesh.appendWorldVertices(object.transform(), out));
        }
        return out.toArray();
    }
}
