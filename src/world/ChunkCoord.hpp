#pragma once

#include <cstddef>

namespace veys::world {

struct ChunkCoord {
    int x{0};
    int z{0};

    [[nodiscard]] bool operator==(const ChunkCoord& other) const noexcept = default;
};

struct ChunkCoordHash {
    [[nodiscard]] std::size_t operator()(const ChunkCoord& coord) const noexcept;
};

} // namespace veys::world
