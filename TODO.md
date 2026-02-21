# VeysCraft TODO (до уровня production)

> Статусы: `[x]` готово, `[~]` в работе, `[ ]` запланировано.

## 0) Foundation
- [x] Базовый C++20 проект на CMake.
- [x] Thread pool (`JobSystem`) и безопасное завершение воркеров.
- [x] Базовый `Application` loop.
- [x] Прототип чанка 32³ с параллельной демо-генерацией.
- [x] Vulkan-aware renderer с fallback-веткой.

## 1) World/Streaming
- [x] Ввести координаты чанков и контейнер мира.
- [x] Асинхронная генерация чанков вокруг позиции игрока.
- [x] Неблокирующий polling завершённых задач генерации.
- [x] Ограниченный радиус стриминга (клипмап уровня 0).
- [x] Удаление дальних чанков и лимиты по памяти (базовый budget).
- [x] Система приоритетов (ближние чанки важнее).
- [x] Сериализация/десериализация чанков на диск (базовый binary cache).

## 2) Render Backend
- [~] CPU stub с обработкой нескольких чанков.
- [ ] Реальный Vulkan backend: instance/device/swapchain.
- [ ] Upload manager и GPU allocator (staging/transient).
- [ ] Mesh generation (greedy meshing / mesh shaders).
- [ ] Visibility + occlusion + Hi-Z.
- [ ] Frame graph и async compute-проходы.

## 3) Simulation
- [~] Позиция игрока и обновление world-streaming по тикy.
- [ ] Ввод (мышь/клавиатура), камера, коллизии.
- [ ] Блок-редактирование (place/break) с репликацией изменений.
- [ ] ECS/компонентная модель для сущностей.
- [ ] Physics + AI + gameplay loops.

## 4) Tooling/Quality
- [x] Документация архитектуры и roadmap.
- [ ] Профилирование (CPU/GPU markers), telemetry HUD.
- [~] Юнит-тесты core/world модулей (добавлен стартовый набор world tests).
- [~] Интеграционные тесты генерации/стриминга (добавлены сценарии cache+eviction move test).
- [ ] Crash reporting и deterministic replay.

## 5) Online/LiveOps
- [ ] Клиент-серверная модель и snapshot interpolation.
- [ ] Authoritative server + anti-cheat baseline.
- [ ] Session services, matchmaking, persistence.

## Ближайшие 3 спринта
1. **Sprint A (сделано в этом PR):** world streaming foundation + async chunk generation + renderer integration.
2. **Sprint B (закрыт):** eviction policy + chunk priorities + memory budget + save/load cache + базовый LRU eviction.
3. **Sprint C:** Vulkan swapchain path, upload buffers, первые drawable chunk meshes.
