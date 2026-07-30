#include "yorengine/scene.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace yorengine {

namespace {

void requireFinite(float value, const char* name) {
    if (!std::isfinite(value)) throw std::invalid_argument(std::string(name) + " must be finite");
}

void requireFiniteNonNegative(float value, const char* name) {
    requireFinite(value, name);
    if (value < 0.0f) throw std::invalid_argument(std::string(name) + " must not be negative");
}

bool finiteTransform(const Transform& transform) noexcept {
    return std::isfinite(transform.position.x) && std::isfinite(transform.position.y) && std::isfinite(transform.position.z) &&
           std::isfinite(transform.rotation.x) && std::isfinite(transform.rotation.y) &&
           std::isfinite(transform.rotation.z) && std::isfinite(transform.rotation.w) &&
           std::isfinite(transform.scale.x) && std::isfinite(transform.scale.y) && std::isfinite(transform.scale.z);
}

} // namespace

void Component::attach(Scene& scene, EntityId entity) {
    ownerScene_ = &scene;
    ownerEntity_ = entity;
    try {
        onAttach(scene, entity);
    } catch (...) {
        ownerScene_ = nullptr;
        ownerEntity_ = {};
        throw;
    }
}

void Component::detach(Scene& scene, EntityId entity) {
    onDetach(scene, entity);
    ownerScene_ = nullptr;
    ownerEntity_ = {};
}

void Component::markChanged() noexcept {
    if (ownerScene_) ownerScene_->markChanged();
}

MeshComponent::MeshComponent(std::vector<MeshVertex> vertices) : vertices_(std::move(vertices)) {}

void MeshComponent::setVertices(std::vector<MeshVertex> vertices) {
    vertices_ = std::move(vertices);
    markChanged();
}

void CameraComponent::setFovYDegrees(float value) {
    requireFinite(value, "Camera fovYDegrees");
    if (value <= 0.0f || value >= 180.0f) {
        throw std::invalid_argument("Camera fovYDegrees must be between 0 and 180");
    }
    fovYDegrees_ = value;
    markChanged();
}

void CameraComponent::setAspectRatio(float value) {
    requireFinite(value, "Camera aspectRatio");
    if (value <= 0.0f) throw std::invalid_argument("Camera aspectRatio must be positive");
    aspectRatio_ = value;
    markChanged();
}

void CameraComponent::setNearPlane(float value) {
    requireFinite(value, "Camera nearPlane");
    if (value <= 0.0f || value >= farPlane_) {
        throw std::invalid_argument("Camera nearPlane must be positive and less than farPlane");
    }
    nearPlane_ = value;
    markChanged();
}

void CameraComponent::setFarPlane(float value) {
    requireFinite(value, "Camera farPlane");
    if (value <= nearPlane_) throw std::invalid_argument("Camera farPlane must be greater than nearPlane");
    farPlane_ = value;
    markChanged();
}

void LightComponent::setColor(Vec3 value) {
    requireFiniteNonNegative(value.x, "Light red");
    requireFiniteNonNegative(value.y, "Light green");
    requireFiniteNonNegative(value.z, "Light blue");
    color_ = value;
    markChanged();
}

void LightComponent::setIntensity(float value) {
    requireFiniteNonNegative(value, "Light intensity");
    intensity_ = value;
    markChanged();
}

void LightComponent::setRange(float value) {
    requireFinite(value, "Light range");
    if (value <= 0.0f) throw std::invalid_argument("Light range must be positive");
    range_ = value;
    markChanged();
}

void LightComponent::setCone(float innerDegrees, float outerDegrees) {
    requireFiniteNonNegative(innerDegrees, "Light inner cone");
    requireFiniteNonNegative(outerDegrees, "Light outer cone");
    if (outerDegrees < innerDegrees || outerDegrees > 180.0f) {
        throw std::invalid_argument("Light cone must satisfy 0 <= inner <= outer <= 180");
    }
    innerConeDegrees_ = innerDegrees;
    outerConeDegrees_ = outerDegrees;
    markChanged();
}

