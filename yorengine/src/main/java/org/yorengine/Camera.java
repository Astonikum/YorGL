package org.yorengine;

import java.util.Optional;

public final class Camera implements Component {
    private float fovYDegrees = 70.0f;
    private float aspectRatio = 16.0f / 9.0f;
    private float nearPlane = 0.05f;
    private float farPlane = 512.0f;
    private SceneObject object;

    @Override
    public void onAttach(SceneObject object) {
        this.object = object;
    }

    public Optional<SceneObject> object() {
        return Optional.ofNullable(object);
    }

    public float fovYDegrees() {
        return fovYDegrees;
    }

    public Camera fovYDegrees(float value) {
        if (value <= 0.0f || value >= 180.0f) {
            throw new IllegalArgumentException("Camera fovYDegrees must be between 0 and 180");
        }
        fovYDegrees = value;
        return this;
    }

    public float aspectRatio() {
        return aspectRatio;
    }

    public Camera aspectRatio(float value) {
        if (value <= 0.0f) {
            throw new IllegalArgumentException("Camera aspectRatio must be positive");
        }
        aspectRatio = value;
        return this;
    }

    public float nearPlane() {
        return nearPlane;
    }

    public Camera nearPlane(float value) {
        if (value <= 0.0f || value >= farPlane) {
            throw new IllegalArgumentException("Camera nearPlane must be positive and less than farPlane");
        }
        nearPlane = value;
        return this;
    }

    public float farPlane() {
        return farPlane;
    }

    public Camera farPlane(float value) {
        if (value <= nearPlane) {
            throw new IllegalArgumentException("Camera farPlane must be greater than nearPlane");
        }
        farPlane = value;
        return this;
    }
}
