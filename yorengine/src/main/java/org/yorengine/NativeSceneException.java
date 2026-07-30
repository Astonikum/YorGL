package org.yorengine;

public final class NativeSceneException extends RuntimeException {
    private final int status;

    public NativeSceneException(int status, String message) {
        super(message);
        this.status = status;
    }

    public int status() {
        return status;
    }
}
