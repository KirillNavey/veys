# VeysCraft — архитектура AAA-уровня (дорожная карта)

Этот репозиторий содержит **технический фундамент** под voxel-sandbox проект уровня Minecraft-next-gen на C++20 с упором на Vulkan и масштабируемость.

## Ключевые принципы
- RAII и строгая ownership-модель для исключения утечек памяти.
- Многопоточность по всем CPU-ядрам через `JobSystem`.
- Подготовка к modern GPU pipeline (bindless, mesh/task shaders, RT, async compute).
- Data-oriented подход (chunks/voxels как компактные массивы).

## Что уже есть
- Базовый `Application` loop.
- `JobSystem` thread pool с количеством воркеров = hardware concurrency.
- Chunk-сетка 32³ и детерминированная генерация рельефа.
- `World` контейнер с асинхронным streaming чанков вокруг игрока.
- Базовый eviction policy + приоритизация генерации ближайших чанков + budget на количество чанков в памяти.
- Рендерер с Vulkan-aware и CPU fallback-веткой.

## Следующие этапы
1. **World streaming production-ready**:
   - [x] eviction + базовый memory budget.
   - [x] приоритизация генерации ближних чанков.
   - [ ] сериализация/кеш чанков на диск.
   - [ ] LRU/temperature-based memory eviction.
2. **Vulkan backend production-ready**:
   - Instance/device/swapchain/descriptor allocator/frame graph.
   - GPU memory allocator + transient allocators.
3. **Rendering**:
   - chunk meshing, PBR materials, denoising, RTGI.
4. **Gameplay systems**:
   - Physics, AI, crafting, multiplayer replication.
5. **Tooling**:
   - Editor, telemetry, crash reporting, hot-reload shaders.

Детализация задач и статусы: [TODO.md](TODO.md)
