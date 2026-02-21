#include "core/JobSystem.hpp"
#include "world/ChunkStorage.hpp"
#include "world/World.hpp"

#include <filesystem>
#include <iostream>

namespace {

bool testChunkStorageRoundTrip() {
    const std::filesystem::path root{"test_cache/chunk_storage"};
    std::filesystem::remove_all(root);

    veys::world::ChunkStorage storage{root};
    veys::world::Chunk source;
    source.generateTerrain(12345u, 4, -7);

    const veys::world::ChunkCoord coord{4, -7};
    if (!storage.saveChunk(coord, source)) {
        std::cerr << "saveChunk failed\n";
        return false;
    }

    veys::world::Chunk loaded;
    if (!storage.loadChunk(coord, loaded)) {
        std::cerr << "loadChunk failed\n";
        return false;
    }

    const auto src = source.voxels();
    const auto dst = loaded.voxels();
    for (std::size_t i = 0; i < src.size(); ++i) {
        if (src[i].id != dst[i].id) {
            std::cerr << "voxel mismatch at " << i << '\n';
            return false;
        }
    }

    return true;
}

bool testWorldCacheHitAfterWarmup() {
    const std::filesystem::path root{"test_cache/world"};
    std::filesystem::remove_all(root);

    veys::JobSystem jobs;
    veys::world::World world{jobs, root};

    for (int i = 0; i < 40; ++i) {
        world.updateStreaming(0.0F, 0.0F, 2);
        world.pollGeneration();
    }

    if (world.loadedChunkCount() == 0 || world.generatedChunks() == 0) {
        std::cerr << "world warmup failed to generate chunks\n";
        return false;
    }

    veys::world::World reload{jobs, root};
    reload.updateStreaming(0.0F, 0.0F, 2);
    reload.pollGeneration();

    if (reload.cacheHits() == 0 || reload.loadedFromCacheChunks() == 0) {
        std::cerr << "expected cache hits after warmup\n";
        return false;
    }

    if (reload.loadedChunkCount() == 0) {
        std::cerr << "reload loadedChunkCount is zero\n";
        return false;
    }

    return true;
}

bool testWorldEvictionWhileMoving() {
    const std::filesystem::path root{"test_cache/world_move"};
    std::filesystem::remove_all(root);

    veys::JobSystem jobs;
    veys::world::World world{jobs, root};

    for (int step = 0; step < 18; ++step) {
        const float x = static_cast<float>(step * 32);
        world.updateStreaming(x, 0.0F, 2);

        for (int i = 0; i < 8; ++i) {
            world.pollGeneration();
        }
    }

    if (world.evictedChunks() == 0) {
        std::cerr << "expected eviction while moving through world\n";
        return false;
    }

    if (world.loadedChunkCount() > 40) {
        std::cerr << "loaded chunk count unexpectedly high\n";
        return false;
    }

    return true;
}

} // namespace

int main() {
    const bool okStorage = testChunkStorageRoundTrip();
    const bool okWorldCache = testWorldCacheHitAfterWarmup();
    const bool okWorldEviction = testWorldEvictionWhileMoving();

    if (!okStorage || !okWorldCache || !okWorldEviction) {
        return 1;
    }

    std::cout << "world_tests passed\n";
    return 0;
}
