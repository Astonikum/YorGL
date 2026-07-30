#include "yorengine/render_snapshot.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>

#define CHECK(expression) \
    do { \
        if (!(expression)) throw std::runtime_error("check failed: " #expression); \
    } while (false)

namespace {

const yorengine::RenderEntity& findEntity(const yorengine::RenderSnapshot& snapshot, yorengine::EntityId entity) {
    for (const auto& value : snapshot.entities()) {
        if (value.entity == entity) return value;
    }
    throw std::runtime_error("render entity not found");
}

void checkNear(float expected, float actual) {
    CHECK(std::fabs(expected - actual) < 0.0001f);
}

} // namespace

int main() {
    try {
        yorengine::Scene scene;
        const yorengine::EntityId parent = scene.createEntity();
        const yorengine::EntityId child = scene.createEntity();
        const yorengine::EntityId inactive = scene.createEntity();
        CHECK(scene.setParent(child, parent));

        yorengine::Transform parentTransform;
        parentTransform.position = {10.0f, 2.0f, -3.0f};
        CHECK(scene.setTransform(parent, parentTransform));
        yorengine::Transform childTransform;
        childTransform.position = {1.0f, 0.0f, 0.0f};
        CHECK(scene.setTransform(child, childTransform));
        scene.emplaceComponent<yorengine::MeshComponent>(child).setVertices({{{1.0f, 0.0f, 0.0f}}});
        auto& camera = scene.emplaceComponent<yorengine::CameraComponent>(parent);
        camera.setFarPlane(1024.0f);
        auto& light = scene.emplaceComponent<yorengine::LightComponent>(parent, yorengine::LightComponent::Kind::Spot);
        light.setIntensity(2.0f);
        scene.emplaceComponent<yorengine::MeshComponent>(inactive);
        CHECK(scene.setActive(inactive, false));

        const yorengine::RenderSnapshot snapshot = scene.captureRenderSnapshot();
        CHECK(snapshot.sourceVersion() == scene.version());
        CHECK(snapshot.entities().size() == 2);
        const auto& parentRender = findEntity(snapshot, parent);
        const auto& childRender = findEntity(snapshot, child);
        CHECK(parentRender.camera.has_value());
        CHECK(parentRender.light.has_value());
        checkNear(1024.0f, parentRender.camera->farPlane);
        checkNear(2.0f, parentRender.light->intensity);
        CHECK(childRender.meshVertices.size() == 1);
        checkNear(11.0f, childRender.worldMatrix.at(0, 3));

        parentTransform.position.x = 20.0f;
        CHECK(scene.setTransform(parent, parentTransform));
        CHECK(scene.destroyEntity(child));
        CHECK(snapshot.entities().size() == 2);
        checkNear(11.0f, findEntity(snapshot, child).worldMatrix.at(0, 3));

        const yorengine::RenderSnapshot updated = scene.captureRenderSnapshot();
        CHECK(updated.entities().size() == 1);
        checkNear(20.0f, findEntity(updated, parent).worldMatrix.at(0, 3));
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}

#undef CHECK
