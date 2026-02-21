#pragma once

#include "core/JobSystem.hpp"

#include <array>
#include <cstdint>

namespace veys::world {

struct Voxel {
    std::uint8_t id{0};
};

class Chunk {
public:
    static constexpr int kSize = 32;
    static constexpr int kVoxelCount = kSize * kSize * kSize;

    void fillDemoTerrain(veys::JobSystem& jobs);
    void generateTerrain(std::uint32_t seed, int chunkX, int chunkZ);

    [[nodiscard]] const Voxel& at(int x, int y, int z) const;
    [[nodiscard]] std::size_t solidVoxelCount() const noexcept;

private:
    static constexpr int idx(int x, int y, int z) {
        return x + y * kSize + z * kSize * kSize;
    }

    std::array<Voxel, kVoxelCount> voxels_{};
};

} // namespace veys::world
