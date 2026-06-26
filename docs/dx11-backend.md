# DX11 Backend

The DirectX 11 backend lives in `src/backends/dx11`.

## Responsibilities

- create the D3D11 device;
- create and resize the swap chain;
- select `VSync` or `Immediate` present mode, using DXGI tearing only when supported;
- own render target and depth target resources;
- host DX11 implementations of GUI, panorama, world mesh, texture, and SDF font modules;
- present frames through the swap chain.

## Modules

- `modules/gui_renderer.*` draws screen-space colored, textured, gradient, SDF, scissored, and blurred rectangles.
- `modules/world_renderer.*` draws generic 3D mesh sections with opaque and translucent layers. Uploaded section layers are stored in default-usage D3D11 vertex buffers because chunk data is static between explicit uploads.
- `modules/cubemap_renderer.*` draws six texture handles as a panorama.
- `modules/sdf_font.*` bakes TTF bytes into an SDF atlas and exposes glyph metrics.
- `modules/yorgl_log.hpp` provides minimal backend logging.

## Boundaries

The DX11 backend does not contain Minecraft, FrostEngine, or game object types. It accepts only generic handles, numbers, and vertex arrays from `src/yorgl/api.h`.

## Build Flag

DX11 is compiled when `YORGL_BUILD_DX11=ON` and the target platform is Windows. Non-Windows CI builds use the `null` backend.
