package org.yorgl3d;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Optional;

public final class SceneObject {
    private final Transform transform = new Transform();
    private final List<Component> components = new ArrayList<>();
    private boolean active = true;

    public Transform transform() {
        return transform;
    }

    public boolean active() {
        return active;
    }

    public SceneObject active(boolean active) {
        this.active = active;
        return this;
    }

    public SceneObject add(Component component) {
        components.add(component);
        component.onAttach(this);
        return this;
    }

    public <T extends Component> Optional<T> component(Class<T> type) {
        return components.stream().filter(type::isInstance).map(type::cast).findFirst();
    }

    public List<Component> components() {
        return Collections.unmodifiableList(components);
    }

    void update(float deltaSeconds) {
        if (!active) return;
        for (Component component : components) {
            component.update(this, deltaSeconds);
        }
    }
}
