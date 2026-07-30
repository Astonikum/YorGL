#pragma once

#include "export.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum YorEngineStatus {
    YORENGINE_STATUS_OK = 0,
    YORENGINE_STATUS_INVALID_ARGUMENT = 1,
    YORENGINE_STATUS_INVALID_ENTITY = 2,
    YORENGINE_STATUS_ALREADY_EXISTS = 3,
    YORENGINE_STATUS_NOT_FOUND = 4,
    YORENGINE_STATUS_CONFLICT = 5,
    YORENGINE_STATUS_BUFFER_TOO_SMALL = 6,
    YORENGINE_STATUS_INTERNAL_ERROR = 7
} YorEngineStatus;

typedef struct YorEngineScene YorEngineScene;

typedef struct YorEngineEntityId {
    uint32_t index;
    uint32_t generation;
} YorEngineEntityId;

typedef struct YorEngineVec3 {
    float x;
    float y;
    float z;
} YorEngineVec3;

typedef struct YorEngineQuaternion {
    float x;
    float y;
    float z;
    float w;
} YorEngineQuaternion;

typedef struct YorEngineTransform {
    YorEngineVec3 position;
    YorEngineQuaternion rotation;
    YorEngineVec3 scale;
} YorEngineTransform;

typedef struct YorEngineMat4 {
    float values[16];
} YorEngineMat4;

typedef struct YorEngineMeshVertex {
    YorEngineVec3 position;
    float r;
    float g;
    float b;
    float a;
    float u;
    float v;
} YorEngineMeshVertex;

typedef struct YorEngineCameraState {
    float fovYDegrees;
    float aspectRatio;
    float nearPlane;
    float farPlane;
} YorEngineCameraState;

typedef enum YorEngineLightKind {
    YORENGINE_LIGHT_DIRECTIONAL = 0,
    YORENGINE_LIGHT_POINT = 1,
    YORENGINE_LIGHT_SPOT = 2
} YorEngineLightKind;

typedef struct YorEngineLightState {
    YorEngineLightKind kind;
    YorEngineVec3 color;
    float intensity;
    float range;
    float innerConeDegrees;
    float outerConeDegrees;
} YorEngineLightState;

YORENGINE_API YorEngineScene* yorengineSceneCreate(void);
YORENGINE_API void yorengineSceneDestroy(YorEngineScene* scene);

YORENGINE_API const char* yorengineLastError(void);
YORENGINE_API const char* yorengineStatusName(YorEngineStatus status);

YORENGINE_API YorEngineStatus yorengineSceneCreateEntity(YorEngineScene* scene, YorEngineEntityId* outEntity);
YORENGINE_API YorEngineStatus yorengineSceneDestroyEntity(YorEngineScene* scene, YorEngineEntityId entity);
YORENGINE_API int yorengineSceneIsAlive(const YorEngineScene* scene, YorEngineEntityId entity);
YORENGINE_API uint64_t yorengineSceneVersion(const YorEngineScene* scene);

YORENGINE_API YorEngineStatus yorengineSceneSetParent(YorEngineScene* scene, YorEngineEntityId child, YorEngineEntityId parent);
YORENGINE_API YorEngineStatus yorengineSceneClearParent(YorEngineScene* scene, YorEngineEntityId child);
YORENGINE_API YorEngineStatus yorengineSceneGetParent(const YorEngineScene* scene, YorEngineEntityId child, YorEngineEntityId* outParent);

YORENGINE_API YorEngineStatus yorengineSceneGetTransform(const YorEngineScene* scene, YorEngineEntityId entity, YorEngineTransform* outTransform);
YORENGINE_API YorEngineStatus yorengineSceneSetTransform(YorEngineScene* scene, YorEngineEntityId entity, const YorEngineTransform* transform);
YORENGINE_API YorEngineStatus yorengineSceneGetWorldMatrix(const YorEngineScene* scene, YorEngineEntityId entity, YorEngineMat4* outMatrix);
YORENGINE_API YorEngineStatus yorengineSceneGetActive(const YorEngineScene* scene, YorEngineEntityId entity, int* outActive);
YORENGINE_API YorEngineStatus yorengineSceneSetActive(YorEngineScene* scene, YorEngineEntityId entity, int active);
YORENGINE_API YorEngineStatus yorengineSceneUpdate(YorEngineScene* scene, double deltaSeconds);

YORENGINE_API YorEngineStatus yorengineSceneSetProperty(YorEngineScene* scene, YorEngineEntityId entity, const char* key, const char* value);
YORENGINE_API YorEngineStatus yorengineSceneGetProperty(const YorEngineScene* scene, YorEngineEntityId entity, const char* key, char* outValue, uint32_t capacity, uint32_t* outLength);

YORENGINE_API YorEngineStatus yorengineSceneAddMesh(YorEngineScene* scene, YorEngineEntityId entity);
YORENGINE_API YorEngineStatus yorengineSceneRemoveMesh(YorEngineScene* scene, YorEngineEntityId entity);
YORENGINE_API YorEngineStatus yorengineSceneSetMeshVertices(YorEngineScene* scene, YorEngineEntityId entity, const YorEngineMeshVertex* vertices, uint32_t vertexCount);
YORENGINE_API YorEngineStatus yorengineSceneGetMeshVertices(const YorEngineScene* scene, YorEngineEntityId entity, YorEngineMeshVertex* outVertices, uint32_t capacity, uint32_t* outCount);

YORENGINE_API YorEngineStatus yorengineSceneAddCamera(YorEngineScene* scene, YorEngineEntityId entity);
YORENGINE_API YorEngineStatus yorengineSceneRemoveCamera(YorEngineScene* scene, YorEngineEntityId entity);
YORENGINE_API YorEngineStatus yorengineSceneGetCamera(const YorEngineScene* scene, YorEngineEntityId entity, YorEngineCameraState* outCamera);
YORENGINE_API YorEngineStatus yorengineSceneSetCamera(YorEngineScene* scene, YorEngineEntityId entity, const YorEngineCameraState* camera);

YORENGINE_API YorEngineStatus yorengineSceneAddLight(YorEngineScene* scene, YorEngineEntityId entity, YorEngineLightKind kind);
YORENGINE_API YorEngineStatus yorengineSceneRemoveLight(YorEngineScene* scene, YorEngineEntityId entity);
YORENGINE_API YorEngineStatus yorengineSceneGetLight(const YorEngineScene* scene, YorEngineEntityId entity, YorEngineLightState* outLight);
YORENGINE_API YorEngineStatus yorengineSceneSetLight(YorEngineScene* scene, YorEngineEntityId entity, const YorEngineLightState* light);

#ifdef __cplusplus
}
#endif
