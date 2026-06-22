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
