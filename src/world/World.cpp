#include "world/World.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <utility>

namespace veys::world {

std::size_t ChunkCoordHash::operator()(const ChunkCoord& coord) const noexcept {
    const std::size_t hx = static_cast<std::size_t>(static_cast<std::uint32_t>(coord.x));
    const std::size_t hz = static_cast<std::size_t>(static_cast<std::uint32_t>(coord.z));
    return hx * 73856093u ^ hz * 19349663u;
}

World::World(veys::JobSystem& jobs)
    : World{jobs, "cache/chunks"} {
}

World::World(veys::JobSystem& jobs, std::filesystem::path storageRoot)
    : jobs_{jobs}, storage_{std::move(storageRoot)} {
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

void World::touchChunk(ChunkCoord coord) {
    touchTick_.insert_or_assign(coord, streamTick_);
}

void World::evictFarChunks(ChunkCoord center, int keepRadius) {
    const int keepDistanceSq = keepRadius * keepRadius;

    for (auto it = chunks_.begin(); it != chunks_.end();) {
        if (distanceSq(it->first, center) > keepDistanceSq) {
            storage_.saveChunk(it->first, it->second);
            ++evictedChunks_;
            touchTick_.erase(it->first);
            it = chunks_.erase(it);
            continue;
        }
        ++it;
    }
}

void World::enforceBudget(ChunkCoord center, int protectedRadius) {
    const int protectedDistanceSq = protectedRadius * protectedRadius;

    while (chunks_.size() > kMaxLoadedChunks) {
        auto victim = chunks_.end();
        std::size_t victimTick = std::numeric_limits<std::size_t>::max();
        int victimDistance = -1;

        for (auto it = chunks_.begin(); it != chunks_.end(); ++it) {
            const int dist = distanceSq(it->first, center);
            if (dist <= protectedDistanceSq) {
                continue;
            }

            const auto touchIt = touchTick_.find(it->first);
            const std::size_t tick = touchIt != touchTick_.end() ? touchIt->second : 0;

            if (tick < victimTick || (tick == victimTick && dist > victimDistance)) {
                victim = it;
                victimTick = tick;
                victimDistance = dist;
            }
        }

        if (victim == chunks_.end()) {
            break;
        }

        storage_.saveChunk(victim->first, victim->second);
        ++evictedChunks_;
        touchTick_.erase(victim->first);
        chunks_.erase(victim);
    }
}

void World::updateStreaming(float playerX, float playerZ, int radius) {
    ++streamTick_;
    const ChunkCoord center{worldToChunk(playerX), worldToChunk(playerZ)};
    evictFarChunks(center, radius + 1);

    std::vector<ChunkCoord> candidates;
    candidates.reserve(static_cast<std::size_t>((radius * 2 + 1) * (radius * 2 + 1)));

    for (int dz = -radius; dz <= radius; ++dz) {
        for (int dx = -radius; dx <= radius; ++dx) {
            const ChunkCoord coord{center.x + dx, center.z + dz};
            if (chunks_.contains(coord) || isChunkPending(coord)) {
                touchChunk(coord);
                continue;
            }

            Chunk cached;
            if (storage_.loadChunk(coord, cached)) {
                ++cacheHits_;
                ++loadedFromCacheChunks_;
                chunks_.insert_or_assign(coord, cached);
                touchChunk(coord);
                continue;
            }

            ++cacheMisses_;
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
        touchChunk(coord);
    }

    enforceBudget(center, radius);
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
        ++generatedChunks_;
        chunks_.insert_or_assign(it->coord, std::move(chunk));
        touchChunk(it->coord);
        it = pending_.erase(it);
    }
}

std::size_t World::loadedChunkCount() const noexcept {
    return chunks_.size();
}

std::size_t World::pendingChunkCount() const noexcept {
    return pending_.size();
}

std::size_t World::cacheHits() const noexcept {
    return cacheHits_;
}

std::size_t World::cacheMisses() const noexcept {
    return cacheMisses_;
}

std::size_t World::generatedChunks() const noexcept {
    return generatedChunks_;
}

std::size_t World::loadedFromCacheChunks() const noexcept {
    return loadedFromCacheChunks_;
}

std::size_t World::evictedChunks() const noexcept {
    return evictedChunks_;
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
