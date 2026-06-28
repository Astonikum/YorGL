package org.yorgl3d;

public final class Transform {
    private Vec3 position = Vec3.ZERO;
    private Vec3 rotationRadians = Vec3.ZERO;
    private Vec3 scale = Vec3.ONE;

    public Vec3 position() {
        return position;
    }

    public Transform position(float x, float y, float z) {
        position = new Vec3(x, y, z);
        return this;
    }

    public Vec3 rotationRadians() {
        return rotationRadians;
    }

    public Transform rotationRadians(float x, float y, float z) {
        rotationRadians = new Vec3(x, y, z);
        return this;
    }

    public Vec3 scale() {
        return scale;
    }

    public Transform scale(float x, float y, float z) {
        scale = new Vec3(x, y, z);
        return this;
    }

    public float[] matrix() {
        float cx = (float) Math.cos(rotationRadians.x());
        float sx = (float) Math.sin(rotationRadians.x());
        float cy = (float) Math.cos(rotationRadians.y());
        float sy = (float) Math.sin(rotationRadians.y());
        float cz = (float) Math.cos(rotationRadians.z());
        float sz = (float) Math.sin(rotationRadians.z());

        float m00 = cy * cz;
        float m01 = sx * sy * cz - cx * sz;
        float m02 = cx * sy * cz + sx * sz;
        float m10 = cy * sz;
        float m11 = sx * sy * sz + cx * cz;
        float m12 = cx * sy * sz - sx * cz;
        float m20 = -sy;
        float m21 = sx * cy;
        float m22 = cx * cy;

        return new float[] {
            m00 * scale.x(), m01 * scale.y(), m02 * scale.z(), 0.0f,
            m10 * scale.x(), m11 * scale.y(), m12 * scale.z(), 0.0f,
            m20 * scale.x(), m21 * scale.y(), m22 * scale.z(), 0.0f,
            position.x(), position.y(), position.z(), 1.0f
        };
    }
}
