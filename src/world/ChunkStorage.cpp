#include "world/ChunkStorage.hpp"

#include <fstream>
#include <system_error>

namespace veys::world {

namespace {
constexpr std::uint32_t kChunkMagic = 0x56435931u; // VCY1
constexpr std::uint32_t kChunkFormatVersion = 1u;

struct ChunkFileHeader {
    std::uint32_t magic{0};
    std::uint32_t version{0};
    std::uint32_t voxelCount{0};
};

[[nodiscard]] constexpr std::uint32_t expectedVoxelCount() {
    return static_cast<std::uint32_t>(Chunk::kVoxelCount);
}

[[nodiscard]] constexpr std::uintmax_t expectedFileSize() {
    return sizeof(ChunkFileHeader) + static_cast<std::uintmax_t>(Chunk::kVoxelCount) * sizeof(Voxel);
}

} // namespace

ChunkStorage::ChunkStorage(std::filesystem::path rootDir)
    : rootDir_{std::move(rootDir)} {
    std::filesystem::create_directories(rootDir_);
}

std::filesystem::path ChunkStorage::chunkPath(ChunkCoord coord) const {
    return rootDir_ / ("chunk_" + std::to_string(coord.x) + "_" + std::to_string(coord.z) + ".bin");
}

bool ChunkStorage::loadChunk(ChunkCoord coord, Chunk& outChunk) const {
    const auto path = chunkPath(coord);

    std::error_code ec;
    const auto size = std::filesystem::file_size(path, ec);
    if (ec || size != expectedFileSize()) {
        return false;
    }

    std::ifstream in{path, std::ios::binary};
    if (!in) {
        return false;
    }

    ChunkFileHeader header{};
    in.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (!in || header.magic != kChunkMagic || header.version != kChunkFormatVersion ||
        header.voxelCount != expectedVoxelCount()) {
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
    const auto finalPath = chunkPath(coord);
    const auto tempPath = finalPath.string() + ".tmp";

    {
        std::ofstream out{tempPath, std::ios::binary | std::ios::trunc};
        if (!out) {
            return false;
        }

        const ChunkFileHeader header{
            .magic = kChunkMagic,
            .version = kChunkFormatVersion,
            .voxelCount = expectedVoxelCount(),
        };
        out.write(reinterpret_cast<const char*>(&header), sizeof(header));

        const auto voxels = chunk.voxels();
        out.write(reinterpret_cast<const char*>(voxels.data()),
                  static_cast<std::streamsize>(voxels.size() * sizeof(Voxel)));

        if (!out) {
            std::error_code removeError;
            std::filesystem::remove(tempPath, removeError);
            return false;
        }
    }

    std::error_code ec;
    std::filesystem::rename(tempPath, finalPath, ec);
    if (ec) {
        std::filesystem::remove(finalPath, ec);
        ec.clear();
        std::filesystem::rename(tempPath, finalPath, ec);
    }

    if (ec) {
        std::error_code removeError;
        std::filesystem::remove(tempPath, removeError);
        return false;
    }

    return true;
}

} // namespace veys::world
