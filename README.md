# YorGL

YorGL is a rendering abstraction library:

```text
Client bindings -> stable YorGL API -> switchable graphics backend
```

Current backend modules:

- `null` - portable test backend.
- `dx11` - Windows DirectX 11 backend.

Kotlin bindings also include `org.yorgl.ui`, a tiny retained UI layer that renders into a client-provided canvas.

Planned backends are added only when they render something real.

## Documentation

- [Architecture](docs/architecture.md)
- [C API](docs/c-api.md)
- [Kotlin Binding](docs/kotlin-binding.md)
- [DX11 Backend](docs/dx11-backend.md)
- [UI](docs/ui.md)

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
}
```

The Kotlin artifact bundles `yorgl.dll` for Windows x64 and extracts it on `YorGL.load()`.

## Build Native

```powershell
cmake -S . -B build -DYORGL_BUILD_DX11=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

## Build Kotlin Artifact

```powershell
./gradlew build
```

## Releases

CI runs on every push and pull request. Tags that start with `v` create a
GitHub Release with:

- Kotlin jar built on Windows with the bundled `yorgl.dll`.
- Native CMake install package for Windows x64 with DX11 enabled.
- Portable native CMake install package with the null backend on Linux.
