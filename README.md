# YorGL

YorGL is the low-level, C++ rendering/API project in the YOR ecosystem:

```text
Client or YorEngine -> stable YorGL API -> switchable graphics backend
```

It owns renderer resources, backend lifetime, command execution, the stable C
ABI, the C++ backend boundary, and the secondary JVM binding. It does not own
scenes, entities, components, gameplay rules, editor state, project manifests,
or UI policy.

## Backends

- `null` — portable deterministic test backend.
- `dx11` — Windows DirectX 11 backend.

DX11 is the current production backend target. Vulkan and other APIs enter the
tree only through the same public backend contract and only with real rendering
behavior plus tests; naming a future backend does not claim support.

YorGL does not ship a retained UI toolkit. Games and editors own their menus,
widgets, input, layout, animation, and styling.

## YOR repositories

- [YorGL](https://github.com/Astonikum/YorGL) — this low-level renderer/API.
- [YorEngine](https://github.com/Astonikum/YorEngine) — C++ runtime, scenes,
  entities, components, assets, and rendering integration.
- [YorStudio](https://github.com/Astonikum/YorStudio) — C++ project launcher
  and editor.

The dependency direction is one-way: `YorStudio -> YorEngine -> YorGL`.
YorGL never depends on YorEngine. When a higher-level product consumes YorGL,
it uses the canonical Git repository pinned to a release tag or immutable
commit; released builds never follow a moving `main` branch.

## Documentation

- [Architecture](docs/architecture.md)
- [C API](docs/c-api.md)
- [Java binding](docs/java-binding.md)
- [DX11 backend](docs/dx11-backend.md)
- [YorGL roadmap](docs/yorgl-roadmap.md)
- [Security policy](SECURITY.md)

## Build native library

```powershell
cmake -S . -B build -DYORGL_BUILD_DX11=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

The `null` backend keeps Linux and other non-Windows builds portable. DX11 is
enabled only on Windows. Add `-DYORGL_BUILD_JNI=OFF` when JNI headers are not
available.

## Build JVM artifact

```powershell
./gradlew build
```

The JVM artifact is a secondary binding. It bundles `yorgl.dll` for Windows
x64 and extracts it when `YorGL.load()` is called; native C++ remains the
source of truth.

## Releases and CI

Every push and pull request runs native smoke tests on Linux and Windows plus
the JVM binding build. Tags beginning with `v` publish a GitHub Release with
the tested native install packages and Windows JVM artifact.
