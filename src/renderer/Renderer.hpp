#pragma once

#include "world/World.hpp"

#include <cstddef>

namespace veys::renderer {

class Renderer {
public:
    Renderer();
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void drawFrame(const world::World& world);

private:
    void initializeVulkan();

    std::size_t frameIndex_{0};
    bool vulkanAvailable_{false};
};

} // namespace veys::renderer
