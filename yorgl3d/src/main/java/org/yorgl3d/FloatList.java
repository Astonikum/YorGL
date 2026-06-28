package org.yorgl3d;

import java.util.Arrays;

final class FloatList {
    private float[] values = new float[256];
    private int size;

    void add(float value) {
        ensure(size + 1);
        values[size++] = value;
    }

    void add(float[] source, int offset, int count) {
        ensure(size + count);
        System.arraycopy(source, offset, values, size, count);
        size += count;
    }

    float[] toArray() {
        return Arrays.copyOf(values, size);
    }

    private void ensure(int wanted) {
        if (wanted <= values.length) return;
        values = Arrays.copyOf(values, Math.max(wanted, values.length * 2));
    }
}
