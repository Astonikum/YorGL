#pragma once

#include "scene.hpp"

#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace yorengine {

struct SystemId {
    static constexpr std::uint32_t InvalidIndex = EntityId::InvalidIndex;

    std::uint32_t index = InvalidIndex;
    std::uint32_t generation = 0;

    constexpr bool valid() const noexcept { return index != InvalidIndex && generation != 0; }
    friend constexpr bool operator==(SystemId left, SystemId right) noexcept = default;
};

class Runtime;

class System {
public:
    virtual ~System() = default;

    virtual void update(Runtime&, Scene&, SystemId, double deltaSeconds) = 0;
};

class Runtime {
public:
    struct Config {
        double fixedDeltaSeconds = 1.0 / 60.0;
        std::uint32_t maxFixedStepsPerAdvance = 8;
    };

    explicit Runtime(Config config = {});
    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;

    Scene& scene() noexcept { return scene_; }
    const Scene& scene() const noexcept { return scene_; }

    SystemId addSystem(std::unique_ptr<System> system);
    template <typename T, typename... Args>
    SystemId addSystem(Args&&... args);
    bool removeSystem(SystemId system);
    bool isSystemAlive(SystemId system) const noexcept;
    std::vector<SystemId> systems() const;

    void start() noexcept;
    void stop() noexcept;
    bool running() const noexcept { return running_; }
    void setPaused(bool paused) noexcept { paused_ = paused; }
    bool paused() const noexcept { return paused_; }

    std::uint32_t advance(double elapsedSeconds);
    void singleStep();

    double fixedDeltaSeconds() const noexcept { return config_.fixedDeltaSeconds; }
    double interpolationAlpha() const noexcept;
    std::uint64_t simulationTick() const noexcept { return simulationTick_; }

private:
    struct SystemSlot {
        std::uint32_t generation = 1;
        bool alive = false;
        bool pendingRemoval = false;
        std::unique_ptr<System> value;
    };

    void executeFixedStep();
    void flushRemovedSystems() noexcept;

    Config config_;
    Scene scene_;
    std::vector<SystemSlot> systems_;
    std::vector<std::uint32_t> freeIndices_;
    double accumulator_ = 0.0;
    std::uint64_t simulationTick_ = 0;
    bool running_ = false;
    bool paused_ = false;
    bool updating_ = false;
};

template <typename T, typename... Args>
SystemId Runtime::addSystem(Args&&... args) {
    static_assert(std::is_base_of_v<System, T>, "T must derive from yorengine::System");
    return addSystem(std::make_unique<T>(std::forward<Args>(args)...));
}

} // namespace yorengine
