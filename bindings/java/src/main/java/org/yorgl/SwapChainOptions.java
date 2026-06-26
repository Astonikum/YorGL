package org.yorgl;

public final class SwapChainOptions {
    public final int width;
    public final int height;
    public final int bufferCount;
    public final PresentMode presentMode;
    public final boolean allowTearing;

    public SwapChainOptions(int width, int height) {
        this(width, height, 2, PresentMode.VSync, true);
    }

    public SwapChainOptions(int width, int height, int bufferCount, PresentMode presentMode, boolean allowTearing) {
        this.width = width;
        this.height = height;
        this.bufferCount = bufferCount;
        this.presentMode = presentMode;
        this.allowTearing = allowTearing;
    }
}
