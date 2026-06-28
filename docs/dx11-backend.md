# DX11 Backend

The DirectX 11 backend lives in `src/backends/dx11`.

## Responsibilities

- create the D3D11 device;
- report D3D feature level, maximum texture size, and present-mode capabilities;
- create and resize the swap chain, preserving explicit buffer count, initial present mode, and tearing policy across resize;
- select `VSync` or `Immediate` present mode, using DXGI tearing only when supported;
- store the last resize/present HRESULT and device-removed reason for client diagnostics;
- own render target and depth target resources;
- host DX11 implementations of GUI, cubemap, world mesh, texture upload/update, and SDF font modules;
- present frames through the swap chain.

## Modules

- `modules/gui_renderer.*` draws screen-space colored, textured, gradient, SDF, scissored, and blurred rectangles.
- `modules/world_renderer.*` draws generic 3D mesh sections with opaque, translucent, and translucent overlay layers. Uploaded section layers are stored in default-usage D3D11 vertex buffers because chunk data is static between explicit uploads.
- The world renderer supports nearest and linear atlas sampling; nearest remains the default for pixel-art atlases.
- `modules/cubemap_renderer.*` draws six texture handles as an inside-out cube.
- `modules/sdf_font.*` bakes TTF bytes into an SDF atlas and exposes glyph metrics.
- `modules/yorgl_log.hpp` provides minimal backend logging.

## Boundaries

The DX11 backend does not contain Minecraft, FrostEngine, or game object types. It accepts only generic handles, numbers, and vertex arrays from `src/yorgl/api.h`.

## Build Flag

DX11 is compiled when `YORGL_BUILD_DX11=ON` and the target platform is Windows. Non-Windows CI builds use the `null` backend.
