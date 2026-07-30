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

The repository contains two separate build products:

- **YorGL** is the low-level renderer project. It owns the C ABI, C++ backend
  interface, native resource lifetime, command execution, and the thin JVM
  binding under `bindings/java`.
- **YorEngine** is the optional `yorengine` JVM module above YorGL. It owns
  generic scene graphs, objects, components, transforms, cameras, lights,
  materials, and future engine runtime systems.

YorEngine may depend on YorGL, but YorGL must never depend on YorEngine. The
engine does not move Minecraft extraction, gameplay rules, networking, or
Frost UI into the renderer library.

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
