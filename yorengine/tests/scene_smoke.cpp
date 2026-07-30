#include "yorengine/scene.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>

#define CHECK(expression) \
    do { \
        if (!(expression)) throw std::runtime_error("check failed: " #expression); \
    } while (false)

namespace {

void assertNear(float expected, float actual) {
    CHECK(std::fabs(expected - actual) < 0.0001f);
}

class CountingComponent final : public yorengine::Component {
public:
    void update(yorengine::Scene&, yorengine::EntityId, double) override { ++updates; }

    int updates = 0;
};

class SelfRemovingComponent final : public yorengine::Component {
public:
    explicit SelfRemovingComponent(int& updates) : updates_(updates) {}

    void update(yorengine::Scene& scene, yorengine::EntityId entity, double) override {
        CHECK(scene.removeComponent<SelfRemovingComponent>(entity));
        ++updates_;
    }

private:
    int& updates_;
};

} // namespace

int main() {
    try {
        using namespace yorengine;

        Scene scene;
        const EntityId parent = scene.createEntity();
        const EntityId child = scene.createEntity();
        CHECK(scene.setParent(child, parent));
        CHECK(!scene.setParent(parent, child));

        Transform parentTransform;
        parentTransform.position = {10.0f, 2.0f, -3.0f};
        CHECK(scene.setTransform(parent, parentTransform));
        Transform childTransform;
        childTransform.position = {1.0f, 0.0f, 0.0f};
        childTransform.scale = {2.0f, 2.0f, 2.0f};
        CHECK(scene.setTransform(child, childTransform));
        const Vec3 worldPoint = scene.worldMatrix(child).transformPoint({1.0f, 0.0f, 0.0f});
        assertNear(13.0f, worldPoint.x);
        assertNear(2.0f, worldPoint.y);
        assertNear(-3.0f, worldPoint.z);

        const std::uint64_t version = scene.version();
        scene.setProperty(child, "source", "test");
        CHECK(scene.version() > version);
        CHECK(scene.property(child, "source").value() == "test");

        auto& counter = scene.emplaceComponent<CountingComponent>(child);
        int selfRemovingUpdates = 0;
        scene.emplaceComponent<SelfRemovingComponent>(child, selfRemovingUpdates);
        scene.update(0.016);
        CHECK(counter.updates == 1);
        CHECK(selfRemovingUpdates == 1);
        CHECK(scene.component<SelfRemovingComponent>(child) == nullptr);

        auto& camera = scene.emplaceComponent<CameraComponent>(parent);
        camera.setAspectRatio(4.0f / 3.0f);
        camera.setNearPlane(0.1f);
        camera.setFarPlane(1024.0f);
        assertNear(1024.0f, camera.farPlane());
        bool invalidCameraValue = false;
        try {
            camera.setFovYDegrees(180.0f);
        } catch (const std::invalid_argument&) {
            invalidCameraValue = true;
        }
        CHECK(invalidCameraValue);

        auto& light = scene.emplaceComponent<LightComponent>(parent, LightComponent::Kind::Spot);
        light.setColor({0.8f, 0.9f, 1.0f});
        light.setIntensity(2.0f);
        light.setRange(32.0f);
        light.setCone(15.0f, 45.0f);
        CHECK(light.kind() == LightComponent::Kind::Spot);
        assertNear(2.0f, light.intensity());

        MeshComponent& mesh = scene.emplaceComponent<MeshComponent>(child);
        mesh.setVertices({{{0.0f, 0.0f, 0.0f}, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f}});
        CHECK(mesh.vertices().size() == 1);

        CHECK(scene.destroyEntity(parent));
        CHECK(!scene.isAlive(parent));
        CHECK(!scene.isAlive(child));
        const EntityId replacement = scene.createEntity();
        CHECK(replacement.index == parent.index);
        CHECK(replacement != parent);
        CHECK(scene.clearParent(replacement));
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}

#undef CHECK
