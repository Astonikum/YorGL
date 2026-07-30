# Development

YorGL has three renderer layers:

- Client bindings: JVM/Kotlin/Java now, more languages later.
- API: C ABI and C++ facade in `src/yorgl`.
- Backend modules: `src/backends/*`.

YorEngine is a separate C++ target under `yorengine`. C++ is the primary
implementation language for both projects. JVM/Kotlin/Java code is limited to
bindings and transitional adapters; it must not duplicate engine simulation or
renderer ownership.

Rules:

- `null` backend must stay buildable on every platform.
- DX11 code stays behind `YORGL_BUILD_DX11` and Windows checks.
- Do not add a new backend until it can clear and present a frame.
- Do not put Minecraft/FrostEngine concepts in YorGL.
- Every public C API or Kotlin API addition gets a matching English doc page update.
- Kotlin artifact builds must keep bundled native loading working on Windows x64.
- Every public YorEngine C++ API addition gets matching English documentation
  and a native test that exercises its ownership and failure behavior.

## Local Checks

```powershell
./gradlew build
cmake -S . -B build-null -DYORGL_BUILD_DX11=OFF -DYORGL_BUILD_TESTS=ON
cmake --build build-null --config Debug
ctest --test-dir build-null -C Debug --output-on-failure
```

The command above builds both native targets by default. To test YorEngine in
an isolated configuration, use `-DYORGL_BUILD_YORENGINE=ON` and run the
`yorengine_smoke` test reported by CTest.

## CI/CD

- `.github/workflows/ci.yml` builds native smoke tests on Linux and Windows,
  builds the JVM artifact on Linux, and builds the Windows JVM artifact with
  bundled `yorgl.dll`.
- `.github/workflows/release.yml` runs on `v*` tags and attaches Kotlin/native
  packages to the GitHub Release.
