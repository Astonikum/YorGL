# YorGL3D

`org.yorgl:yorgl3d` is an optional JVM module above the raw `org.yorgl:yorgl`
binding. It stays renderer-adjacent: no game rules, no UI toolkit, no asset
pipeline, and no engine loop ownership.

## Scope

- `Scene` owns a list of `SceneObject`s and updates active objects.
- `SceneObject` owns one `Transform`, a small add/remove component list, and a
  string-keyed custom property map for client metadata such as source ids or
  editor tags.
- `SceneObject` can parent child objects. Child world matrices inherit parent
  transforms while local transforms stay editable.
- `Component` and `Script` provide attach/update hooks.
- Component updates iterate over a frame-local snapshot, so a component can add
  or remove components during its own update without invalidating the tick.
- `Scene.version()` increments when objects, active state, components, or
  transforms, properties, or removals change, so clients can skip redundant
  scene bakes/uploads.
- `MeshComponent` stores local `x,y,z,r,g,b,a,u,v` vertices, an optional
  `Material` reference, and bakes them through the owning object's world matrix
  to vertices accepted by YorGL world rendering.
- `Camera` and `Light` hold low-level scene data for clients that want to keep
  render state beside scene objects.
- `Material` is a low-level color/name holder for clients that need material
  identity beside mesh objects before mapping it to renderer-specific states.

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

Parented objects keep local transforms:

```java
SceneObject parent = scene.createObject();
parent.transform().position(10.0f, 0.0f, 0.0f);

SceneObject child = scene.createObject(parent);
child.transform().position(1.0f, 0.0f, 0.0f);
```

Clients can map block grids, entities, particles, editor gizmos, or custom
objects into the same object/component path. Grid placement remains a client
policy; YorGL3D only stores arbitrary transforms and bakes vertices.
