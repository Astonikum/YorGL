package org.yorgl;

public enum YorGLResult {
    Ok(0),
    InvalidArgument(1),
    InvalidHandle(2),
    NotReady(3),
    BackendFailure(4);

    final int id;

    YorGLResult(int id) {
        this.id = id;
    }

    static YorGLResult fromId(int id) {
        for (YorGLResult result : values()) {
            if (result.id == id) return result;
        }
        throw new IllegalArgumentException("Unknown YorGL result: " + id);
    }
}
