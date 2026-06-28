# YorGL

YorGL is a rendering abstraction library:

```text
Client code -> stable YorGL API -> switchable graphics backend
```

Current backend modules:

- `null` - portable test backend.
- `dx11` - Windows DirectX 11 backend.

YorGL does not ship a retained UI toolkit. It exposes low-level drawing calls only; game engines own their menus, widgets, input, layout, animation, and styling.

`yorgl3d` is an optional JVM module with low-level scene objects, transforms,
components, scripts, cameras, lights, and mesh baking helpers for clients that
want a small object/component layer above the raw renderer.

Planned backends are added only when they render something real.

## Documentation

- [Architecture](docs/architecture.md)
- [C API](docs/c-api.md)
- [Java Binding](docs/java-binding.md)
- [YorGL3D](docs/yorgl3d.md)
- [DX11 Backend](docs/dx11-backend.md)
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
    implementation("org.yorgl:yorgl3d:0.1.0-SNAPSHOT")
}
```

The JVM artifact bundles `yorgl.dll` for Windows x64 and extracts it on `YorGL.load()`.

## Build Native

```powershell
cmake -S . -B build -DYORGL_BUILD_DX11=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

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
