cmake_minimum_required(VERSION 3.16)
project(linr-example VERSION 0.1.0)

include(cmake/prelude.cmake)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(fmt REQUIRED)
find_package(CLI11 REQUIRED)

add_subdirectory(lib/linr)

function(create_exe name)
  add_executable(${name} source/${name}.cpp)
  target_include_directories(${name} PRIVATE source)
  target_link_libraries(${name} PRIVATE linr fmt::fmt CLI11::CLI11)
  target_compile_options(${name} PRIVATE -Wall -Wextra -Wconversion)

  # # sanitizer
  # target_compile_options(${name} PRIVATE -fsanitize=address,leak,undefined)
  # target_link_options(${name} PRIVATE -fsanitize=address,leak,undefined)
endfunction()

create_exe(main)
create_exe(bench)
create_exe(bufread)
create_exe(custom_type)
