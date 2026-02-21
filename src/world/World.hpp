#pragma once

#include "core/JobSystem.hpp"
#include "world/Chunk.hpp"

#include <cstddef>
#include <cstdint>
#include <future>
#include <optional>
#include <unordered_map>
#include <vector>

namespace veys::world {

struct ChunkCoord {
    int x{0};
    int z{0};

    [[nodiscard]] bool operator==(const ChunkCoord& other) const noexcept = default;
};

struct ChunkCoordHash {
    [[nodiscard]] std::size_t operator()(const ChunkCoord& coord) const noexcept;
};

class World {
public:
    explicit World(veys::JobSystem& jobs);

    void updateStreaming(float playerX, float playerZ, int radius);
    void pollGeneration();

    [[nodiscard]] std::size_t loadedChunkCount() const noexcept;
    [[nodiscard]] std::vector<const Chunk*> loadedChunks() const;

private:
    struct PendingChunk {
        ChunkCoord coord{};
        std::future<Chunk> future{};
    };

    static int worldToChunk(float worldPos) noexcept;
    [[nodiscard]] static std::uint32_t makeSeed(ChunkCoord coord) noexcept;

    veys::JobSystem& jobs_;
    std::unordered_map<ChunkCoord, Chunk, ChunkCoordHash> chunks_{};
    std::vector<PendingChunk> pending_{};
};

} // namespace veys::world
