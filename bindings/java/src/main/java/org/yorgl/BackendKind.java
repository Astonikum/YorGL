package org.yorgl;

public enum BackendKind {
    Null(0),
    Dx11(1);

    final int id;

    BackendKind(int id) {
        this.id = id;
    }
}
