#include "yorengine/runtime.hpp"

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

#define CHECK(expression) \
    do { \
        if (!(expression)) throw std::runtime_error("check failed: " #expression); \
    } while (false)

namespace {

class CountingComponent final : public yorengine::Component {
public:
    explicit CountingComponent(int& updates) : updates_(updates) {}

    void update(yorengine::Scene&, yorengine::EntityId, double) override { ++updates_; }

private:
    int& updates_;
};

class OrderedSystem final : public yorengine::System {
public:
    OrderedSystem(int value, int& updates, std::vector<int>& order) : value_(value), updates_(updates), order_(order) {}

    void update(yorengine::Runtime&, yorengine::Scene&, yorengine::SystemId, double) override {
        ++updates_;
        order_.push_back(value_);
    }

private:
    int value_;
    int& updates_;
    std::vector<int>& order_;
};

class SelfRemovingSystem final : public yorengine::System {
public:
    explicit SelfRemovingSystem(int& updates) : updates_(updates) {}

    void update(yorengine::Runtime& runtime, yorengine::Scene&, yorengine::SystemId self, double) override {
        ++updates_;
        CHECK(runtime.removeSystem(self));
    }

private:
    int& updates_;
};

} // namespace

int main() {
    try {
        bool invalidConfig = false;
        try {
            yorengine::Runtime invalid({0.0, 4});
        } catch (const std::invalid_argument&) {
            invalidConfig = true;
        }
        CHECK(invalidConfig);

        yorengine::Runtime runtime({0.1, 4});
        const auto entity = runtime.scene().createEntity();
        int componentUpdates = 0;
        runtime.scene().emplaceComponent<CountingComponent>(entity, componentUpdates);

        int firstUpdates = 0;
        int secondUpdates = 0;
        std::vector<int> order;
        const auto first = runtime.addSystem<OrderedSystem>(1, firstUpdates, order);
        const auto second = runtime.addSystem<OrderedSystem>(2, secondUpdates, order);
        CHECK(runtime.isSystemAlive(first));
        CHECK(runtime.systems().size() == 2);

        runtime.start();
        CHECK(runtime.advance(0.25) == 2);
        CHECK(runtime.simulationTick() == 2);
        CHECK(runtime.interpolationAlpha() > 0.49 && runtime.interpolationAlpha() < 0.51);
        CHECK(firstUpdates == 2 && secondUpdates == 2 && componentUpdates == 2);
        CHECK((order == std::vector<int>{1, 2, 1, 2}));

        runtime.setPaused(true);
        CHECK(runtime.advance(1.0) == 0);
        CHECK(runtime.simulationTick() == 2);
        runtime.singleStep();
        CHECK(runtime.simulationTick() == 3);
        CHECK(firstUpdates == 3 && secondUpdates == 3 && componentUpdates == 3);

        runtime.setPaused(false);
        CHECK(runtime.advance(1.0) == 4);
        CHECK(runtime.simulationTick() == 7);
        CHECK(firstUpdates == 7 && secondUpdates == 7 && componentUpdates == 7);

        int selfRemovingUpdates = 0;
        const auto selfRemoving = runtime.addSystem<SelfRemovingSystem>(selfRemovingUpdates);
        CHECK(runtime.advance(0.1) == 1);
        CHECK(selfRemovingUpdates == 1);
        CHECK(!runtime.isSystemAlive(selfRemoving));
        const auto replacement = runtime.addSystem<OrderedSystem>(3, firstUpdates, order);
        CHECK(replacement.index == selfRemoving.index);
        CHECK(replacement != selfRemoving);

        runtime.stop();
        CHECK(!runtime.running());
        CHECK(runtime.advance(1.0) == 0);
        bool invalidStep = false;
        try {
            runtime.singleStep();
        } catch (const std::logic_error&) {
            invalidStep = true;
        }
        CHECK(invalidStep);

        bool invalidDelta = false;
        try {
            runtime.advance(std::numeric_limits<double>::quiet_NaN());
        } catch (const std::invalid_argument&) {
            invalidDelta = true;
        }
        CHECK(invalidDelta);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}

#undef CHECK
