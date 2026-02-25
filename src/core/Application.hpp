#pragma once

#include "core/JobSystem.hpp"
#include "renderer/Renderer.hpp"
#include "world/World.hpp"

namespace veys {

class Application {
public:
    Application();
    void run();

private:
    void tick(float deltaSeconds);

    renderer::Renderer renderer_;
    JobSystem jobs_;
    world::World world_;
    bool running_{true};

    float playerX_{0.0F};
    float playerZ_{0.0F};
};

} // namespace veys
