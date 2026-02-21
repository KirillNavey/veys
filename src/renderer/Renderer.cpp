#include "renderer/Renderer.hpp"

#include <iostream>

#if __has_include(<vulkan/vulkan.h>)
#include <vulkan/vulkan.h>
#define VEYS_HAS_VULKAN 1
#else
#define VEYS_HAS_VULKAN 0
#endif

namespace veys::renderer {

Renderer::Renderer() {
    initializeVulkan();
}

Renderer::~Renderer() = default;

void Renderer::initializeVulkan() {
#if VEYS_HAS_VULKAN
    vulkanAvailable_ = true;
    std::cout << "Vulkan headers detected. Ready for full GPU backend integration.\n";
#else
    vulkanAvailable_ = false;
    std::cout << "Vulkan SDK not found at compile time. Running CPU-only stub renderer.\n";
#endif
}

void Renderer::drawFrame(const world::World& world) {
    ++frameIndex_;

    const auto chunks = world.loadedChunks();

    std::size_t visibleVoxels = 0;
    for (const world::Chunk* chunk : chunks) {
        visibleVoxels += chunk->solidVoxelCount();
    }

    if (frameIndex_ % 60 == 0) {
        std::cout << "Frame " << frameIndex_
                  << " | backend=" << (vulkanAvailable_ ? "Vulkan" : "Stub")
                  << " | loaded chunks=" << chunks.size()
                  << " | visible voxels=" << visibleVoxels << '\n';
    }
}

} // namespace veys::renderer
