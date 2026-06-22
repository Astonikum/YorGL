# Kotlin Binding

The Kotlin binding lives in `bindings/kotlin/src/main/kotlin/org/yorgl`.

## Loading

```kotlin
YorGL.load()
```

`YorGL.load()` first tries `System.loadLibrary("yorgl")`. If that fails, it extracts a bundled native library from the jar. Windows x64 currently ships as `org/yorgl/native/windows-x64/yorgl.dll`.

## Renderer

```kotlin
YorGL.create(BackendKind.Dx11).use { renderer ->
    renderer.createSwapChain(hwnd, width, height)
    renderer.beginFrame()
    renderer.setViewport(0f, 0f, width.toFloat(), height.toFloat())
    renderer.clearColor(0f, 0f, 0f, 1f)
    renderer.clearDepth(1f)
    renderer.endFrame()
}
```

`YorGL` implements `AutoCloseable`; use `use` or call `close()`.

## Surface

The Kotlin class mirrors the C API:

- swap chain and frame control;
- texture upload and release;
- GUI quads, gradients, scissor, blur, SDF mode;
- panorama rendering;
- world mesh upload, section layer upload, texture binding, sky color, render;
- SDF font atlas, metrics, glyphs, kerning.

Kotlin does not introduce game-specific concepts. Game engines should translate their own world, UI, and asset data before calling YorGL.
