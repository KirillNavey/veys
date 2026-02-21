#include "world/ChunkMesher.hpp"

namespace veys::world {

namespace {
[[nodiscard]] bool isSolid(const Chunk& chunk, int x, int y, int z) {
    if (x < 0 || y < 0 || z < 0 || x >= Chunk::kSize || y >= Chunk::kSize || z >= Chunk::kSize) {
        return false;
    }
    return chunk.at(x, y, z).id != 0;
}
} // namespace

MeshStats ChunkMesher::buildSurfaceStats(const Chunk& chunk) {
    std::size_t faces = 0;

    for (int z = 0; z < Chunk::kSize; ++z) {
        for (int y = 0; y < Chunk::kSize; ++y) {
            for (int x = 0; x < Chunk::kSize; ++x) {
                if (!isSolid(chunk, x, y, z)) {
                    continue;
                }

                faces += !isSolid(chunk, x + 1, y, z) ? 1u : 0u;
                faces += !isSolid(chunk, x - 1, y, z) ? 1u : 0u;
                faces += !isSolid(chunk, x, y + 1, z) ? 1u : 0u;
                faces += !isSolid(chunk, x, y - 1, z) ? 1u : 0u;
                faces += !isSolid(chunk, x, y, z + 1) ? 1u : 0u;
                faces += !isSolid(chunk, x, y, z - 1) ? 1u : 0u;
            }
        }
    }

    return MeshStats{.visibleFaces = faces, .estimatedTriangles = faces * 2u};
}

} // namespace veys::world
