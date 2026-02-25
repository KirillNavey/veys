#pragma once

#include "world/Chunk.hpp"
#include "world/ChunkCoord.hpp"

#include <filesystem>

namespace veys::world {

class ChunkStorage {
public:
    explicit ChunkStorage(std::filesystem::path rootDir);

    [[nodiscard]] bool loadChunk(ChunkCoord coord, Chunk& outChunk) const;
    bool saveChunk(ChunkCoord coord, const Chunk& chunk) const;

private:
    [[nodiscard]] std::filesystem::path chunkPath(ChunkCoord coord) const;

    std::filesystem::path rootDir_;
};

} // namespace veys::world
