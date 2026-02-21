#include "world/World.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <utility>

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

int World::distanceSq(ChunkCoord a, ChunkCoord b) noexcept {
    const int dx = a.x - b.x;
    const int dz = a.z - b.z;
    return dx * dx + dz * dz;
}

bool World::isChunkPending(ChunkCoord coord) const {
    return std::any_of(
        pending_.begin(), pending_.end(),
        [&coord](const PendingChunk& pending) { return pending.coord == coord; });
}

void World::evictFarChunks(ChunkCoord center, int keepRadius) {
    const int keepDistanceSq = keepRadius * keepRadius;

    for (auto it = chunks_.begin(); it != chunks_.end();) {
        if (distanceSq(it->first, center) > keepDistanceSq) {
            storage_.saveChunk(it->first, it->second);
            it = chunks_.erase(it);
            continue;
        }
        ++it;
    }
}

void World::updateStreaming(float playerX, float playerZ, int radius) {
    const ChunkCoord center{worldToChunk(playerX), worldToChunk(playerZ)};
    evictFarChunks(center, radius + 1);

    std::vector<ChunkCoord> candidates;
    candidates.reserve(static_cast<std::size_t>((radius * 2 + 1) * (radius * 2 + 1)));

    for (int dz = -radius; dz <= radius; ++dz) {
        for (int dx = -radius; dx <= radius; ++dx) {
            const ChunkCoord coord{center.x + dx, center.z + dz};
            if (chunks_.contains(coord) || isChunkPending(coord)) {
                continue;
            }

            Chunk cached;
            if (storage_.loadChunk(coord, cached)) {
                chunks_.insert_or_assign(coord, cached);
                continue;
            }

            candidates.push_back(coord);
        }
    }

    std::sort(candidates.begin(), candidates.end(), [center](ChunkCoord lhs, ChunkCoord rhs) {
        return distanceSq(lhs, center) < distanceSq(rhs, center);
    });

    const std::size_t loadedAndPending = chunks_.size() + pending_.size();
    const std::size_t availableBudget = loadedAndPending < kMaxLoadedChunks
                                            ? kMaxLoadedChunks - loadedAndPending
                                            : 0;
    const std::size_t newRequests = std::min({candidates.size(), availableBudget,
                                              kMaxGenerationRequestsPerUpdate});

    for (std::size_t i = 0; i < newRequests; ++i) {
        const ChunkCoord coord = candidates[i];
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

void World::pollGeneration() {
    auto it = pending_.begin();
    while (it != pending_.end()) {
        if (it->future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
            ++it;
            continue;
        }

        Chunk chunk = it->future.get();
        storage_.saveChunk(it->coord, chunk);
        chunks_.insert_or_assign(it->coord, std::move(chunk));
        it = pending_.erase(it);
    }
}

std::size_t World::loadedChunkCount() const noexcept {
    return chunks_.size();
}

std::size_t World::pendingChunkCount() const noexcept {
    return pending_.size();
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
