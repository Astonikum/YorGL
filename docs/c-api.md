# C API

The C API in `src/yorgl/api.h` is the stable boundary used by language bindings.

## Lifetime

- `yorglCreate(backend)` creates a renderer using `YORGL_BACKEND_NULL` or `YORGL_BACKEND_DX11`.
- `yorglDestroy(renderer)` releases the renderer and all backend-owned graphics resources.
- `yorglIsValid(renderer)` reports whether backend initialization succeeded.
- `yorglBackendName(renderer)` returns a static backend name.
- `yorglGetCapabilities(renderer, outCapabilities)` reports the backend kind, graphics feature level, maximum texture size, and supported present modes.
- `yorglGetDiagnostics(renderer, outDiagnostics)` reports the last resize HRESULT, last present HRESULT, and backend device-removed reason when available.

## Frame And Swap Chain

- `yorglCreateSwapChain(renderer, windowHandle, width, height)` attaches rendering to a native window.
- `yorglCreateSwapChainWithOptions(renderer, windowHandle, options)` attaches rendering with explicit width, height, buffer count, initial present mode, and tearing policy.
- `yorglResize(renderer, width, height)` resizes backend render targets while keeping the swap-chain buffer count and tearing policy chosen at creation time.
- `yorglBeginFrame(renderer)` binds frame targets.
- `yorglSetViewport(renderer, x, y, width, height)` sets the active viewport.
- `yorglClearColor(renderer, r, g, b, a)` clears the color target.
- `yorglClearDepth(renderer, depth)` clears the depth target.
- `yorglSetPresentMode(renderer, YORGL_PRESENT_VSYNC)` presents with vertical sync.
- `yorglSetPresentMode(renderer, YORGL_PRESENT_IMMEDIATE)` presents without waiting for vblank; the DX11 backend enables DXGI tearing when the OS and driver report support.
- `yorglEndFrame(renderer)` presents the frame when the backend owns a swap chain.

## Textures

- `yorglCreateTexture(renderer, width, height, rgba, byteCount)` uploads RGBA8 pixels and returns an opaque texture handle.
- `yorglUpdateTextureRegion(renderer, texture, x, y, width, height, rgba, byteCount)` replaces one RGBA8 rectangle in a texture created by the same renderer.
- `yorglDestroyTexture(renderer, texture)` releases a texture handle created by the same renderer.

## Screen-Space Draw Calls

These calls render screen-space quads through the active backend. They are renderer primitives, not a retained UI toolkit:

- `yorglGuiBegin`
- `yorglGuiDrawQuad`
- `yorglGuiDrawGradientQuad`
- `yorglGuiSetTexture`
- `yorglGuiSetScissor`
- `yorglGuiClearScissor`
- `yorglGuiSetSdfMode`
- `yorglGuiSetSdfParams`
- `yorglGuiBlurRect`
- `yorglGuiEnd`

## Cubemap Rendering

`yorglCubemapRender(renderer, faces6, yawRadians, width, height)` draws six
texture handles as an inside-out cube. Face order is front, right, back, left,
top, bottom. `yorglPanoramaRender` remains as a compatibility alias for older
clients.

## World Rendering

World rendering is generic mesh rendering. YorGL does not know about Minecraft chunks; callers provide mesh identifiers, positions, vertices, camera direction, FOV, far plane, and texture handles.

- `yorglWorldUploadMesh`
- `yorglWorldUploadSection`
- `yorglWorldUploadSectionLayer`
- `yorglWorldUploadSectionLayerTextured`
- `yorglWorldRemoveSection`
- `yorglWorldClearSections`
- `yorglWorldSetTexture`
- `yorglWorldSetTextureFilter`
- `yorglWorldSetSkyColor`
- `yorglWorldRender`

`yorglWorldUploadSectionLayerTextured` uploads one section layer with a
per-layer texture override. Use it for dynamic world layers such as text or
markers that need a different atlas from the global world texture.

## Fonts

The SDF font API creates a backend-owned atlas from TTF bytes:

- `yorglSdfFontCreate`
- `yorglSdfFontDestroy`
- `yorglSdfFontAtlas`
- `yorglSdfFontMetrics`
- `yorglSdfFontGlyph`
- `yorglSdfFontKerning`
- `yorglSdfFontTextWidth`
- `yorglSdfFontLineHeight`
- `yorglSdfFontDrawText`
