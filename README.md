# VeysCraft

Прототип высокопроизводительного voxel-движка на C++20.

## Быстрый старт
```bash
cmake -S . -B build
cmake --build build -j
./build/veyscraft
ctest --test-dir build --output-on-failure
```

## Что уже реализовано
- Масштабирование по всем ядрам CPU через `JobSystem`.
- Асинхронный world streaming: генерация чанков вокруг игрока в фоне.
- Базовые chunk priorities и eviction policy с лимитом на число чанков в памяти.
- Базовый бинарный cache чанков на диск (save/load) с валидацией формата и атомарным сохранением.
- Базовый LRU eviction + runtime telemetry cache hit/miss/eviction.
- Surface-face meshing baseline и оценка triangle count в runtime telemetry.
- Подготовка к Vulkan backend для современных GPU (включая RTX 50xx).
- Безопасная архитектура с RAII и без ручного управления временем жизни.

## Планы
- Полный backlog и статусы задач: [TODO.md](TODO.md)
- Архитектурная дорожная карта: [ARCHITECTURE.md](ARCHITECTURE.md)
