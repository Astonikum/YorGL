package org.yorgl;

public final class RenderDiagnostics {
    public final int lastResizeResult;
    public final int lastPresentResult;
    public final int deviceRemovedReason;

    RenderDiagnostics(int[] values) {
        this.lastResizeResult = values[0];
        this.lastPresentResult = values[1];
        this.deviceRemovedReason = values[2];
    }
}
