package org.yorengine;

public final class Material {
    private String name = "";
    private float r = 1.0f;
    private float g = 1.0f;
    private float b = 1.0f;
    private float a = 1.0f;

    public String name() {
        return name;
    }

    public Material name(String value) {
        name = value == null ? "" : value;
        return this;
    }

    public Material color(float r, float g, float b, float a) {
        this.r = r;
        this.g = g;
        this.b = b;
        this.a = a;
        return this;
    }

    public float[] rgba() {
        return new float[] {r, g, b, a};
    }
}
