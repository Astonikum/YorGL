#include "yorengine/api.h"

#include "yorengine/scene.hpp"

#include <cmath>
#include <cstring>
#include <exception>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

struct YorEngineScene {
    yorengine::Scene scene;
};

namespace {

thread_local std::string lastError;

YorEngineStatus failure(YorEngineStatus status, const char* message) noexcept {
    try {
        lastError = message ? message : "YorEngine operation failed";
    } catch (...) {
        lastError.clear();
    }
    return status;
}

template <typename Function>
YorEngineStatus invoke(Function&& function) noexcept {
    lastError.clear();
    try {
        return function();
    } catch (const std::invalid_argument& error) {
        return failure(YORENGINE_STATUS_INVALID_ARGUMENT, error.what());
    } catch (const std::out_of_range& error) {
        return failure(YORENGINE_STATUS_INVALID_ENTITY, error.what());
    } catch (const std::logic_error& error) {
        return failure(YORENGINE_STATUS_CONFLICT, error.what());
    } catch (const std::exception& error) {
        return failure(YORENGINE_STATUS_INTERNAL_ERROR, error.what());
    } catch (...) {
        return failure(YORENGINE_STATUS_INTERNAL_ERROR, "Unknown YorEngine exception");
    }
}

yorengine::EntityId toNative(YorEngineEntityId entity) noexcept {
    return {entity.index, entity.generation};
}

YorEngineEntityId toApi(yorengine::EntityId entity) noexcept {
    return {entity.index, entity.generation};
}

yorengine::Vec3 toNative(YorEngineVec3 value) noexcept {
    return {value.x, value.y, value.z};
}

YorEngineVec3 toApi(yorengine::Vec3 value) noexcept {
    return {value.x, value.y, value.z};
}

yorengine::Transform toNative(const YorEngineTransform& value) noexcept {
    return {toNative(value.position), {value.rotation.x, value.rotation.y, value.rotation.z, value.rotation.w}, toNative(value.scale)};
}

YorEngineTransform toApi(const yorengine::Transform& value) noexcept {
    return {
        toApi(value.position),
        {value.rotation.x, value.rotation.y, value.rotation.z, value.rotation.w},
        toApi(value.scale),
    };
}

YorEngineStatus requireScene(const YorEngineScene* scene) noexcept {
    return scene ? YORENGINE_STATUS_OK : failure(YORENGINE_STATUS_INVALID_ARGUMENT, "Scene must not be null");
}

YorEngineStatus requireEntity(const YorEngineScene* scene, YorEngineEntityId entity) noexcept {
    if (const auto status = requireScene(scene); status != YORENGINE_STATUS_OK) return status;
    return scene->scene.isAlive(toNative(entity))
        ? YORENGINE_STATUS_OK
        : failure(YORENGINE_STATUS_INVALID_ENTITY, "EntityId is invalid or stale");
}

bool validLightKind(YorEngineLightKind kind) noexcept {
    return kind == YORENGINE_LIGHT_DIRECTIONAL || kind == YORENGINE_LIGHT_POINT || kind == YORENGINE_LIGHT_SPOT;
}

yorengine::LightComponent::Kind toNative(YorEngineLightKind kind) noexcept {
    switch (kind) {
        case YORENGINE_LIGHT_POINT: return yorengine::LightComponent::Kind::Point;
        case YORENGINE_LIGHT_SPOT: return yorengine::LightComponent::Kind::Spot;
        case YORENGINE_LIGHT_DIRECTIONAL:
        default: return yorengine::LightComponent::Kind::Directional;
    }
}

YorEngineLightKind toApi(yorengine::LightComponent::Kind kind) noexcept {
    switch (kind) {
        case yorengine::LightComponent::Kind::Point: return YORENGINE_LIGHT_POINT;
        case yorengine::LightComponent::Kind::Spot: return YORENGINE_LIGHT_SPOT;
        case yorengine::LightComponent::Kind::Directional:
        default: return YORENGINE_LIGHT_DIRECTIONAL;
    }
}

bool validCamera(const YorEngineCameraState& camera) noexcept {
    return std::isfinite(camera.fovYDegrees) && camera.fovYDegrees > 0.0f && camera.fovYDegrees < 180.0f &&
           std::isfinite(camera.aspectRatio) && camera.aspectRatio > 0.0f &&
           std::isfinite(camera.nearPlane) && camera.nearPlane > 0.0f &&
           std::isfinite(camera.farPlane) && camera.farPlane > camera.nearPlane;
}

bool validLight(const YorEngineLightState& light) noexcept {
    return validLightKind(light.kind) &&
           std::isfinite(light.color.x) && light.color.x >= 0.0f &&
           std::isfinite(light.color.y) && light.color.y >= 0.0f &&
           std::isfinite(light.color.z) && light.color.z >= 0.0f &&
           std::isfinite(light.intensity) && light.intensity >= 0.0f &&
           std::isfinite(light.range) && light.range > 0.0f &&
           std::isfinite(light.innerConeDegrees) && light.innerConeDegrees >= 0.0f &&
           std::isfinite(light.outerConeDegrees) && light.outerConeDegrees >= light.innerConeDegrees &&
           light.outerConeDegrees <= 180.0f;
}

YorEngineMeshVertex toApi(const yorengine::MeshVertex& value) noexcept {
    return {toApi(value.position), value.r, value.g, value.b, value.a, value.u, value.v};
}

yorengine::MeshVertex toNative(const YorEngineMeshVertex& value) noexcept {
    return {toNative(value.position), value.r, value.g, value.b, value.a, value.u, value.v};
}

} // namespace

