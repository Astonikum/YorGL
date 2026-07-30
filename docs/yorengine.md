# YorEngine

`org.yorgl:yorengine` is the engine-side JVM module in this repository. It is
separate from the low-level `org.yorgl:yorgl` renderer artifact even though both
are built from one repository for now.

## Current Scope

The current module provides the first engine-facing scene layer:

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

This is a foundation, not a claim that the engine is already a complete game
runtime. The missing systems and their acceptance criteria are tracked in
[`yorengine-roadmap.md`](yorengine-roadmap.md).

## Dependency Direction

```text
game/application integration
          |
          v
      YorEngine  --->  YorGL JVM binding  --->  YorGL C ABI  --->  backend
```

YorEngine does not own a renderer backend, Minecraft concepts, networking,
asset extraction, or a retained UI toolkit. A client translates its own world
and assets into engine data, and the engine translates renderable data into
YorGL calls.

## Example

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
