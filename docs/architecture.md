# Architecture

`YorGL` exposes a stable C ABI for language bindings and a small C++ backend interface for renderer modules.

```text
Kotlin / future bindings
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

The API owns renderer lifetime. Backends own graphics-device resources. Client projects own game data, asset conversion, world meshing, UI state, and platform window handles.
