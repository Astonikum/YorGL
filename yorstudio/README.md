# YorStudio

YorStudio is the planned C++ desktop launcher/editor for the YOR ecosystem.
Its ownership, project format, module structure, and implementation gates are
documented before the executable target is added.

- Product overview: [`../docs/yorstudio.md`](../docs/yorstudio.md)
- Implementation roadmap: [`../docs/yorstudio-roadmap.md`](../docs/yorstudio-roadmap.md)
- Ecosystem boundaries: [`../docs/yor-ecosystem.md`](../docs/yor-ecosystem.md)

The first executable must arrive with a tested project manifest/launcher,
platform window contract, and replaceable UI port. Dear ImGui will be the first
adapter; it must not leak into YorGL, YorEngine, or editor/project models.
