#include "yorengine/runtime.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

namespace yorengine {

Runtime::Runtime(Config config) : config_(config) {
    if (!std::isfinite(config_.fixedDeltaSeconds) || config_.fixedDeltaSeconds <= 0.0) {
        throw std::invalid_argument("Runtime fixedDeltaSeconds must be finite and positive");
    }
    if (config_.maxFixedStepsPerAdvance == 0) {
        throw std::invalid_argument("Runtime maxFixedStepsPerAdvance must be positive");
    }
}

SystemId Runtime::addSystem(std::unique_ptr<System> system) {
    if (!system) throw std::invalid_argument("Runtime system must not be null");

    std::uint32_t index;
    if (freeIndices_.empty()) {
        index = static_cast<std::uint32_t>(systems_.size());
        if (index == SystemId::InvalidIndex) throw std::overflow_error("Runtime system limit reached");
        systems_.emplace_back();
    } else {
        index = freeIndices_.back();
        freeIndices_.pop_back();
    }

    SystemSlot& slot = systems_[index];
    slot.alive = true;
    slot.pendingRemoval = false;
    slot.value = std::move(system);
    return {index, slot.generation};
}

bool Runtime::removeSystem(SystemId system) {
    if (!isSystemAlive(system)) return false;

    SystemSlot& slot = systems_[system.index];
    slot.alive = false;
    slot.pendingRemoval = true;
    ++slot.generation;
    if (slot.generation == 0) slot.generation = 1;
    if (!updating_) flushRemovedSystems();
    return true;
}

bool Runtime::isSystemAlive(SystemId system) const noexcept {
    return system.valid() && system.index < systems_.size() && systems_[system.index].alive &&
           systems_[system.index].generation == system.generation;
}

std::vector<SystemId> Runtime::systems() const {
    std::vector<SystemId> result;
    result.reserve(systems_.size() - freeIndices_.size());
    for (std::uint32_t index = 0; index < systems_.size(); ++index) {
        const SystemSlot& slot = systems_[index];
        if (slot.alive) result.push_back({index, slot.generation});
    }
    return result;
}

void Runtime::start() noexcept {
    running_ = true;
    paused_ = false;
}

void Runtime::stop() noexcept {
    running_ = false;
    paused_ = false;
    accumulator_ = 0.0;
}

std::uint32_t Runtime::advance(double elapsedSeconds) {
    if (!std::isfinite(elapsedSeconds) || elapsedSeconds < 0.0) {
        throw std::invalid_argument("Runtime elapsedSeconds must be finite and non-negative");
    }
    if (updating_) throw std::logic_error("Runtime cannot advance from a system update");
    if (!running_ || paused_) return 0;

    const double maximumAccumulator = config_.fixedDeltaSeconds * config_.maxFixedStepsPerAdvance;
    accumulator_ = std::min(accumulator_ + elapsedSeconds, maximumAccumulator);

    std::uint32_t steps = 0;
    while (steps < config_.maxFixedStepsPerAdvance && accumulator_ + 1.0e-12 >= config_.fixedDeltaSeconds) {
        executeFixedStep();
        accumulator_ -= config_.fixedDeltaSeconds;
        if (accumulator_ < 1.0e-12) accumulator_ = 0.0;
        ++steps;
    }
    return steps;
}

void Runtime::singleStep() {
    if (!running_) throw std::logic_error("Runtime must be running for singleStep");
    if (updating_) throw std::logic_error("Runtime cannot singleStep from a system update");
    executeFixedStep();
}

double Runtime::interpolationAlpha() const noexcept {
    return accumulator_ / config_.fixedDeltaSeconds;
}

void Runtime::executeFixedStep() {
    const auto snapshot = systems();
    updating_ = true;
    try {
        for (const SystemId system : snapshot) {
            if (!isSystemAlive(system)) continue;
            systems_[system.index].value->update(*this, scene_, system, config_.fixedDeltaSeconds);
        }
        scene_.update(config_.fixedDeltaSeconds);
        updating_ = false;
        flushRemovedSystems();
    } catch (...) {
        updating_ = false;
        flushRemovedSystems();
        throw;
    }
    ++simulationTick_;
}

void Runtime::flushRemovedSystems() noexcept {
    for (std::uint32_t index = 0; index < systems_.size(); ++index) {
        SystemSlot& slot = systems_[index];
        if (!slot.pendingRemoval) continue;
        slot.value.reset();
        slot.pendingRemoval = false;
        freeIndices_.push_back(index);
    }
}

} // namespace yorengine
