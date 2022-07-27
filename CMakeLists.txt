cmake_minimum_required(VERSION 3.0.0)
project(SimpleSuperMalloc VERSION 0.0.1)

include(CTest)
enable_testing()

aux_source_directory(src sources)
add_executable(SimpleSuperMalloc ${sources})

add_subdirectory(esm)
target_include_directories(SimpleSuperMalloc PUBLIC esm)
target_link_libraries(SimpleSuperMalloc PUBLIC esm)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 14)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
