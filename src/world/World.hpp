#pragma once

#include "core/JobSystem.hpp"
#include "world/Chunk.hpp"
#include "world/ChunkCoord.hpp"
#include "world/ChunkStorage.hpp"

#include <cstddef>
#include <filesystem>
#include <cstdint>
#include <future>
#include <unordered_map>
#include <vector>

namespace veys::world {

class World {
public:
    explicit World(veys::JobSystem& jobs);
    World(veys::JobSystem& jobs, std::filesystem::path storageRoot);

    void updateStreaming(float playerX, float playerZ, int radius);
    void pollGeneration();

    [[nodiscard]] std::size_t loadedChunkCount() const noexcept;
    [[nodiscard]] std::size_t pendingChunkCount() const noexcept;
    [[nodiscard]] std::size_t cacheHits() const noexcept;
    [[nodiscard]] std::size_t cacheMisses() const noexcept;
    [[nodiscard]] std::size_t generatedChunks() const noexcept;
    [[nodiscard]] std::size_t loadedFromCacheChunks() const noexcept;
    [[nodiscard]] std::size_t evictedChunks() const noexcept;
    [[nodiscard]] std::vector<const Chunk*> loadedChunks() const;

private:
    struct PendingChunk {
        ChunkCoord coord{};
        std::future<Chunk> future{};
    };

    static constexpr std::size_t kMaxLoadedChunks = 96;
    static constexpr std::size_t kMaxGenerationRequestsPerUpdate = 6;

    static int worldToChunk(float worldPos) noexcept;
    [[nodiscard]] static std::uint32_t makeSeed(ChunkCoord coord) noexcept;
    [[nodiscard]] static int distanceSq(ChunkCoord a, ChunkCoord b) noexcept;

    [[nodiscard]] bool isChunkPending(ChunkCoord coord) const;
    void touchChunk(ChunkCoord coord);
    void evictFarChunks(ChunkCoord center, int keepRadius);
    void enforceBudget(ChunkCoord center, int protectedRadius);

    veys::JobSystem& jobs_;
    ChunkStorage storage_;
    std::unordered_map<ChunkCoord, Chunk, ChunkCoordHash> chunks_{};
    std::vector<PendingChunk> pending_{};
    std::unordered_map<ChunkCoord, std::size_t, ChunkCoordHash> touchTick_{};
    std::size_t streamTick_{0};
    std::size_t cacheHits_{0};
    std::size_t cacheMisses_{0};
    std::size_t generatedChunks_{0};
    std::size_t loadedFromCacheChunks_{0};
    std::size_t evictedChunks_{0};
};

} // namespace veys::world
