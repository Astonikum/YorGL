#include "yorengine/render_snapshot.hpp"

#include <utility>

namespace yorengine {

RenderSnapshot Scene::captureRenderSnapshot() const {
    std::vector<RenderEntity> result;
    for (const EntityId entity : entities()) {
        if (!active(entity)) continue;

        RenderEntity renderEntity;
        renderEntity.entity = entity;
        renderEntity.worldMatrix = worldMatrix(entity);

        if (const auto* mesh = component<MeshComponent>(entity)) {
            renderEntity.meshVertices = mesh->vertices();
        }
        if (const auto* camera = component<CameraComponent>(entity)) {
            renderEntity.camera = RenderCamera{
                camera->fovYDegrees(),
                camera->aspectRatio(),
                camera->nearPlane(),
                camera->farPlane(),
            };
        }
        if (const auto* light = component<LightComponent>(entity)) {
            renderEntity.light = RenderLight{
                light->kind(),
                light->color(),
                light->intensity(),
                light->range(),
                light->innerConeDegrees(),
                light->outerConeDegrees(),
            };
        }

        if (!renderEntity.meshVertices.empty() || renderEntity.camera || renderEntity.light) {
            result.push_back(std::move(renderEntity));
        }
    }
    return RenderSnapshot{version_, std::move(result)};
}

} // namespace yorengine
