package org.yorgl;

public final class RendererCapabilities {
    public final BackendKind backend;
    public final int featureLevelMajor;
    public final int featureLevelMinor;
    public final int maxTextureSize;
    public final boolean presentVSync;
    public final boolean presentImmediate;
    public final boolean presentTearing;

    RendererCapabilities(int[] values) {
        this.backend = values[0] == BackendKind.Dx11.id ? BackendKind.Dx11 : BackendKind.Null;
        this.featureLevelMajor = values[1];
        this.featureLevelMinor = values[2];
        this.maxTextureSize = values[3];
        this.presentVSync = values[4] != 0;
        this.presentImmediate = values[5] != 0;
        this.presentTearing = values[6] != 0;
    }
}
