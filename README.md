# YorGL

YorGL is a small rendering abstraction library:

```text
Client bindings -> stable YorGL API -> switchable graphics backend
```

Current backends:

- `null` - portable test backend.
- `dx11` - Windows DirectX 11 backend.

Planned backends are added only when they render something real.

## Use From Gradle/JitPack

After `Astonikum/YorGL` exists on GitHub:

```kotlin
repositories {
    maven("https://jitpack.io")
}

dependencies {
    implementation("com.github.Astonikum:YorGL:main-SNAPSHOT")
}
```

Native binaries still have to be on `java.library.path` or next to the app executable.

## Build Native

```powershell
cmake -S . -B build -DYORGL_BUILD_DX11=ON
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

## Build Kotlin Binding

```powershell
./gradlew build
```
