# YorGL

YorGL is a rendering abstraction library:

```text
Client code -> stable YorGL API -> switchable graphics backend
```

Current backend modules:

- `null` - portable test backend.
- `dx11` - Windows DirectX 11 backend.

YorGL does not ship a retained UI toolkit. It exposes low-level drawing calls only; game engines own their menus, widgets, input, layout, animation, and styling.

This repository is becoming the YOR ecosystem with three hard product
boundaries:

- `YorGL` is the low-level C++ renderer/API project. It owns the stable C ABI,
  native resource handles, backend implementations, and the JVM renderer
  binding.
- `YorEngine` is the separate C++ engine project under `yorengine`. Its core
  owns generic 3D scene objects, components, transforms, cameras, lights, and
  the engine runtime. JVM/Kotlin code is only a secondary binding/adapter and
  must not become the home of engine logic.
- `YorStudio` is the planned C++ desktop launcher/editor under `yorstudio`.
  It will own project lifecycle, editor commands, content tooling, and UI
  adapters; it will not become a runtime dependency or duplicate YorEngine
  state.

The native build produces the `yorgl` renderer library, the `yorengine` C++
static library, and the `yorengine_api` shared C binding. YorEngine currently
exposes the foundational math and scene contracts; higher-level runtime systems
are added only after
their ownership, lifetime, threading, and test contracts are documented.

The checked-in JVM scene classes are an interim migration slice from the old
`YorGL3D` module. They are not the target engine architecture; the C++ core and
its binding boundary are tracked in the [YorEngine roadmap](docs/yorengine-roadmap.md).

Planned backends are added only when they render something real.

## Documentation

- [Architecture](docs/architecture.md)
- [C API](docs/c-api.md)
- [Java Binding](docs/java-binding.md)
- [YorEngine](docs/yorengine.md)
- [DX11 Backend](docs/dx11-backend.md)
- [YorGL roadmap](docs/yorgl-roadmap.md)
- [YorEngine roadmap](docs/yorengine-roadmap.md)
- [YOR ecosystem](docs/yor-ecosystem.md)
- [YorStudio](docs/yorstudio.md)
- [YorStudio roadmap](docs/yorstudio-roadmap.md)
- [Security Policy](SECURITY.md)

## Use From Gradle Git Source Dependency

```kotlin
// settings.gradle.kts
sourceControl {
    gitRepository(uri("https://github.com/Astonikum/YorGL.git")) {
        producesModule("org.yorgl:yorgl")
    }
}

// build.gradle.kts
dependencies {
    implementation("org.yorgl:yorgl:0.1.0-SNAPSHOT")
    implementation("org.yorgl:yorengine:0.1.0-SNAPSHOT")
}
```

The JVM artifact bundles `yorgl.dll` for Windows x64 and extracts it on `YorGL.load()`.

## Build Native

```powershell
cmake -S . -B build -DYORGL_BUILD_DX11=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Use `-DYORGL_BUILD_YORENGINE=OFF` only when a renderer-only build is required.
The default build includes YorEngine and its native smoke test.

YorStudio is currently at the contract/roadmap stage. Its first executable
will be added only with a tested project manifest, launcher lifecycle, platform
window, and replaceable UI boundary; there is intentionally no placeholder
ImGui executable in the native build yet.

## Build JVM Artifact

```powershell
./gradlew build
```

## Releases

CI runs on every push and pull request. Tags that start with `v` create a
GitHub Release with:

- JVM jar built on Windows with the bundled `yorgl.dll`.
- Native CMake install package for Windows x64 with DX11 enabled.
- Portable native CMake install package with the null backend on Linux.
