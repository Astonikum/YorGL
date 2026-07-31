# YorGL Roadmap

This roadmap finishes YorGL as a dependable low-level renderer, with DirectX
11 as the first production-quality backend and a backend-neutral API that can
later host D3D12, Vulkan, and other implementations.

The order is deliberate: correctness and explicit resource ownership come
before advanced effects. A feature is not complete until the public API,
capability reporting, null-backend behavior, DX11 implementation, diagnostics,
documentation, and a runnable regression check agree.

## Baseline: what exists today

- Stable C ABI and thin Java/JNI binding.
- Portable null backend used by native smoke tests.
- DX11 device, flip-model swap chain, resize, VSync/immediate present, tearing
  detection, color/depth targets, texture upload/update, GUI quads, SDF text,
  cubemap, generic world section meshes, fog, and layered blending.
- No formal render graph, typed resource descriptors, shader cache, GPU timing,
  HDR path, temporal history, motion-vector contract, ray-tracing backend, or
  vendor feature integration.

## Phase 1 — API and lifetime correctness

- Implemented slice: the C ABI now reports thread-local result codes and
  rejects invalid dimensions, byte counts, numeric values, present modes,
  world vertex strides, layers, and font inputs before backend dispatch. Native
  smoke coverage and the Java/JNI result mirror are in place; opaque resource
  ownership tracking and complete destruction-order enforcement remain below.
- Replace ambiguous integer arguments with versioned descriptors where the ABI
  needs them; keep old entry points only when they are exact aliases.
- Validate dimensions, byte counts, vertex strides, layer ids, present options,
  null pointers, and renderer ownership at every C boundary.
- Make opaque handles type-safe internally, reject foreign or destroyed handles,
  and define destruction order for textures, fonts, swap chains, and devices.
- Return structured status/error values instead of silently swallowing failed
  resource creation or shader compilation.
- Define the render-thread contract: all context, swap-chain, present, and
  backend resource mutation calls are serialized on the owning thread.
- Add native tests for invalid inputs, repeated shutdown, resize failure,
  device removal diagnostics, and null-backend no-op behavior.

## Phase 2 — DX11 core renderer

- Separate device, swap-chain, resource, pipeline-state, upload, and pass
  ownership so a resize cannot invalidate an unrelated resource silently.
- Add explicit formats, mip levels, array textures, render targets, depth
  resources, staging/readback resources, samplers, vertex/index buffers, and
  constant/storage buffers.
- Add shader compilation from checked-in HLSL or precompiled DXBC with include
  handling, compiler diagnostics, permutation keys, and a disk cache.
- Add immutable pipeline descriptions and state restoration at pass boundaries;
  keep the current immediate context on one render thread.
- Replace the current one-format world vertex path with declared vertex layouts,
  index buffers, instancing, frustum/occlusion-friendly bounds, and explicit
  opaque/translucent ordering.
- Make texture upload pitch, color space, alpha mode, and mip generation
  explicit; support sRGB and HDR formats without changing shader meaning.

## Phase 3 — Frame graph and performance

- Introduce a backend-neutral frame graph with resource lifetimes, pass
  dependencies, transient targets, barriers/state transitions, and named GPU
  markers.
- Add bounded upload rings for constants, vertices, indices, and texture data;
  measure map stalls before selecting discard/no-overwrite strategies.
- Add CPU/GPU timestamps, per-pass timings, memory counters, frame capture
  labels, and an optional debug validation layer.
- Move scene traversal, sorting, culling, mesh preparation, and asset decoding
  off the render thread. Keep immediate-context, DXGI, and `Present` on the
  owning thread; use D3D11 deferred contexts only after capability checks and
  measured benefit.
- Add deterministic stress tests for thousands of sections, texture churn,
  resize storms, minimized windows, and device loss.

## Phase 4 — Image quality and display

- Establish linear-light internal color handling, sRGB conversion, exposure,
  tone mapping, bloom, motion blur, temporal anti-aliasing, and a stable
  depth/motion-vector convention.
- Add HDR10/scRGB capability negotiation, swap-chain color-space selection,
  metadata policy, screenshot/readback conversion, and SDR fallback.
