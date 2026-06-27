# Development

YorGL has three layers:

- Client bindings: Kotlin now, more languages later.
- API: C ABI and C++ facade in `src/yorgl`.
- Backend modules: `src/backends/*`.

Rules:

- `null` backend must stay buildable on every platform.
- DX11 code stays behind `YORGL_BUILD_DX11` and Windows checks.
- Do not add a new backend until it can clear and present a frame.
- Do not put Minecraft/FrostEngine concepts in YorGL.
- Every public C API or Kotlin API addition gets a matching English doc page update.
- Kotlin artifact builds must keep bundled native loading working on Windows x64.

## Local Checks

```powershell
./gradlew build
cmake -S . -B build-null -DYORGL_BUILD_DX11=OFF -DYORGL_BUILD_TESTS=ON
cmake --build build-null --config Debug
ctest --test-dir build-null -C Debug --output-on-failure
```

## CI/CD

- `.github/workflows/ci.yml` builds native smoke tests on Linux and Windows,
  builds the JVM artifact on Linux, and builds the Windows JVM artifact with
  bundled `yorgl.dll`.
- `.github/workflows/release.yml` runs on `v*` tags and attaches Kotlin/native
  packages to the GitHub Release.
