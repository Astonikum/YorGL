package org.yorgl;

public enum TextureFilter {
    Nearest(0),
    Linear(1);

    final int id;

    TextureFilter(int id) {
        this.id = id;
    }
}
