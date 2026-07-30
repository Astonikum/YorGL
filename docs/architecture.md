# Architecture

YorGL exposes a stable C ABI and a small C++ backend interface for renderer
modules.

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

## Product boundary

YorGL owns renderer lifetime, native graphics resources, backend-specific
command execution, capability reporting, and the binding boundary. Client
projects own game data, asset conversion, world/scene systems, UI state, and
platform window handles.

YorEngine is a separate C++ project and may consume YorGL through this public
API. YorGL must never depend on YorEngine. YorStudio is a separate C++ project
above YorEngine and is not a runtime dependency of shipped games.

The intended data direction is:

```text
YorStudio editor -> YorEngine public runtime/render data -> YorGL -> backend
```

The repository contains no engine/editor code or UI framework. ImGui belongs
only to the YorStudio adapter layer.

## Layers

- **Bindings** call the C ABI from Java, Kotlin, and future languages.
- **C API** keeps ABI names, handles, errors, and lifecycle stable.
- **C++ renderer facade** owns backend selection and renderer lifetime.
- **Backend modules** implement the common behavior for each graphics API.

## Git integration

Higher-level repositories consume YorGL with the canonical URL
`https://github.com/Astonikum/YorGL.git`, pinned to a release tag or immutable
commit. YorGL itself has no reverse dependency and builds from a clean checkout.
