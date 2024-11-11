cmake_minimum_required(VERSION 3.16)
project(cppread-example VERSION 0.0.0)

include(cmake/prelude.cmake)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(lib/cppread)

# ~~~
# main
# ~~~
add_executable(main source/main.cpp)
target_include_directories(main PRIVATE source)
target_link_libraries(main PRIVATE cppread)
target_compile_options(main PRIVATE -Wall -Wextra -Wconversion)

# sanitizer
target_compile_options(main PRIVATE -fsanitize=address,leak,undefined)
target_link_options(main PRIVATE -fsanitize=address,leak,undefined)

# ~~~
# bench
# ~~~
add_executable(bench source/bench.cpp)
target_include_directories(bench PRIVATE source)
target_link_libraries(bench PRIVATE cppread)
target_compile_options(bench PRIVATE -Wall -Wextra -Wconversion)

add_executable(bench_cin source/bench.cpp)
target_include_directories(bench_cin PRIVATE source)
target_link_libraries(bench_cin PRIVATE cppread)
target_compile_options(bench_cin PRIVATE -Wall -Wextra -Wconversion)
target_compile_definitions(bench_cin PRIVATE USE_CIN)
