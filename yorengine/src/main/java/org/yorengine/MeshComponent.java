package org.yorengine;

import java.util.Arrays;

public final class MeshComponent implements Component {
    public static final int STRIDE_FLOATS = 9;
    private final float[] localVertices;
    private Material material;

    public MeshComponent(float[] localVertices) {
        if (localVertices.length % STRIDE_FLOATS != 0) {
            throw new IllegalArgumentException("Mesh vertices must use x,y,z,r,g,b,a,u,v stride");
        }
        this.localVertices = Arrays.copyOf(localVertices, localVertices.length);
    }

    public float[] localVertices() {
        return Arrays.copyOf(localVertices, localVertices.length);
    }

    public MeshComponent material(Material material) {
        this.material = material;
        return this;
    }

    public Material material() {
        return material;
    }

    void appendWorldVertices(SceneObject object, FloatList out) {
        float[] m = object.worldMatrix();
        for (int i = 0; i < localVertices.length; i += STRIDE_FLOATS) {
            float x = localVertices[i];
            float y = localVertices[i + 1];
            float z = localVertices[i + 2];
            out.add(x * m[0] + y * m[4] + z * m[8] + m[12]);
            out.add(x * m[1] + y * m[5] + z * m[9] + m[13]);
            out.add(x * m[2] + y * m[6] + z * m[10] + m[14]);
            out.add(localVertices, i + 3, STRIDE_FLOATS - 3);
        }
    }
}
