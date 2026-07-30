package org.yorengine;

import java.util.Optional;

public final class Light implements Component {
    public enum Kind { DIRECTIONAL, POINT, SPOT }

    private final Kind kind;
    private float r = 1.0f;
    private float g = 1.0f;
    private float b = 1.0f;
    private float intensity = 1.0f;
    private float range = 16.0f;
    private float innerConeDegrees = 20.0f;
    private float outerConeDegrees = 35.0f;
    private SceneObject object;

    public Light(Kind kind) {
        if (kind == null) {
            throw new IllegalArgumentException("Light kind must not be null");
        }
        this.kind = kind;
    }

    @Override
    public void onAttach(SceneObject object) {
        this.object = object;
    }

    public Optional<SceneObject> object() {
        return Optional.ofNullable(object);
    }

    public Kind kind() {
        return kind;
    }

    public Light color(float r, float g, float b) {
        this.r = r;
        this.g = g;
        this.b = b;
        return this;
    }

    public float red() {
        return r;
    }

    public float green() {
        return g;
    }

    public float blue() {
        return b;
    }

    public Light intensity(float intensity) {
        if (intensity < 0.0f) {
            throw new IllegalArgumentException("Light intensity must not be negative");
        }
        this.intensity = intensity;
        return this;
    }

    public float intensity() {
        return intensity;
    }

    public Light range(float range) {
        if (range < 0.0f) {
            throw new IllegalArgumentException("Light range must not be negative");
        }
        this.range = range;
        return this;
    }

    public float range() {
        return range;
    }

    public Light cone(float innerDegrees, float outerDegrees) {
        if (innerDegrees < 0.0f || outerDegrees < innerDegrees || outerDegrees > 180.0f) {
            throw new IllegalArgumentException("Light cone must satisfy 0 <= inner <= outer <= 180");
        }
        innerConeDegrees = innerDegrees;
        outerConeDegrees = outerDegrees;
        return this;
    }

    public float innerConeDegrees() {
        return innerConeDegrees;
    }

    public float outerConeDegrees() {
        return outerConeDegrees;
    }

    public float[] rgba() {
        return new float[] {r * intensity, g * intensity, b * intensity, 1.0f};
    }
}
