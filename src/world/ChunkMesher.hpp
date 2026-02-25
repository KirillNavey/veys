#pragma once

#include "world/Chunk.hpp"

#include <cstddef>

namespace veys::world {

struct MeshStats {
    std::size_t visibleFaces{0};
    std::size_t estimatedTriangles{0};
};

class ChunkMesher {
public:
    [[nodiscard]] static MeshStats buildSurfaceStats(const Chunk& chunk);
};

} // namespace veys::world
