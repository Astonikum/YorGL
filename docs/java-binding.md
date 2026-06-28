# Java Binding

The JVM binding lives in `bindings/java/src/main/java/org/yorgl`.

It is intentionally a renderer binding, not a UI toolkit. Games keep their own UI, input, screens, layout, animation, and state. YorGL only exposes drawing and renderer resource calls.

## Loading

```java
YorGL.load();
```

`YorGL.load()` first tries `System.loadLibrary("yorgl")`. If that fails, it extracts a bundled native library from the jar. Windows x64 currently ships as `org/yorgl/native/windows-x64/yorgl.dll`.

## Renderer

```java
try (YorGL renderer = YorGL.create(BackendKind.Dx11)) {
    RendererCapabilities caps = renderer.getCapabilities();
    renderer.createSwapChain(hwnd, new SwapChainOptions(width, height));
    renderer.setPresentMode(PresentMode.VSync);
    renderer.beginFrame();
    renderer.setViewport(0f, 0f, (float) width, (float) height);
    renderer.clearColor(0f, 0f, 0f, 1f);
    renderer.clearDepth(1f);
    renderer.endFrame();
    RenderDiagnostics diagnostics = renderer.getDiagnostics();
}
```

Kotlin clients can use the same classes directly:

```kotlin
YorGL.create(BackendKind.Dx11).use { renderer ->
    renderer.createSwapChain(hwnd, width, height)
}
```

## Surface

The Java class mirrors the C API:

- swap chain and frame control;
- explicit swap-chain creation options through `SwapChainOptions`;
- backend capability query through `RendererCapabilities`;
- resize, present, and device-removed diagnostics through `RenderDiagnostics`;
- present mode control through `PresentMode.VSync` and `PresentMode.Immediate`;
- texture upload, region update, and release;
- screen-space quads, gradients, scissor, blur, and SDF mode;
- cubemap rendering through `cubemapRender`; `panoramaRender` remains as a compatibility alias;
- world mesh upload, section layer upload, texture binding, texture filter, sky color, render;
- SDF font atlas, metrics, glyphs, kerning, text measurement, and text drawing.

The binding does not introduce game-specific concepts. Game engines translate their own world, UI, and asset data before calling YorGL.
