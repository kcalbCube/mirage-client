cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

project(MirageClient CXX)
set(PROJECT_NAME client)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

include(CMake/Utils.cmake)
include(CMake/cotire.cmake)

add_subdirectory("${CMAKE_SOURCE_DIR}/include/SDL_image")
add_subdirectory("${CMAKE_SOURCE_DIR}/include/SDL")
add_subdirectory("${CMAKE_SOURCE_DIR}/include/boost" EXCLUDE_FROM_ALL)
add_subdirectory("${CMAKE_SOURCE_DIR}/include/fmtlog")

add_subdirectory("${CMAKE_SOURCE_DIR}/core")

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "src/*.h" "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

cotire(boost_serialization)
cotire(mirage-core)
cotire(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PUBLIC SDL2 SDL2main boost_serialization fmt-header-only fmtlog-static mirage-core)
