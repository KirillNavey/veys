#include "core/Application.hpp"

#include <chrono>
#include <thread>

namespace veys {

Application::Application()
    : renderer_{}, jobs_{}, world_{jobs_} {
    world_.updateStreaming(playerX_, playerZ_, 2);
    world_.pollGeneration();
}

void Application::run() {
    using clock = std::chrono::high_resolution_clock;
    auto last = clock::now();

    for (int frame = 0; frame < 300 && running_; ++frame) {
        auto now = clock::now();
        const float dt = std::chrono::duration<float>(now - last).count();
        last = now;

        tick(dt);
        renderer_.drawFrame(world_);

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

void Application::tick(float deltaSeconds) {
    playerX_ += deltaSeconds * 8.0F;
    playerZ_ += deltaSeconds * 5.0F;

    world_.updateStreaming(playerX_, playerZ_, 2);
    world_.pollGeneration();
}

} // namespace veys
