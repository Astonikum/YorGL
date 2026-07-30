package org.yorengine;

public interface Component {
    default void onAttach(SceneObject object) {
    }

    default void update(SceneObject object, float deltaSeconds) {
    }
}
