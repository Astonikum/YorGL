# YorEngine

YorEngine is the C++ engine-side project in this repository. It is separate
from the low-level YorGL renderer project even though both are built from one
repository for now. JVM/Kotlin code is a secondary binding/adapter layer.

The existing `org.yorgl:yorengine` JVM artifact is an interim migration slice
left by the old `YorGL3D` module. It must not become the engine core or receive
new gameplay/runtime ownership.

## Current C++ Scope

The first native YorEngine slice is now a standalone C++20 static library
(`yorengine`) with public headers under `yorengine/include/yorengine`. It
currently provides:

- `Vec3`, `Quaternion`, `Mat4`, and `Transform` math with explicit column-major
  matrix semantics and `T * R * S` local transforms;
- generation-checked `EntityId` values with stale-handle rejection and index
  reuse;
- `Scene` entity creation/destruction, deterministic entity enumeration,
  parent/child hierarchy, cycle rejection, local/world transforms, active state,
  cached world transforms with hierarchy invalidation, version tracking, and
  string metadata;
- a snapshot-safe `Component` lifecycle with attach/detach/update hooks and
  safe structural removal during update;
- generic custom components plus initial `MeshComponent`, `CameraComponent`,
  and `LightComponent` data contracts with input validation.

The native scene core deliberately does not claim to be a complete runtime
yet: asset loading, render submission, physics, animation, audio, input,
serialization, editor tooling, and the remainder of the public binding
contract are later roadmap systems.

## C Binding Boundary

`yorengine_api` is the first stable C binding boundary for the native core. It
uses an opaque `YorEngineScene*`, generation-checked `YorEngineEntityId`
values, explicit `YorEngineStatus` results, and a thread-local diagnostic from
`yorengineLastError()`. C++ exceptions never cross this ABI. The current C
surface covers:

- scene/entity lifetime, parent links, transforms, world matrices, active state,
  updates, versioning, and string metadata;
- built-in mesh vertex transfer with caller-owned buffers;
- camera and light component creation, queries, and validated updates.

`YorEngineMat4` values are column-major, matching the native `Mat4` contract.
Transform position, rotation, and scale values must be finite; invalid values
are rejected before they can poison a world-matrix cache. World matrices are
cached behind a scene transform revision and are recomputed after a local
transform or hierarchy change.
The error string remains valid until the next YorEngine C API call on the same
thread. Generic user-defined C++ components are intentionally not exposed as
fake C handles; a later binding design must specify their ownership and
callback lifecycle first.

Minimal native usage:

```cpp
#include "yorengine/scene.hpp"

yorengine::Scene scene;
const yorengine::EntityId parent = scene.createEntity();
const yorengine::EntityId child = scene.createEntity();
scene.setParent(child, parent);
scene.emplaceComponent<yorengine::CameraComponent>(parent);
scene.update(1.0 / 60.0);
```

`Scene` owns entities and components. Components must not retain raw pointers
to scene storage; use their attach/detach hooks and stable `EntityId` values.
Structural changes made by a component during `Scene::update` are handled by a
snapshot of the current component list and are visible on the next relevant
iteration.

## Transitional JVM Scope

The interim JVM slice currently provides a first engine-facing scene layer:

- `Scene` owns scene objects and frame updates;
- `SceneObject` owns a transform, parent/child links, components, and custom
  properties;
- `Component` and `Script` provide attach/update hooks;
- `Camera` and `Light` keep render-facing view and lighting data on ordinary
  scene objects;
- `MeshComponent` stores generic `x,y,z,r,g,b,a,u,v` vertices and bakes them
  through an object's world transform;
- `Material` carries client-side material identity until a real material system
  is added;
- `Scene.version()` allows clients to skip unchanged mesh uploads.

This JVM API remains migration input and is not a second engine
implementation. New engine systems must be implemented in C++ first; a JVM
binding is added only after the native contract is stable and tested. The
remaining migration work and missing systems are tracked in
[`yorengine-roadmap.md`](yorengine-roadmap.md).

## Dependency Direction

```text
game/application integration
          |
          v
      YorEngine C++  --->  YorGL C++ API  --->  backend
           |
           +---- secondary JVM/Kotlin binding
```

YorEngine does not own a renderer backend, Minecraft concepts, networking,
asset extraction, or a retained UI toolkit. A client translates its own world
and assets into engine data, and the C++ engine translates renderable data into
YorGL calls.

## Transitional JVM Example

```java
Scene scene = new Scene();
scene.createObject()
    .add(new MeshComponent(vertices))
    .transform()
    .position(10.0f, 2.0f, -3.0f);

scene.update(deltaSeconds);
float[] worldVertices = scene.bakeWorldVertices();
renderer.worldUploadMesh(worldVertices, worldVertices.length);
```

Parent transforms remain editable as local transforms:

```java
SceneObject parent = scene.createObject();
parent.transform().position(10.0f, 0.0f, 0.0f);

SceneObject child = scene.createObject(parent);
child.transform().position(1.0f, 0.0f, 0.0f);
```
