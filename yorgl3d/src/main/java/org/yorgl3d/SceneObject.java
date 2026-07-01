package org.yorgl3d;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

public final class SceneObject {
    private final Transform transform = new Transform();
    private final List<Component> components = new ArrayList<>();
    private final List<SceneObject> children = new ArrayList<>();
    private final Map<String, Object> properties = new HashMap<>();
    private Runnable onChange = () -> {};
    private SceneObject parent;
    private boolean active = true;

    public SceneObject() {
        transform.onChange(this::touch);
    }

    public Transform transform() {
        return transform;
    }

    public Optional<SceneObject> parent() {
        return Optional.ofNullable(parent);
    }

    public List<SceneObject> children() {
        return Collections.unmodifiableList(children);
    }

    public SceneObject addChild(SceneObject child) {
        if (child == null) {
            throw new IllegalArgumentException("Child object must not be null");
        }
        if (child == this || isAncestorOf(child)) {
            throw new IllegalArgumentException("SceneObject hierarchy cannot contain cycles");
        }
        if (child.parent == this) return this;
        if (child.parent != null) {
            child.parent.children.remove(child);
            child.parent.touch();
        }
        child.parent = this;
        children.add(child);
        touch();
        child.touch();
        return this;
    }

    public boolean removeChild(SceneObject child) {
        boolean removed = children.remove(child);
        if (removed) {
            child.parent = null;
            touch();
            child.touch();
        }
        return removed;
    }

    public float[] worldMatrix() {
        float[] local = transform.matrix();
        if (parent == null) return local;
        return multiply(parent.worldMatrix(), local);
    }

    public boolean active() {
        return active;
    }

    public SceneObject active(boolean active) {
        if (this.active == active) return this;
        this.active = active;
        touch();
        return this;
    }

    public SceneObject add(Component component) {
        components.add(component);
        component.onAttach(this);
        touch();
        return this;
    }

    public <T extends Component> Optional<T> component(Class<T> type) {
        return components.stream().filter(type::isInstance).map(type::cast).findFirst();
    }

    public boolean remove(Component component) {
        boolean removed = components.remove(component);
        if (removed) {
            touch();
        }
        return removed;
    }

    public <T extends Component> boolean remove(Class<T> type) {
        for (Component component : components) {
            if (type.isInstance(component)) {
                return remove(component);
            }
        }
        return false;
    }

    public List<Component> components() {
        return Collections.unmodifiableList(components);
    }

    public SceneObject property(String key, Object value) {
        if (key == null || key.isBlank()) {
            throw new IllegalArgumentException("Property key must not be blank");
        }
        if (value == null) {
            properties.remove(key);
        } else {
            properties.put(key, value);
        }
        touch();
        return this;
    }

    public Optional<Object> property(String key) {
        return Optional.ofNullable(properties.get(key));
    }

    public Map<String, Object> properties() {
        return Collections.unmodifiableMap(properties);
    }

    void update(float deltaSeconds) {
        if (!active) return;
        for (Component component : new ArrayList<>(components)) {
            component.update(this, deltaSeconds);
        }
    }

    void onChange(Runnable onChange) {
        this.onChange = onChange;
    }

    private boolean isAncestorOf(SceneObject object) {
        SceneObject current = this;
        while (current != null) {
            if (current == object) return true;
            current = current.parent;
        }
        return false;
    }

    private static float[] multiply(float[] a, float[] b) {
        float[] out = new float[16];
        for (int col = 0; col < 4; col++) {
            for (int row = 0; row < 4; row++) {
                out[col * 4 + row] =
                    a[0 * 4 + row] * b[col * 4 + 0] +
                    a[1 * 4 + row] * b[col * 4 + 1] +
                    a[2 * 4 + row] * b[col * 4 + 2] +
                    a[3 * 4 + row] * b[col * 4 + 3];
            }
        }
        return out;
    }

    private void touch() {
        onChange.run();
    }
}
