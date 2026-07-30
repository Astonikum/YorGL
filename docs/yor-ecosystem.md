# YOR Ecosystem

YOR is the open-source C++ game technology ecosystem built from three
separate products. They live in one repository for now, but each product has
its own ownership, build target, public API, tests, and release boundary.

## Products

| Product | Role | Owns | Must not own |
| --- | --- | --- | --- |
| YorGL | Low-level renderer/API | C ABI, renderer resources, backend abstraction, DX11/Vulkan/DX12 implementations | scenes, gameplay, editor state, project files, UI policy |
| YorEngine | Generic game engine | worlds, entities, components, runtime, assets, rendering integration, physics, animation, audio, save services | backend-specific calls, Minecraft/Frost rules, editor-only metadata |
| YorStudio | Desktop launcher and editor | project discovery, project lifecycle, scene editing, assets, inspectors, tools, editor metadata | runtime simulation ownership, game-specific rules, direct backend calls |

The dependency direction is one-way:

```text
Game project code and content
              |
              v
        YorStudio editor
              |
              v
        YorEngine C++
              |
              v
          YorGL C++
              |
              v
       DX11 / Vulkan / DX12 / null
```

YorStudio reads through YorEngine public queries and render snapshots and writes
through explicit editor commands. It must not reach into private Scene storage
or recreate a second ECS/world model.

## Repository shape

The current repository already contains the implemented YorGL target at the
root and the separate `yorengine` target. The intended end state is:

```text
YOR/
  CMakeLists.txt
  src/                       # YorGL low-level implementation during migration
  bindings/                  # YorGL secondary language bindings
  yorengine/
    include/yorengine/       # public C++ engine contracts
    src/                     # C++ engine implementation
    tests/                   # native engine contract tests
  yorstudio/
    app/                      # executable entry point and lifecycle
    core/                     # editor services, commands, selection, state
    project/                  # manifests, launcher, paths, validation
    editor/                   # hierarchy, inspector, viewport, assets, console
    content/                  # import/build/cache services
    platform/                 # window, input, filesystem dialogs, clipboard
    ui/                       # UI port and adapters
      imgui/                  # first Dear ImGui adapter only
    tests/
  docs/
  samples/
  tools/
```

The physical move of the current root YorGL sources is postponed until package
and CI paths are prepared. That avoids a cosmetic tree move that breaks native
consumers without improving ownership.

## YOR project format

Every game or tool is a real project directory, not a loose collection of
files:

```text
MyGame/
  project.yorproject        # versioned human-owned manifest
  code/                      # C++ project code and public/private headers
    include/
    src/
    tests/
  assets/                    # source assets and import metadata
  scenes/                    # editable YorEngine scenes and prefabs
  shaders/                   # source shaders and includes
  config/                    # versioned game/editor configuration
  plugins/                   # explicitly enabled native extensions
  build/                     # local build output; never source content
  .yor/                      # hidden generated Studio state
    project.lock             # schema/toolchain lock, not user settings
    editor/                  # layout, selection, breakpoints, local state
    cache/                   # import and shader cache, disposable
    derived/                 # generated asset database and intermediate data
    logs/                    # Studio/import/build logs
    generated/               # generated code; never hand-edit
```

`project.yorproject`, `code`, `assets`, `scenes`, `shaders`, `config`, and
`plugins` are project inputs. `.yor`, `build`, and generated output are
disposable or machine-local and receive explicit ignore/rebuild rules. A
project must remain buildable without YorStudio; the launcher is not a runtime
dependency.

The manifest will be versioned and validated before any code or plugin is
loaded. Opening a project never executes project code. Build, plugin loading,
and editor scripting are explicit user actions with a safe mode that disables
third-party extensions.

## YorStudio modularity rule

Dear ImGui is the first UI implementation, not the editor architecture.

- `project`, `core`, `content`, and editor models contain no ImGui headers or
  ImGui types.
- A small `StudioUiPort` owns windows, menus, tables, text input, docking,
  notifications, and draw-list access needed by editor modules.
- `ui/imgui` adapts that port to Dear ImGui.
- A later Qt, native, or web UI adapter can replace the ImGui module without
  changing project parsing, commands, scene editing, asset import, or tests.
- Window/input/file-dialog services are separate platform ports; they are not
  hidden inside the ImGui adapter.

The first implementation may use one concrete ImGui adapter. The port exists
because UI ownership is an explicit product requirement, not as speculative
framework layering.

## Runtime/editor separation

YorStudio has two modes with different rules:

1. **Edit mode** mutates an editor transaction and produces undoable commands.
2. **Play mode** runs a YorEngine Runtime using a cloned/instantiated world;
   editor metadata and unsaved authoring state stay outside the runtime world.

The editor owns selection, gizmos, layout, import progress, diagnostics,
undo/redo, and project metadata. YorEngine owns transforms, entities,
components, systems, simulation time, and render snapshots. YorGL receives
render work only through YorEngine integration.

## Language policy

C++ is the primary implementation language for YorGL, YorEngine, and
YorStudio. Kotlin/Java/JVM code is secondary and may provide bindings, tooling
adapters, or integrations only after the native contract exists and is tested.
No engine or editor source of truth is allowed to migrate into Kotlin.
