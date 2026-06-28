package org.yorgl3d;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public final class Scene {
    private final List<SceneObject> objects = new ArrayList<>();
    private long version = 1;

    public SceneObject createObject() {
        SceneObject object = new SceneObject();
        object.onChange(this::touch);
        objects.add(object);
        touch();
        return object;
    }

    public List<SceneObject> objects() {
        return Collections.unmodifiableList(objects);
    }

    public boolean removeObject(SceneObject object) {
        boolean removed = objects.remove(object);
        if (removed) {
            object.onChange(() -> {});
            touch();
        }
        return removed;
    }

    public void update(float deltaSeconds) {
        for (SceneObject object : objects) {
            object.update(deltaSeconds);
        }
    }

    public long version() {
        return version;
    }

    public float[] bakeWorldVertices() {
        FloatList out = new FloatList();
        for (SceneObject object : objects) {
            if (!object.active()) continue;
            object.component(MeshComponent.class).ifPresent(mesh -> mesh.appendWorldVertices(object.transform(), out));
        }
        return out.toArray();
    }

    private void touch() {
        version++;
    }
}
