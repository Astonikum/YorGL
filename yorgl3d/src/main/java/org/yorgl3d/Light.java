package org.yorgl3d;

public final class Light implements Component {
    public enum Kind { DIRECTIONAL, POINT }

    private final Kind kind;
    private float r = 1.0f;
    private float g = 1.0f;
    private float b = 1.0f;
    private float intensity = 1.0f;

    public Light(Kind kind) {
        this.kind = kind;
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

    public Light intensity(float intensity) {
        this.intensity = intensity;
        return this;
    }

    public float[] rgba() {
        return new float[] {r * intensity, g * intensity, b * intensity, 1.0f};
    }
}
