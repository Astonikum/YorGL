# YorEngine Roadmap

YorEngine is the generic 3D engine layer above YorGL. Its goal is to make a
complete non-Minecraft game possible without turning YorGL into a game engine
or moving Frost UI into either renderer project.

The engine must remain usable as a library: every system has explicit lifetime,
thread, ownership, serialization, and deterministic-update rules. A system is
complete only when a small sample game can use it without reaching into private
implementation classes.

## Baseline: what exists today

- Java module `org.yorgl:yorengine` with `Scene`, `SceneObject`, parent/child
  transforms, `Component`, `Script`, `Camera`, `Light`, `Material`, and
  `MeshComponent`.
- Snapshot-safe component updates and a scene version for skipping unchanged
  mesh uploads.
- Generic vertex baking into the current YorGL world vertex format.
- No engine loop, stable entity ids, asset database, material/shader system,
  render graph, animation, physics, audio, input abstraction, UI, editor,
  save format, networking, tooling, or complete sample game.

## Phase 0 — Stable engine contracts

- Define `Engine`, `World`, `EntityId`, `Scene`, `ComponentStore`, `System`,
  `Transform`, `Camera`, `Light`, `Mesh`, `Material`, and `AssetHandle`
  ownership without exposing mutable implementation collections.
- Add stable numeric ids, generation checks, explicit active/destroyed states,
  parent-cycle validation, and deterministic iteration order.
- Separate simulation state, render snapshots, and editor/debug metadata.
- Define units, coordinate handedness, radians/degrees policy, depth range,
  color space, threading model, and serialization versioning.
- Replace string-only custom properties with typed extension data while keeping
  a small metadata escape hatch for integrations.

## Phase 1 — Runtime and scene model

- Add an engine lifecycle with startup, fixed simulation ticks, variable render
  ticks, pause, single-step, shutdown, and failure propagation.
- Make component attach/detach/dispose behavior explicit; support safe command
  buffers for structural changes during iteration.
- Add scene loading/instantiation, nested scenes/prefabs, tags/layers, queries,
  visibility flags, spawn/despawn events, and stable references.
- Add transform dirty propagation, cached world matrices, bounds, parent
  reparenting, and a render snapshot that cannot mutate during draw submission.
- Add a generic input/event adapter and time source. Minecraft/Frost input and
  gameplay remain on the FrostEngine side; YorEngine consumes generic events.

## Phase 2 — Assets, materials, and rendering integration

- Define asynchronous asset handles, loaders, cancellation, dependency graphs,
  hot reload, cache eviction, and shutdown-safe upload queues.
- Add mesh/index/instance formats, tangent generation, skin weights, bounds,
  LODs, meshlets only after profiling, and a material graph that compiles to
  YorGL pipeline/resource descriptions.
- Add texture color-space/alpha metadata, sampler policy, shader includes,
  reflection validation, permutation limits, and material defaults.
- Build a renderer integration that submits cameras, opaque/masked/translucent
  queues, shadows, sky, particles, post effects, and debug geometry to YorGL;
  never make the engine issue backend-specific DirectX calls.
- Add render-layer sorting, frustum/occlusion culling, instancing, batching,
  visibility history, and frame capture labels.

## Phase 3 — Cameras, lighting, and world quality

- Add perspective/orthographic cameras, exposure, jitter, camera cuts, stereo
  views, and explicit depth/motion-vector outputs.
- Add directional, point, spot, area, emissive, and environment lights with
  shadow policy, light culling, probes, reflection captures, and clustered or
  tiled selection.
- Add physically based materials, normal/roughness/metallic/occlusion maps,
  image-based lighting, sky atmosphere, fog, decals, transparency, and a
  documented fallback path for low-end hardware.
- Add terrain/streaming worlds, sectors, occlusion data, instanced foliage,
  and bounded upload budgets without making the core scene Minecraft-specific.

## Phase 4 — Animation and gameplay-ready simulation

- Add skeletal animation clips, blending graphs, state machines, root motion,
  inverse kinematics, morph targets, animation events, and deterministic pose
  evaluation.
- Add generic physics integration boundaries for broad phase, rigid bodies,
  colliders, character controllers, joints, queries, and fixed-step ownership.
- Add particles/VFX with GPU/CPU simulation, deterministic seeds, pooling,
  bounds, collision policy, and render-queue integration.
- Add audio devices, buses, spatial emitters, streaming music, effects, and
  pause/visibility behavior behind an optional dependency.

## Phase 5 — Game services and content pipeline

- Add save/load with schema versions, migrations, atomic writes, checksums, and
  safe recovery from partial files.
- Add generic input maps, rebinding, localization, controller/gamepad support,
  accessibility hooks, and platform window services. FrostEngine keeps its
  Minecraft and Frost UI policies outside this module.
- Add a content build pipeline for source assets, imported metadata, dependency
  manifests, compression, shader compilation, thumbnails, and reproducible
  cache keys.
- Add optional networking primitives only for generic game sessions; FrostEngine
  remains the owner of Minecraft networking and gameplay truth.
- Add scripting/plugin boundaries with capability-limited services, reload
  rules, error isolation, and explicit deterministic vs nondeterministic APIs.

## Phase 6 — Tools and editor

- Add an inspectable world format, editor-only metadata, undo/redo commands,
  selection, gizmos, hierarchy view, material/asset inspection, and play-mode
  separation.
- Add render/debug views for bounds, overdraw, depth, normals, light clusters,
  shadow maps, motion vectors, TAA history, and frame timings.
- Add automated import validation, missing-asset reports, shader/material
  diagnostics, scene linting, and CI asset checks.
- Keep editor/UI implementation replaceable and separate from Frost UI; the
  engine exposes data and render hooks, not a retained Frost menu toolkit.

## Phase 7 — Shipping one complete game

- Build one small but complete reference game using only public YorEngine APIs:
  title/settings screens, loading, save/load, input rebinding, one streamed
  world, camera and lighting, animated actors, physics, particles, audio,
  localization, pause, error recovery, and a release package.
- Add replayable deterministic smoke scenarios for load, movement, combat or
  interaction, scene transition, save recovery, and device loss.
- Test low-end fallback, controller-only navigation, high-DPI/window resize,
  alt-tab, suspend/resume, missing asset, shader compile failure, and corrupted
  save behavior.
- Define versioned public APIs and migration policy before third-party games
  depend on the module.

## Definition of done

YorEngine is "done" for this roadmap when an independent game can implement its
simulation, content, scenes, rendering, animation, physics, audio, saves, input,
tooling, and shipping loop through public engine contracts, while YorGL remains
replaceable and FrostEngine can continue to own Minecraft extraction, gameplay,
networking, and Frost UI.
