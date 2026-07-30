#include "yorengine/api.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <stdexcept>

#define CHECK(expression) \
    do { \
        if (!(expression)) throw std::runtime_error("check failed: " #expression); \
    } while (false)

namespace {

void checkStatus(YorEngineStatus actual, YorEngineStatus expected) {
    if (actual != expected) {
        throw std::runtime_error(std::string("unexpected status: ") + yorengineStatusName(actual) + ": " + yorengineLastError());
    }
}

void checkNear(float expected, float actual) {
    CHECK(std::fabs(expected - actual) < 0.0001f);
}

} // namespace

int main() {
    try {
        YorEngineEntityId invalid{};
        checkStatus(yorengineSceneCreateEntity(nullptr, &invalid), YORENGINE_STATUS_INVALID_ARGUMENT);
        CHECK(std::string(yorengineLastError()).find("null") != std::string::npos);

        YorEngineScene* scene = yorengineSceneCreate();
        CHECK(scene != nullptr);

        YorEngineEntityId parent{};
        YorEngineEntityId child{};
        checkStatus(yorengineSceneCreateEntity(scene, &parent), YORENGINE_STATUS_OK);
        checkStatus(yorengineSceneCreateEntity(scene, &child), YORENGINE_STATUS_OK);
        checkStatus(yorengineSceneSetParent(scene, child, parent), YORENGINE_STATUS_OK);

        YorEngineTransform parentTransform{{10.0f, 2.0f, -3.0f}, {0.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}};
        YorEngineTransform childTransform{{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}, {2.0f, 2.0f, 2.0f}};
        checkStatus(yorengineSceneSetTransform(scene, parent, &parentTransform), YORENGINE_STATUS_OK);
        checkStatus(yorengineSceneSetTransform(scene, child, &childTransform), YORENGINE_STATUS_OK);
        YorEngineTransform invalidTransform = childTransform;
        invalidTransform.position.x = std::numeric_limits<float>::quiet_NaN();
        checkStatus(yorengineSceneSetTransform(scene, child, &invalidTransform), YORENGINE_STATUS_INVALID_ARGUMENT);

        YorEngineMat4 world{};
        checkStatus(yorengineSceneGetWorldMatrix(scene, child, &world), YORENGINE_STATUS_OK);
        checkNear(11.0f, world.values[12]);
        checkNear(2.0f, world.values[13]);
        checkNear(-3.0f, world.values[14]);
        parentTransform.position.x = 12.0f;
        checkStatus(yorengineSceneSetTransform(scene, parent, &parentTransform), YORENGINE_STATUS_OK);
        checkStatus(yorengineSceneGetWorldMatrix(scene, child, &world), YORENGINE_STATUS_OK);
        checkNear(13.0f, world.values[12]);

        YorEngineEntityId actualParent{};
        checkStatus(yorengineSceneGetParent(scene, child, &actualParent), YORENGINE_STATUS_OK);
        CHECK(actualParent.index == parent.index && actualParent.generation == parent.generation);
        checkStatus(yorengineSceneSetParent(scene, parent, child), YORENGINE_STATUS_CONFLICT);

        checkStatus(yorengineSceneSetProperty(scene, child, "source", "c-api"), YORENGINE_STATUS_OK);
        uint32_t propertyLength = 0;
        checkStatus(yorengineSceneGetProperty(scene, child, "source", nullptr, 0, &propertyLength), YORENGINE_STATUS_OK);
        CHECK(propertyLength == 5);
        char property[6]{};
        checkStatus(yorengineSceneGetProperty(scene, child, "source", property, sizeof(property), &propertyLength), YORENGINE_STATUS_OK);
        CHECK(std::string(property) == "c-api");
        char tooSmall[5]{};
        checkStatus(yorengineSceneGetProperty(scene, child, "source", tooSmall, sizeof(tooSmall), &propertyLength), YORENGINE_STATUS_BUFFER_TOO_SMALL);

        checkStatus(yorengineSceneAddMesh(scene, child), YORENGINE_STATUS_OK);
        const YorEngineMeshVertex vertex{{1.0f, 2.0f, 3.0f}, 1.0f, 0.5f, 0.25f, 1.0f, 0.0f, 1.0f};
        checkStatus(yorengineSceneSetMeshVertices(scene, child, &vertex, 1), YORENGINE_STATUS_OK);
        YorEngineMeshVertex vertices[1]{};
        uint32_t vertexCount = 0;
        checkStatus(yorengineSceneGetMeshVertices(scene, child, vertices, 1, &vertexCount), YORENGINE_STATUS_OK);
        CHECK(vertexCount == 1);
        checkNear(3.0f, vertices[0].position.z);

        checkStatus(yorengineSceneAddCamera(scene, parent), YORENGINE_STATUS_OK);
        YorEngineCameraState camera{70.0f, 16.0f / 9.0f, 0.1f, 1024.0f};
        checkStatus(yorengineSceneSetCamera(scene, parent, &camera), YORENGINE_STATUS_OK);
        camera.farPlane = 0.0f;
        checkStatus(yorengineSceneSetCamera(scene, parent, &camera), YORENGINE_STATUS_INVALID_ARGUMENT);
        camera.nearPlane = 600.0f;
        camera.farPlane = 700.0f;
        checkStatus(yorengineSceneSetCamera(scene, parent, &camera), YORENGINE_STATUS_OK);
        camera.nearPlane = 0.1f;
        camera.farPlane = 0.2f;
        checkStatus(yorengineSceneSetCamera(scene, parent, &camera), YORENGINE_STATUS_OK);

        checkStatus(yorengineSceneAddLight(scene, parent, YORENGINE_LIGHT_SPOT), YORENGINE_STATUS_OK);
        YorEngineLightState light{YORENGINE_LIGHT_SPOT, {0.8f, 0.9f, 1.0f}, 2.0f, 32.0f, 15.0f, 45.0f};
        checkStatus(yorengineSceneSetLight(scene, parent, &light), YORENGINE_STATUS_OK);
        YorEngineLightState actualLight{};
        checkStatus(yorengineSceneGetLight(scene, parent, &actualLight), YORENGINE_STATUS_OK);
        CHECK(actualLight.kind == YORENGINE_LIGHT_SPOT);
        checkNear(2.0f, actualLight.intensity);

        checkStatus(yorengineSceneUpdate(scene, 1.0 / 60.0), YORENGINE_STATUS_OK);
        checkStatus(yorengineSceneDestroyEntity(scene, parent), YORENGINE_STATUS_OK);
        CHECK(yorengineSceneIsAlive(scene, parent) == 0);
        CHECK(yorengineSceneIsAlive(scene, child) == 0);
        checkStatus(yorengineSceneDestroyEntity(scene, child), YORENGINE_STATUS_INVALID_ENTITY);
        yorengineSceneDestroy(scene);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}

#undef CHECK
