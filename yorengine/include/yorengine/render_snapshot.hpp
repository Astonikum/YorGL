#pragma once

#include "scene.hpp"

#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

namespace yorengine {

struct RenderCamera {
    float fovYDegrees = 70.0f;
    float aspectRatio = 16.0f / 9.0f;
    float nearPlane = 0.05f;
    float farPlane = 512.0f;
};

struct RenderLight {
    LightComponent::Kind kind = LightComponent::Kind::Directional;
    Vec3 color{1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
    float range = 10.0f;
    float innerConeDegrees = 15.0f;
    float outerConeDegrees = 45.0f;
};

struct RenderEntity {
    EntityId entity{};
    Mat4 worldMatrix{};
    std::vector<MeshVertex> meshVertices;
    std::optional<RenderCamera> camera;
    std::optional<RenderLight> light;
};

class RenderSnapshot {
public:
    std::uint64_t sourceVersion() const noexcept { return sourceVersion_; }
    const std::vector<RenderEntity>& entities() const noexcept { return entities_; }

private:
    friend class Scene;

    RenderSnapshot(std::uint64_t sourceVersion, std::vector<RenderEntity> entities)
        : sourceVersion_(sourceVersion), entities_(std::move(entities)) {}

    std::uint64_t sourceVersion_;
    std::vector<RenderEntity> entities_;
};

} // namespace yorengine
