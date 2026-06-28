package org.yorgl3d;

public final class Camera implements Component {
    private float fovYDegrees = 70.0f;
    private float farPlane = 512.0f;

    public float fovYDegrees() {
        return fovYDegrees;
    }

    public Camera fovYDegrees(float value) {
        fovYDegrees = value;
        return this;
    }

    public float farPlane() {
        return farPlane;
    }

    public Camera farPlane(float value) {
        farPlane = value;
        return this;
    }
}
