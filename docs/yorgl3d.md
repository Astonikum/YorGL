# YorGL3D

`org.yorgl:yorgl3d` is an optional JVM module above the raw `org.yorgl:yorgl`
binding. It stays renderer-adjacent: no game rules, no UI toolkit, no asset
pipeline, and no engine loop ownership.

## Scope

- `Scene` owns a flat list of `SceneObject`s and updates active objects.
- `SceneObject` owns one `Transform` and a small component list.
- `Component` and `Script` provide attach/update hooks.
- `Scene.version()` increments when objects, active state, components, or
  transforms change, so clients can skip redundant scene bakes/uploads.
- `MeshComponent` stores local `x,y,z,r,g,b,a,u,v` vertices and bakes them to
  world-space vertices accepted by YorGL world rendering.
- `Camera` and `Light` hold low-level scene data for clients that want to keep
  render state beside scene objects.

## Use

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

Clients can map block grids, entities, particles, editor gizmos, or custom
objects into the same object/component path. Grid placement remains a client
policy; YorGL3D only stores arbitrary transforms and bakes vertices.