extern "C" {

YorEngineScene* yorengineSceneCreate(void) {
    lastError.clear();
    try {
        return new YorEngineScene{};
    } catch (const std::exception& error) {
        failure(YORENGINE_STATUS_INTERNAL_ERROR, error.what());
        return nullptr;
    } catch (...) {
        failure(YORENGINE_STATUS_INTERNAL_ERROR, "Unknown YorEngine allocation failure");
        return nullptr;
    }
}

void yorengineSceneDestroy(YorEngineScene* scene) {
    lastError.clear();
    delete scene;
}

const char* yorengineLastError(void) {
    return lastError.c_str();
}

const char* yorengineStatusName(YorEngineStatus status) {
    switch (status) {
        case YORENGINE_STATUS_OK: return "ok";
        case YORENGINE_STATUS_INVALID_ARGUMENT: return "invalid_argument";
        case YORENGINE_STATUS_INVALID_ENTITY: return "invalid_entity";
        case YORENGINE_STATUS_ALREADY_EXISTS: return "already_exists";
        case YORENGINE_STATUS_NOT_FOUND: return "not_found";
        case YORENGINE_STATUS_CONFLICT: return "conflict";
        case YORENGINE_STATUS_BUFFER_TOO_SMALL: return "buffer_too_small";
        case YORENGINE_STATUS_INTERNAL_ERROR: return "internal_error";
        default: return "unknown";
    }
}

YorEngineStatus yorengineSceneCreateEntity(YorEngineScene* scene, YorEngineEntityId* outEntity) {
    return invoke([&] {
        if (const auto status = requireScene(scene); status != YORENGINE_STATUS_OK) return status;
        if (!outEntity) return failure(YORENGINE_STATUS_INVALID_ARGUMENT, "Output EntityId must not be null");
        *outEntity = toApi(scene->scene.createEntity());
        return YORENGINE_STATUS_OK;
    });
}

YorEngineStatus yorengineSceneDestroyEntity(YorEngineScene* scene, YorEngineEntityId entity) {
    return invoke([&] {
        if (const auto status = requireEntity(scene, entity); status != YORENGINE_STATUS_OK) return status;
        if (!scene->scene.destroyEntity(toNative(entity))) return failure(YORENGINE_STATUS_INVALID_ENTITY, "EntityId is invalid or stale");
        return YORENGINE_STATUS_OK;
    });
}

int yorengineSceneIsAlive(const YorEngineScene* scene, YorEngineEntityId entity) {
    lastError.clear();
    return scene && scene->scene.isAlive(toNative(entity)) ? 1 : 0;
}

uint64_t yorengineSceneVersion(const YorEngineScene* scene) {
    lastError.clear();
    return scene ? scene->scene.version() : 0;
}

YorEngineStatus yorengineSceneSetParent(YorEngineScene* scene, YorEngineEntityId child, YorEngineEntityId parent) {
    return invoke([&] {
        if (const auto status = requireEntity(scene, child); status != YORENGINE_STATUS_OK) return status;
        if (const auto status = requireEntity(scene, parent); status != YORENGINE_STATUS_OK) return status;
        if (!scene->scene.setParent(toNative(child), toNative(parent))) {
            return failure(YORENGINE_STATUS_CONFLICT, "Parent assignment would create a cycle");
        }
        return YORENGINE_STATUS_OK;
    });
}

YorEngineStatus yorengineSceneClearParent(YorEngineScene* scene, YorEngineEntityId child) {
    return invoke([&] {
        if (const auto status = requireEntity(scene, child); status != YORENGINE_STATUS_OK) return status;
        return scene->scene.clearParent(toNative(child))
            ? YORENGINE_STATUS_OK
            : failure(YORENGINE_STATUS_INVALID_ENTITY, "EntityId is invalid or stale");
    });
}

YorEngineStatus yorengineSceneGetParent(const YorEngineScene* scene, YorEngineEntityId child, YorEngineEntityId* outParent) {
    return invoke([&] {
        if (const auto status = requireEntity(scene, child); status != YORENGINE_STATUS_OK) return status;
        if (!outParent) return failure(YORENGINE_STATUS_INVALID_ARGUMENT, "Output parent EntityId must not be null");
        *outParent = toApi(scene->scene.parent(toNative(child)));
        return YORENGINE_STATUS_OK;
    });
}

YorEngineStatus yorengineSceneGetTransform(const YorEngineScene* scene, YorEngineEntityId entity, YorEngineTransform* outTransform) {
    return invoke([&] {
        if (const auto status = requireEntity(scene, entity); status != YORENGINE_STATUS_OK) return status;
        if (!outTransform) return failure(YORENGINE_STATUS_INVALID_ARGUMENT, "Output transform must not be null");
        *outTransform = toApi(scene->scene.transform(toNative(entity)));
        return YORENGINE_STATUS_OK;
    });
}

YorEngineStatus yorengineSceneSetTransform(YorEngineScene* scene, YorEngineEntityId entity, const YorEngineTransform* transform) {
    return invoke([&] {
        if (const auto status = requireEntity(scene, entity); status != YORENGINE_STATUS_OK) return status;
        if (!transform) return failure(YORENGINE_STATUS_INVALID_ARGUMENT, "Transform must not be null");
        scene->scene.setTransform(toNative(entity), toNative(*transform));
        return YORENGINE_STATUS_OK;
    });
}

YorEngineStatus yorengineSceneGetWorldMatrix(const YorEngineScene* scene, YorEngineEntityId entity, YorEngineMat4* outMatrix) {
    return invoke([&] {
        if (const auto status = requireEntity(scene, entity); status != YORENGINE_STATUS_OK) return status;
        if (!outMatrix) return failure(YORENGINE_STATUS_INVALID_ARGUMENT, "Output matrix must not be null");
        const yorengine::Mat4 matrix = scene->scene.worldMatrix(toNative(entity));
        for (uint32_t column = 0; column < 4; ++column) {
            for (uint32_t row = 0; row < 4; ++row) {
                outMatrix->values[column * 4 + row] = matrix.at(row, column);
            }
        }
        return YORENGINE_STATUS_OK;
    });
}

YorEngineStatus yorengineSceneGetActive(const YorEngineScene* scene, YorEngineEntityId entity, int* outActive) {
    return invoke([&] {
        if (const auto status = requireEntity(scene, entity); status != YORENGINE_STATUS_OK) return status;
        if (!outActive) return failure(YORENGINE_STATUS_INVALID_ARGUMENT, "Output active state must not be null");
        *outActive = scene->scene.active(toNative(entity)) ? 1 : 0;
        return YORENGINE_STATUS_OK;
    });
}

YorEngineStatus yorengineSceneSetActive(YorEngineScene* scene, YorEngineEntityId entity, int active) {
    return invoke([&] {
        if (const auto status = requireEntity(scene, entity); status != YORENGINE_STATUS_OK) return status;
        scene->scene.setActive(toNative(entity), active != 0);
        return YORENGINE_STATUS_OK;
    });
}

YorEngineStatus yorengineSceneUpdate(YorEngineScene* scene, double deltaSeconds) {
    return invoke([&] {
        if (const auto status = requireScene(scene); status != YORENGINE_STATUS_OK) return status;
        scene->scene.update(deltaSeconds);
        return YORENGINE_STATUS_OK;
    });
}

YorEngineStatus yorengineSceneSetProperty(YorEngineScene* scene, YorEngineEntityId entity, const char* key, const char* value) {
    return invoke([&] {
        if (const auto status = requireEntity(scene, entity); status != YORENGINE_STATUS_OK) return status;
        if (!key || !value) return failure(YORENGINE_STATUS_INVALID_ARGUMENT, "Property key and value must not be null");
        scene->scene.setProperty(toNative(entity), key, value);
        return YORENGINE_STATUS_OK;
    });
}

YorEngineStatus yorengineSceneGetProperty(const YorEngineScene* scene, YorEngineEntityId entity, const char* key, char* outValue, uint32_t capacity, uint32_t* outLength) {
    return invoke([&] {
        if (const auto status = requireEntity(scene, entity); status != YORENGINE_STATUS_OK) return status;
        if (!key || !outLength) return failure(YORENGINE_STATUS_INVALID_ARGUMENT, "Property key and output length must not be null");
        const auto value = scene->scene.property(toNative(entity), key);
        if (!value) return failure(YORENGINE_STATUS_NOT_FOUND, "Property does not exist");
        if (value->size() > std::numeric_limits<uint32_t>::max()) return failure(YORENGINE_STATUS_INTERNAL_ERROR, "Property is too large");
        *outLength = static_cast<uint32_t>(value->size());
        if (capacity == 0) return YORENGINE_STATUS_OK;
        if (!outValue) return failure(YORENGINE_STATUS_INVALID_ARGUMENT, "Property output buffer must not be null");
        if (capacity <= value->size()) return failure(YORENGINE_STATUS_BUFFER_TOO_SMALL, "Property output buffer is too small");
        std::memcpy(outValue, value->data(), value->size());
        outValue[value->size()] = '\0';
        return YORENGINE_STATUS_OK;
    });
}

YorEngineStatus yorengineSceneAddMesh(YorEngineScene* scene, YorEngineEntityId entity) {
    return invoke([&] {
        if (const auto status = requireEntity(scene, entity); status != YORENGINE_STATUS_OK) return status;
        if (scene->scene.component<yorengine::MeshComponent>(toNative(entity))) return failure(YORENGINE_STATUS_ALREADY_EXISTS, "Entity already has a mesh component");
        scene->scene.emplaceComponent<yorengine::MeshComponent>(toNative(entity));
        return YORENGINE_STATUS_OK;
    });
}

YorEngineStatus yorengineSceneRemoveMesh(YorEngineScene* scene, YorEngineEntityId entity) {
    return invoke([&] {
        if (const auto status = requireEntity(scene, entity); status != YORENGINE_STATUS_OK) return status;
        return scene->scene.removeComponent<yorengine::MeshComponent>(toNative(entity))
            ? YORENGINE_STATUS_OK
            : failure(YORENGINE_STATUS_NOT_FOUND, "Entity has no mesh component");
    });
}

YorEngineStatus yorengineSceneSetMeshVertices(YorEngineScene* scene, YorEngineEntityId entity, const YorEngineMeshVertex* vertices, uint32_t vertexCount) {
    return invoke([&] {
        if (const auto status = requireEntity(scene, entity); status != YORENGINE_STATUS_OK) return status;
        if (vertexCount > 0 && !vertices) return failure(YORENGINE_STATUS_INVALID_ARGUMENT, "Mesh vertices must not be null when vertexCount is non-zero");
        auto* mesh = scene->scene.component<yorengine::MeshComponent>(toNative(entity));
        if (!mesh) return failure(YORENGINE_STATUS_NOT_FOUND, "Entity has no mesh component");
        std::vector<yorengine::MeshVertex> nativeVertices;
        nativeVertices.reserve(vertexCount);
        for (uint32_t index = 0; index < vertexCount; ++index) nativeVertices.push_back(toNative(vertices[index]));
        mesh->setVertices(std::move(nativeVertices));
        return YORENGINE_STATUS_OK;
    });
}

YorEngineStatus yorengineSceneGetMeshVertices(const YorEngineScene* scene, YorEngineEntityId entity, YorEngineMeshVertex* outVertices, uint32_t capacity, uint32_t* outCount) {
    return invoke([&] {
        if (const auto status = requireEntity(scene, entity); status != YORENGINE_STATUS_OK) return status;
        if (!outCount) return failure(YORENGINE_STATUS_INVALID_ARGUMENT, "Output vertex count must not be null");
        const auto* mesh = scene->scene.component<yorengine::MeshComponent>(toNative(entity));
        if (!mesh) return failure(YORENGINE_STATUS_NOT_FOUND, "Entity has no mesh component");
        const auto& vertices = mesh->vertices();
        if (vertices.size() > std::numeric_limits<uint32_t>::max()) return failure(YORENGINE_STATUS_INTERNAL_ERROR, "Mesh has too many vertices");
        *outCount = static_cast<uint32_t>(vertices.size());
        if (vertices.empty()) return YORENGINE_STATUS_OK;
        if (!outVertices) return failure(YORENGINE_STATUS_INVALID_ARGUMENT, "Output vertices must not be null");
        if (capacity < vertices.size()) return failure(YORENGINE_STATUS_BUFFER_TOO_SMALL, "Output vertex buffer is too small");
        for (uint32_t index = 0; index < *outCount; ++index) outVertices[index] = toApi(vertices[index]);
        return YORENGINE_STATUS_OK;
    });
}

YorEngineStatus yorengineSceneAddCamera(YorEngineScene* scene, YorEngineEntityId entity) {
    return invoke([&] {
        if (const auto status = requireEntity(scene, entity); status != YORENGINE_STATUS_OK) return status;
        if (scene->scene.component<yorengine::CameraComponent>(toNative(entity))) return failure(YORENGINE_STATUS_ALREADY_EXISTS, "Entity already has a camera component");
        scene->scene.emplaceComponent<yorengine::CameraComponent>(toNative(entity));
        return YORENGINE_STATUS_OK;
    });
}

YorEngineStatus yorengineSceneRemoveCamera(YorEngineScene* scene, YorEngineEntityId entity) {
    return invoke([&] {
        if (const auto status = requireEntity(scene, entity); status != YORENGINE_STATUS_OK) return status;
        return scene->scene.removeComponent<yorengine::CameraComponent>(toNative(entity))
            ? YORENGINE_STATUS_OK
            : failure(YORENGINE_STATUS_NOT_FOUND, "Entity has no camera component");
    });
}

YorEngineStatus yorengineSceneGetCamera(const YorEngineScene* scene, YorEngineEntityId entity, YorEngineCameraState* outCamera) {
    return invoke([&] {
        if (const auto status = requireEntity(scene, entity); status != YORENGINE_STATUS_OK) return status;
        if (!outCamera) return failure(YORENGINE_STATUS_INVALID_ARGUMENT, "Output camera must not be null");
        const auto* camera = scene->scene.component<yorengine::CameraComponent>(toNative(entity));
        if (!camera) return failure(YORENGINE_STATUS_NOT_FOUND, "Entity has no camera component");
        *outCamera = {camera->fovYDegrees(), camera->aspectRatio(), camera->nearPlane(), camera->farPlane()};
        return YORENGINE_STATUS_OK;
    });
}

YorEngineStatus yorengineSceneSetCamera(YorEngineScene* scene, YorEngineEntityId entity, const YorEngineCameraState* camera) {
    return invoke([&] {
        if (const auto status = requireEntity(scene, entity); status != YORENGINE_STATUS_OK) return status;
        if (!camera) return failure(YORENGINE_STATUS_INVALID_ARGUMENT, "Camera must not be null");
        if (!validCamera(*camera)) return failure(YORENGINE_STATUS_INVALID_ARGUMENT, "Camera values are invalid");
        auto* target = scene->scene.component<yorengine::CameraComponent>(toNative(entity));
        if (!target) return failure(YORENGINE_STATUS_NOT_FOUND, "Entity has no camera component");
        target->setFovYDegrees(camera->fovYDegrees);
        target->setAspectRatio(camera->aspectRatio);
        if (camera->farPlane > target->nearPlane()) {
            target->setFarPlane(camera->farPlane);
            target->setNearPlane(camera->nearPlane);
        } else {
            target->setNearPlane(camera->nearPlane);
            target->setFarPlane(camera->farPlane);
        }
        return YORENGINE_STATUS_OK;
    });
}

YorEngineStatus yorengineSceneAddLight(YorEngineScene* scene, YorEngineEntityId entity, YorEngineLightKind kind) {
    return invoke([&] {
        if (const auto status = requireEntity(scene, entity); status != YORENGINE_STATUS_OK) return status;
        if (!validLightKind(kind)) return failure(YORENGINE_STATUS_INVALID_ARGUMENT, "Light kind is invalid");
        if (scene->scene.component<yorengine::LightComponent>(toNative(entity))) return failure(YORENGINE_STATUS_ALREADY_EXISTS, "Entity already has a light component");
        scene->scene.emplaceComponent<yorengine::LightComponent>(toNative(entity), toNative(kind));
        return YORENGINE_STATUS_OK;
    });
}

YorEngineStatus yorengineSceneRemoveLight(YorEngineScene* scene, YorEngineEntityId entity) {
    return invoke([&] {
        if (const auto status = requireEntity(scene, entity); status != YORENGINE_STATUS_OK) return status;
        return scene->scene.removeComponent<yorengine::LightComponent>(toNative(entity))
            ? YORENGINE_STATUS_OK
            : failure(YORENGINE_STATUS_NOT_FOUND, "Entity has no light component");
    });
}

YorEngineStatus yorengineSceneGetLight(const YorEngineScene* scene, YorEngineEntityId entity, YorEngineLightState* outLight) {
    return invoke([&] {
        if (const auto status = requireEntity(scene, entity); status != YORENGINE_STATUS_OK) return status;
        if (!outLight) return failure(YORENGINE_STATUS_INVALID_ARGUMENT, "Output light must not be null");
        const auto* light = scene->scene.component<yorengine::LightComponent>(toNative(entity));
        if (!light) return failure(YORENGINE_STATUS_NOT_FOUND, "Entity has no light component");
        *outLight = {
            toApi(light->kind()),
            toApi(light->color()),
            light->intensity(),
            light->range(),
            light->innerConeDegrees(),
            light->outerConeDegrees(),
        };
        return YORENGINE_STATUS_OK;
    });
}

YorEngineStatus yorengineSceneSetLight(YorEngineScene* scene, YorEngineEntityId entity, const YorEngineLightState* light) {
    return invoke([&] {
        if (const auto status = requireEntity(scene, entity); status != YORENGINE_STATUS_OK) return status;
        if (!light) return failure(YORENGINE_STATUS_INVALID_ARGUMENT, "Light must not be null");
        if (!validLight(*light)) return failure(YORENGINE_STATUS_INVALID_ARGUMENT, "Light values are invalid");
        auto* target = scene->scene.component<yorengine::LightComponent>(toNative(entity));
        if (!target) return failure(YORENGINE_STATUS_NOT_FOUND, "Entity has no light component");
        if (target->kind() != toNative(light->kind)) return failure(YORENGINE_STATUS_CONFLICT, "Changing light kind requires remove and add");
        target->setColor(toNative(light->color));
        target->setIntensity(light->intensity);
        target->setRange(light->range);
        target->setCone(light->innerConeDegrees, light->outerConeDegrees);
        return YORENGINE_STATUS_OK;
    });
}

} // extern "C"