EntityId Scene::createEntity() {
    std::uint32_t index;
    if (freeIndices_.empty()) {
        index = static_cast<std::uint32_t>(slots_.size());
        if (index == EntityId::InvalidIndex) throw std::overflow_error("Scene entity limit reached");
        slots_.emplace_back();
    } else {
        index = freeIndices_.back();
        freeIndices_.pop_back();
    }

    EntitySlot& slot = slots_[index];
    slot.alive = true;
    slot.active = true;
    slot.parent = {};
    slot.children.clear();
    slot.transform = {};
    slot.cachedWorldMatrix = {};
    slot.worldTransformVersion = 0;
    slot.components.clear();
    slot.properties.clear();
    markChanged();
    return {index, slot.generation};
}

bool Scene::destroyEntity(EntityId entity) {
    if (!isAlive(entity)) return false;

    std::vector<EntityId> order;
    std::vector<EntityId> pending{entity};
    while (!pending.empty()) {
        const EntityId current = pending.back();
        pending.pop_back();
        if (!isAlive(current)) continue;
        order.push_back(current);
        const auto childList = slots_[current.index].children;
        pending.insert(pending.end(), childList.begin(), childList.end());
    }

    const EntityId rootParent = slots_[entity.index].parent;
    if (rootParent.valid() && isAlive(rootParent)) unlinkChild(rootParent, entity);

    for (auto it = order.rbegin(); it != order.rend(); ++it) {
        EntitySlot& slot = slots_[it->index];
        auto components = std::move(slot.components);
        for (auto& component : components) component->detach(*this, *it);
        slot.components.clear();
        slot.properties.clear();
        slot.children.clear();
        slot.parent = {};
        slot.active = false;
        slot.alive = false;
        ++slot.generation;
        if (slot.generation == 0) slot.generation = 1;
        freeIndices_.push_back(it->index);
    }

    markChanged();
    return true;
}

bool Scene::isAlive(EntityId entity) const noexcept {
    return entity.valid() && entity.index < slots_.size() && slots_[entity.index].alive &&
           slots_[entity.index].generation == entity.generation;
}

std::vector<EntityId> Scene::entities() const {
    std::vector<EntityId> result;
    result.reserve(slots_.size() - freeIndices_.size());
    for (std::uint32_t index = 0; index < slots_.size(); ++index) {
        const EntitySlot& slot = slots_[index];
        if (slot.alive) result.push_back({index, slot.generation});
    }
    return result;
}

bool Scene::setParent(EntityId child, EntityId newParent) {
    if (!isAlive(child) || !isAlive(newParent) || child == newParent) return false;
    if (isAncestor(newParent, child)) return false;
    EntitySlot& childSlot = slots_[child.index];
    if (childSlot.parent == newParent) return true;
    if (childSlot.parent.valid() && isAlive(childSlot.parent)) unlinkChild(childSlot.parent, child);
    childSlot.parent = newParent;
    slots_[newParent.index].children.push_back(child);
    markTransformChanged();
    markChanged();
    return true;
}

bool Scene::clearParent(EntityId child) {
    if (!isAlive(child)) return false;
    EntitySlot& childSlot = slots_[child.index];
    if (!childSlot.parent.valid()) return true;
    if (isAlive(childSlot.parent)) unlinkChild(childSlot.parent, child);
    childSlot.parent = {};
    markTransformChanged();
    markChanged();
    return true;
}

EntityId Scene::parent(EntityId child) const {
    return requireSlot(child).parent;
}

std::vector<EntityId> Scene::children(EntityId parentEntity) const {
    return requireSlot(parentEntity).children;
}

Transform Scene::transform(EntityId entity) const {
    return requireSlot(entity).transform;
}

