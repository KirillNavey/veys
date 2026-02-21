#include "world/World.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>

namespace veys::world {

std::size_t ChunkCoordHash::operator()(const ChunkCoord& coord) const noexcept {
    const std::size_t hx = static_cast<std::size_t>(static_cast<std::uint32_t>(coord.x));
    const std::size_t hz = static_cast<std::size_t>(static_cast<std::uint32_t>(coord.z));
    return hx * 73856093u ^ hz * 19349663u;
}

World::World(veys::JobSystem& jobs)
    : jobs_{jobs} {
}

int World::worldToChunk(float worldPos) noexcept {
    return static_cast<int>(std::floor(worldPos / static_cast<float>(Chunk::kSize)));
}

std::uint32_t World::makeSeed(ChunkCoord coord) noexcept {
    const auto x = static_cast<std::uint32_t>(coord.x * 1664525);
    const auto z = static_cast<std::uint32_t>(coord.z * 1013904223);
    return x ^ z ^ 0x9E3779B9u;
}

void World::updateStreaming(float playerX, float playerZ, int radius) {
    const ChunkCoord center{worldToChunk(playerX), worldToChunk(playerZ)};

    for (int dz = -radius; dz <= radius; ++dz) {
        for (int dx = -radius; dx <= radius; ++dx) {
            ChunkCoord coord{center.x + dx, center.z + dz};
            if (chunks_.contains(coord)) {
                continue;
            }

            const bool isPending = std::any_of(
                pending_.begin(), pending_.end(),
                [&coord](const PendingChunk& pending) { return pending.coord == coord; });
            if (isPending) {
                continue;
            }

            pending_.push_back(PendingChunk{
                .coord = coord,
                .future = jobs_.enqueue([coord]() {
                    Chunk chunk;
                    chunk.generateTerrain(makeSeed(coord), coord.x, coord.z);
                    return chunk;
                }),
            });
        }
    }
}

void World::pollGeneration() {
    auto it = pending_.begin();
    while (it != pending_.end()) {
        if (it->future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
            ++it;
            continue;
        }

        chunks_.insert_or_assign(it->coord, it->future.get());
        it = pending_.erase(it);
    }
}

std::size_t World::loadedChunkCount() const noexcept {
    return chunks_.size();
}

std::vector<const Chunk*> World::loadedChunks() const {
    std::vector<const Chunk*> result;
    result.reserve(chunks_.size());
    for (const auto& [coord, chunk] : chunks_) {
        (void)coord;
        result.push_back(&chunk);
    }
    return result;
}

} // namespace veys::world
