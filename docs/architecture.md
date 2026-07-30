# Architecture

`YorGL` exposes a stable C ABI for language bindings and a small C++ backend interface for renderer modules.

```text
JVM / future bindings
        |
        v
YorGL C ABI
        |
        v
Renderer instance
        |
        v
Null / DX11 / future backend
```

## Project Boundary

The repository contains two implemented native products and one planned
desktop product:

- **YorGL** is the low-level renderer project. It owns the C ABI, C++ backend
  interface, native resource lifetime, command execution, and the thin JVM
  binding under `bindings/java`.
- **YorEngine** is the separate C++ engine project under `yorengine`. It owns
  generic scene graphs, objects, components, transforms, cameras, lights,
  materials, and engine runtime systems. JVM/Kotlin code is only a secondary
  binding/adapter layer.
- **YorStudio** is the separate C++ desktop launcher/editor product planned
  under `yorstudio`. It owns project files, editor commands, content tooling,
  selection, undo/redo, diagnostics, and UI adapters.

The native build keeps the projects as separate targets: `yorgl` is the
renderer library, while `yorengine` is the C++ engine library and
`yorengine_api` is its shared C binding boundary. YorEngine's public headers
are under `yorengine/include/yorengine`; the engine may call YorGL through a
future public renderer facade, but the current scene core has no
backend-specific dependency.

YorEngine may depend on YorGL, but YorGL must never depend on YorEngine. The
engine does not move Minecraft extraction, gameplay rules, networking, or
Frost UI into the renderer library. The current simulation flow is:

```text
YorEngine Scene/Runtime -> immutable RenderSnapshot -> YorGL C API -> backend
```

The current JVM scene slice is transitional and must migrate behind the C++
YorEngine API rather than grow new engine logic.

YorStudio must depend on YorEngine public contracts and RenderSnapshot values.
It may submit editor commands to mutate authoring state, but it must not call
private engine storage or make ImGui types part of YorEngine/YorGL headers.

## Layers

- **Client bindings** call the C API from Java, Kotlin, and future languages.
- **C API** keeps a stable ABI in `src/yorgl/api.h`.
- **C++ renderer facade** owns backend lifetime.
- **Backend modules** implement the same renderer behavior for each graphics API.

## Ownership

YorGL owns renderer lifetime, native graphics resources, and backend-specific command execution.

Client projects own game data, asset conversion, world meshing, UI state, and platform window handles.

## Backend Rule

A backend enters the public tree only when it can execute real rendering commands. Placeholder backend names may appear in docs or enums only when they do not claim feature support.
