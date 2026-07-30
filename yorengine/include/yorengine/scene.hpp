#pragma once

#include "math.hpp"

#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace yorengine {

class Scene;
class RenderSnapshot;

struct EntityId {
    static constexpr std::uint32_t InvalidIndex = std::numeric_limits<std::uint32_t>::max();

    std::uint32_t index = InvalidIndex;
    std::uint32_t generation = 0;

    constexpr bool valid() const noexcept { return index != InvalidIndex && generation != 0; }
    friend constexpr bool operator==(EntityId left, EntityId right) noexcept = default;
};

struct MeshVertex {
    Vec3 position{};
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;
    float u = 0.0f;
    float v = 0.0f;
};

class Component {
public:
    virtual ~Component() = default;

    virtual void onAttach(Scene&, EntityId) {}
    virtual void onDetach(Scene&, EntityId) {}
    virtual void update(Scene&, EntityId, double) {}

protected:
    void markChanged() noexcept;
    Scene* ownerScene() const noexcept { return ownerScene_; }
    EntityId ownerEntity() const noexcept { return ownerEntity_; }

private:
    friend class Scene;

    void attach(Scene& scene, EntityId entity);
    void detach(Scene& scene, EntityId entity);

    Scene* ownerScene_ = nullptr;
    EntityId ownerEntity_{};
};

class MeshComponent final : public Component {
public:
    explicit MeshComponent(std::vector<MeshVertex> vertices = {});

    const std::vector<MeshVertex>& vertices() const noexcept { return vertices_; }
    void setVertices(std::vector<MeshVertex> vertices);

private:
    std::vector<MeshVertex> vertices_;
};

class CameraComponent final : public Component {
public:
    float fovYDegrees() const noexcept { return fovYDegrees_; }
    float aspectRatio() const noexcept { return aspectRatio_; }
    float nearPlane() const noexcept { return nearPlane_; }
    float farPlane() const noexcept { return farPlane_; }

    void setFovYDegrees(float value);
    void setAspectRatio(float value);
    void setNearPlane(float value);
    void setFarPlane(float value);

private:
    float fovYDegrees_ = 70.0f;
    float aspectRatio_ = 16.0f / 9.0f;
    float nearPlane_ = 0.05f;
    float farPlane_ = 512.0f;
};

class LightComponent final : public Component {
public:
    enum class Kind {
        Directional,
        Point,
        Spot,
    };

    explicit LightComponent(Kind kind = Kind::Directional) : kind_(kind) {}

    Kind kind() const noexcept { return kind_; }
    Vec3 color() const noexcept { return color_; }
    float intensity() const noexcept { return intensity_; }
    float range() const noexcept { return range_; }
    float innerConeDegrees() const noexcept { return innerConeDegrees_; }
    float outerConeDegrees() const noexcept { return outerConeDegrees_; }

    void setColor(Vec3 value);
    void setIntensity(float value);
    void setRange(float value);
    void setCone(float innerDegrees, float outerDegrees);

private:
    Kind kind_ = Kind::Directional;
    Vec3 color_{1.0f, 1.0f, 1.0f};
    float intensity_ = 1.0f;
    float range_ = 10.0f;
    float innerConeDegrees_ = 15.0f;
    float outerConeDegrees_ = 45.0f;
};

class Scene {
public:
    Scene() = default;
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    EntityId createEntity();
    bool destroyEntity(EntityId entity);
    bool isAlive(EntityId entity) const noexcept;
    std::vector<EntityId> entities() const;

    bool setParent(EntityId child, EntityId parent);
    bool clearParent(EntityId child);
    EntityId parent(EntityId child) const;
    std::vector<EntityId> children(EntityId parent) const;

    Transform transform(EntityId entity) const;
    bool setTransform(EntityId entity, Transform transform);
    Mat4 worldMatrix(EntityId entity) const;

    bool active(EntityId entity) const;
    bool setActive(EntityId entity, bool active);

    void update(double deltaSeconds);
    RenderSnapshot captureRenderSnapshot() const;

    std::uint64_t version() const noexcept { return version_; }
    void setProperty(EntityId entity, std::string key, std::string value);
    std::optional<std::string> property(EntityId entity, std::string_view key) const;

    template <typename T, typename... Args>
    T& emplaceComponent(EntityId entity, Args&&... args);

    template <typename T>
    T* component(EntityId entity) noexcept;

    template <typename T>
    const T* component(EntityId entity) const noexcept;

    template <typename T>
    bool removeComponent(EntityId entity);

    void markChanged() noexcept;

private:
    struct EntitySlot {
        std::uint32_t generation = 1;
        bool alive = false;
        bool active = true;
        EntityId parent{};
        std::vector<EntityId> children;
        Transform transform{};
        mutable Mat4 cachedWorldMatrix{};
        mutable std::uint64_t worldTransformVersion = 0;
        std::vector<std::shared_ptr<Component>> components;
        std::unordered_map<std::string, std::string> properties;
    };

    EntitySlot& requireSlot(EntityId entity);
    const EntitySlot& requireSlot(EntityId entity) const;
    bool hasComponent(EntityId entity, const std::shared_ptr<Component>& component) const noexcept;
    bool isAncestor(EntityId candidate, EntityId possibleAncestor) const noexcept;
    void unlinkChild(EntityId parent, EntityId child);
    void markTransformChanged() noexcept;

    std::vector<EntitySlot> slots_;
    std::vector<std::uint32_t> freeIndices_;
    std::uint64_t version_ = 1;
    std::uint64_t transformVersion_ = 1;
};

template <typename T, typename... Args>
T& Scene::emplaceComponent(EntityId entity, Args&&... args) {
    static_assert(std::is_base_of_v<Component, T>, "T must derive from yorengine::Component");
    EntitySlot& slot = requireSlot(entity);
    if (component<T>(entity) != nullptr) {
        throw std::logic_error("Entity already has this component type");
    }
    auto value = std::make_shared<T>(std::forward<Args>(args)...);
    slot.components.push_back(value);
    try {
        value->attach(*this, entity);
    } catch (...) {
        slot.components.pop_back();
        throw;
    }
    markChanged();
    return *value;
}

template <typename T>
T* Scene::component(EntityId entity) noexcept {
    static_assert(std::is_base_of_v<Component, T>, "T must derive from yorengine::Component");
    if (!isAlive(entity)) return nullptr;
    EntitySlot& slot = slots_[entity.index];
    for (const auto& value : slot.components) {
        if (auto typed = std::dynamic_pointer_cast<T>(value)) return typed.get();
    }
    return nullptr;
}

template <typename T>
const T* Scene::component(EntityId entity) const noexcept {
    static_assert(std::is_base_of_v<Component, T>, "T must derive from yorengine::Component");
    if (!isAlive(entity)) return nullptr;
    const EntitySlot& slot = slots_[entity.index];
    for (const auto& value : slot.components) {
        if (auto typed = std::dynamic_pointer_cast<const T>(value)) return typed.get();
    }
    return nullptr;
}

template <typename T>
bool Scene::removeComponent(EntityId entity) {
    static_assert(std::is_base_of_v<Component, T>, "T must derive from yorengine::Component");
    if (!isAlive(entity)) return false;
    EntitySlot& slot = slots_[entity.index];
    for (auto it = slot.components.begin(); it != slot.components.end(); ++it) {
        if (std::dynamic_pointer_cast<T>(*it)) {
            (*it)->detach(*this, entity);
            slot.components.erase(it);
            markChanged();
            return true;
        }
    }
    return false;
}

} // namespace yorengine