bool Scene::setTransform(EntityId entity, Transform value) {
    if (!isAlive(entity) || !finiteTransform(value)) return false;
    slots_[entity.index].transform = value;
    markTransformChanged();
    markChanged();
    return true;
}

Mat4 Scene::worldMatrix(EntityId entity) const {
    const EntitySlot& slot = requireSlot(entity);
    if (slot.worldTransformVersion == transformVersion_) return slot.cachedWorldMatrix;

    const EntityId parentEntity = slot.parent;
    const Mat4 local = slot.transform.localMatrix();
    slot.cachedWorldMatrix = parentEntity.valid() && isAlive(parentEntity)
        ? worldMatrix(parentEntity) * local
        : local;
    slot.worldTransformVersion = transformVersion_;
    return slot.cachedWorldMatrix;
}

bool Scene::active(EntityId entity) const {
    return requireSlot(entity).active;
}

bool Scene::setActive(EntityId entity, bool value) {
    if (!isAlive(entity)) return false;
    EntitySlot& slot = slots_[entity.index];
    if (slot.active == value) return true;
    slot.active = value;
    markChanged();
    return true;
}

void Scene::update(double deltaSeconds) {
    if (!std::isfinite(deltaSeconds) || deltaSeconds < 0.0) {
        throw std::invalid_argument("Scene deltaSeconds must be finite and non-negative");
    }

    for (const EntityId entity : entities()) {
        if (!isAlive(entity) || !slots_[entity.index].active) continue;
        const auto snapshot = slots_[entity.index].components;
        for (const auto& component : snapshot) {
            if (!isAlive(entity) || !slots_[entity.index].active) break;
            if (hasComponent(entity, component)) component->update(*this, entity, deltaSeconds);
        }
    }
}

void Scene::setProperty(EntityId entity, std::string key, std::string value) {
    if (key.empty()) throw std::invalid_argument("Scene property key must not be empty");
    EntitySlot& slot = requireSlot(entity);
    const auto existing = slot.properties.find(key);
    if (existing != slot.properties.end() && existing->second == value) return;
    slot.properties.insert_or_assign(std::move(key), std::move(value));
    markChanged();
}

std::optional<std::string> Scene::property(EntityId entity, std::string_view key) const {
    const EntitySlot& slot = requireSlot(entity);
    const auto it = slot.properties.find(std::string(key));
    if (it == slot.properties.end()) return std::nullopt;
    return it->second;
}

void Scene::markChanged() noexcept {
    if (version_ != std::numeric_limits<std::uint64_t>::max()) ++version_;
}

void Scene::markTransformChanged() noexcept {
    if (transformVersion_ != std::numeric_limits<std::uint64_t>::max()) {
        ++transformVersion_;
        return;
    }

    transformVersion_ = 1;
    for (auto& slot : slots_) slot.worldTransformVersion = 0;
}

Scene::EntitySlot& Scene::requireSlot(EntityId entity) {
    if (!isAlive(entity)) throw std::out_of_range("Invalid or stale EntityId");
    return slots_[entity.index];
}

const Scene::EntitySlot& Scene::requireSlot(EntityId entity) const {
    if (!isAlive(entity)) throw std::out_of_range("Invalid or stale EntityId");
    return slots_[entity.index];
}

bool Scene::hasComponent(EntityId entity, const std::shared_ptr<Component>& component) const noexcept {
    if (!isAlive(entity)) return false;
    const auto& values = slots_[entity.index].components;
    return std::find(values.begin(), values.end(), component) != values.end();
}

bool Scene::isAncestor(EntityId candidate, EntityId possibleAncestor) const noexcept {
    for (EntityId current = candidate; current.valid() && isAlive(current); current = slots_[current.index].parent) {
        if (current == possibleAncestor) return true;
    }
    return false;
}

void Scene::unlinkChild(EntityId parentEntity, EntityId child) {
    auto& values = slots_[parentEntity.index].children;
    values.erase(std::remove(values.begin(), values.end(), child), values.end());
}

} // namespace yorengine
