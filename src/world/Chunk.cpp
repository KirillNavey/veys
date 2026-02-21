#include "world/Chunk.hpp"

#include <cmath>
#include <stdexcept>
#include <vector>

namespace veys::world {

void Chunk::fillDemoTerrain(veys::JobSystem& jobs) {
    std::vector<std::future<void>> futures;
    futures.reserve(kSize);

    for (int z = 0; z < kSize; ++z) {
        futures.emplace_back(jobs.enqueue([this, z]() {
            for (int y = 0; y < kSize; ++y) {
                for (int x = 0; x < kSize; ++x) {
                    const bool solid = y < (kSize / 2 + (x + z) % 3);
                    voxels_[idx(x, y, z)] = Voxel{static_cast<std::uint8_t>(solid ? 1 : 0)};
                }
            }
        }));
    }

    for (auto& f : futures) {
        f.get();
    }
}

void Chunk::generateTerrain(std::uint32_t seed, int chunkX, int chunkZ) {
    for (int z = 0; z < kSize; ++z) {
        for (int x = 0; x < kSize; ++x) {
            const int worldX = chunkX * kSize + x;
            const int worldZ = chunkZ * kSize + z;

            const float nx = static_cast<float>(worldX) * 0.06F;
            const float nz = static_cast<float>(worldZ) * 0.06F;
            const float waveA = std::sin(nx + static_cast<float>(seed & 1023u) * 0.001F);
            const float waveB = std::cos(nz + static_cast<float>((seed >> 10u) & 1023u) * 0.001F);
            const float waveC = std::sin((nx + nz) * 0.5F);

            const int height = 12 + static_cast<int>((waveA + waveB + waveC) * 4.0F);

            for (int y = 0; y < kSize; ++y) {
                const bool solid = y <= height;
                voxels_[idx(x, y, z)] = Voxel{static_cast<std::uint8_t>(solid ? 1 : 0)};
            }
        }
    }
}

const Voxel& Chunk::at(int x, int y, int z) const {
    if (x < 0 || y < 0 || z < 0 || x >= kSize || y >= kSize || z >= kSize) {
        throw std::out_of_range{"Chunk::at out of range"};
    }
    return voxels_[idx(x, y, z)];
}

std::size_t Chunk::solidVoxelCount() const noexcept {
    std::size_t count = 0;
    for (const Voxel& voxel : voxels_) {
        count += voxel.id != 0 ? 1u : 0u;
    }
    return count;
}

std::span<const Voxel, Chunk::kVoxelCount> Chunk::voxels() const noexcept {
    return std::span<const Voxel, kVoxelCount>{voxels_};
}

void Chunk::replaceAll(const std::array<Voxel, kVoxelCount>& data) noexcept {
    voxels_ = data;
}

} // namespace veys::world
