package org.yorgl;

public enum PresentMode {
    VSync(0),
    Immediate(1);

    final int id;

    PresentMode(int id) {
        this.id = id;
    }
}
