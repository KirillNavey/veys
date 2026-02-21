#include "world/ChunkStorage.hpp"

#include <fstream>

namespace veys::world {

namespace {
constexpr std::uint32_t kChunkMagic = 0x56435931u; // VCY1
}

ChunkStorage::ChunkStorage(std::filesystem::path rootDir)
    : rootDir_{std::move(rootDir)} {
    std::filesystem::create_directories(rootDir_);
}

std::filesystem::path ChunkStorage::chunkPath(ChunkCoord coord) const {
    return rootDir_ / ("chunk_" + std::to_string(coord.x) + "_" + std::to_string(coord.z) + ".bin");
}

bool ChunkStorage::loadChunk(ChunkCoord coord, Chunk& outChunk) const {
    std::ifstream in{chunkPath(coord), std::ios::binary};
    if (!in) {
        return false;
    }

    std::uint32_t magic = 0;
    in.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    if (!in || magic != kChunkMagic) {
        return false;
    }

    std::array<Voxel, Chunk::kVoxelCount> voxels{};
    in.read(reinterpret_cast<char*>(voxels.data()), static_cast<std::streamsize>(voxels.size() * sizeof(Voxel)));
    if (!in) {
        return false;
    }

    outChunk.replaceAll(voxels);
    return true;
}

bool ChunkStorage::saveChunk(ChunkCoord coord, const Chunk& chunk) const {
    std::ofstream out{chunkPath(coord), std::ios::binary | std::ios::trunc};
    if (!out) {
        return false;
    }

    const std::uint32_t magic = kChunkMagic;
    out.write(reinterpret_cast<const char*>(&magic), sizeof(magic));

    const auto voxels = chunk.voxels();
    out.write(reinterpret_cast<const char*>(voxels.data()), static_cast<std::streamsize>(voxels.size() * sizeof(Voxel)));

    return static_cast<bool>(out);
}

} // namespace veys::world
