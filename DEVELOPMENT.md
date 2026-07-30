# Development

YorGL has three renderer layers:

- client bindings: JVM/Kotlin/Java now, more languages later;
- API: stable C ABI and C++ facade in `src/yorgl`;
- backend modules: `src/backends/*`.

## Boundaries

- C++ is the authoritative implementation language.
- Kotlin/Java/JVM is secondary binding/tooling code and must not own renderer
  state or simulation.
- YorGL never contains YorEngine scenes, gameplay, project files, editor state,
  FrostEngine/Minecraft rules, or ImGui headers.
- YorEngine and YorStudio are independent repositories above this project.
  Their Git dependencies must use a release tag or immutable commit.

## Backend rules

- The `null` backend stays buildable on every supported platform.
- DX11 code stays behind `YORGL_BUILD_DX11` and Windows checks.
- A backend enters the public tree only when it can execute real commands,
  present a frame where applicable, and has focused tests.
- New backend work must preserve the stable API/resource ownership contract.

## API and binding rules

- Every public C/C++ API change gets English documentation and native tests for
  ownership, failure behavior, and compatibility.
- Every JVM API change gets matching docs and keeps Windows x64 native loading
  working.
- Renderer modules must not leak backend-specific types through the portable
  public API.
- Keep commits focused, use `<version>-<task>` branches, and push after each
  meaningful commit so CI remains visible.

## Local checks

```powershell
./gradlew build
cmake -S . -B build-null -DYORGL_BUILD_DX11=OFF -DYORGL_BUILD_TESTS=ON
cmake --build build-null --config Debug
ctest --test-dir build-null -C Debug --output-on-failure
```

## CI/CD

`.github/workflows/ci.yml` builds native smoke tests on Linux and Windows and
the JVM artifact on Linux and Windows. `.github/workflows/release.yml` runs on
`v*` tags and attaches tested native/JVM packages to the GitHub Release.
