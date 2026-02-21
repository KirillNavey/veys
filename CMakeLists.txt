cmake_minimum_required(VERSION 3.22)
project(VeysCraft VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(veyscraft
    src/main.cpp
    src/core/Application.cpp
    src/core/JobSystem.cpp
    src/world/Chunk.cpp
    src/world/World.cpp
    src/world/ChunkStorage.cpp
    src/world/ChunkMesher.cpp
    src/renderer/Renderer.cpp
)

target_include_directories(veyscraft PRIVATE src)

target_compile_options(veyscraft PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

find_package(Threads REQUIRED)
target_link_libraries(veyscraft PRIVATE Threads::Threads)

include(CTest)
if(BUILD_TESTING)
    add_executable(world_tests
        tests/world_tests.cpp
        src/core/JobSystem.cpp
        src/world/Chunk.cpp
        src/world/World.cpp
        src/world/ChunkStorage.cpp
        src/world/ChunkMesher.cpp
    )

    target_include_directories(world_tests PRIVATE src)
    target_compile_options(world_tests PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
    )
    target_link_libraries(world_tests PRIVATE Threads::Threads)

    add_test(NAME world_tests COMMAND world_tests)
endif()