- Make jitter, camera cuts, history invalidation, dynamic resolution, and
  viewport scaling explicit in the frame contract.
- Add spatial fallback upscaling and sharpening first, then temporal/vendor
  paths behind capability and user-selection checks.

## Phase 5 — DLSS, Streamline, and cross-vendor reconstruction

- Integrate NVIDIA Streamline behind an optional runtime-loaded feature module;
  verify signed production binaries and never make the base renderer depend on
  proprietary DLLs.
- Implement and validate, independently, DLSS Super Resolution, DLAA, NVIDIA
  Image Scaling, Reflex markers, DLSS Frame Generation, and supported Multi
  Frame Generation modes. Each feature must report hardware/API support and
  required buffers instead of being exposed as a blind toggle.
- Provide the complete input contract: render-resolution color, output color,
  depth (including linear depth where required), motion vectors, exposure,
  jitter, camera reset, frame index, HUD-less color, and UI composition rules.
- Add an integration matrix for DX11 and future D3D12/Vulkan backends. Do not
  assume that a feature available through Streamline works on every backend or
  GPU generation.
- Add non-NVIDIA fallbacks through an interchangeable reconstruction interface:
  AMD FidelityFX Super Resolution, Intel XeSS where its runtime/API contract
  fits, and YorGL's own spatial/temporal fallback. Keep vendor SDKs optional.
- Add image-quality captures, ghosting/flicker checks, latency measurements,
  resolution-mode tests, and packaging/license checks for every vendor plugin.

## Phase 6 — RTX and ray-traced quality

- Keep DX11's raster backend complete and honest. DX11 is not the home for the
  engine's DXR acceleration-structure and ray-tracing pipeline.
- Extend the backend-neutral API with acceleration structures, ray-tracing
  pipelines, bindless/resource indexing, and ray-query capability bits only
  when a D3D12 or Vulkan backend can implement them.
- Add a D3D12/Vulkan ray-tracing backend for hardware ray tracing, then integrate
  shadows, reflections, ambient occlusion, global illumination, and path-traced
  modes with temporal accumulation and denoising.
- Integrate NVIDIA RTX denoisers/NRD, RTXGI/RTXDI where licensing and API
  requirements allow, and vendor-neutral fallbacks. Ray Reconstruction belongs
  after reliable linear depth, motion vectors, ray outputs, and denoiser
  exclusion are implemented.
- Validate acceleration-structure rebuild/refit, shader binding tables,
  residency, async compute, denoiser history invalidation, and device removal.

## Phase 7 — Backends and shipping quality

- Add a Vulkan backend only after the API contract is exercised by DX11 and the
  null backend; then add D3D12 for ray tracing and modern scheduling.
- Keep backend-specific code behind feature flags and capability queries. A
  backend enters the public tree only when it can create a device, clear, draw a
  real frame, resize, present, report failures, and pass resource lifetime tests.
- Publish stable C headers, CMake package metadata, JVM artifacts, native
  packages, runtime DLL/plugin manifests, symbol packages, and reproducible CI
  matrices for Windows/Linux and supported GPU classes.

## Definition of done

YorGL is "done" for this roadmap when a client can choose a backend, create a
typed frame graph, upload real assets, render SDR or HDR frames, use an
optional supported reconstruction/latency feature, recover from resize/device
loss, inspect timings and diagnostics, and run the same API through the null
backend in CI without backend-specific game code.

## Primary references

- [NVIDIA Streamline](https://github.com/NVIDIA-RTX/Streamline)
- [NVIDIA DLSS](https://developer.nvidia.com/rtx/dlss)
- [NVIDIA Streamline DLSS integration guide](https://developer.nvidia.com/blog/how-to-integrate-nvidia-dlss-4-into-your-game-with-nvidia-streamline/)
- [Direct3D 11 multithreading](https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-render-multi-thread)
- [Direct3D 12 ray tracing](https://learn.microsoft.com/en-us/windows/win32/direct3d12/direct3d-12-raytracing)
- [AMD FidelityFX Super Resolution](https://gpuopen.com/fidelityfx-superresolution/)
