cmake_minimum_required(VERSION 3.12)

project(
  linr
  VERSION 0.0.0
  LANGUAGES CXX
  DESCRIPTION "Simple console input library")

add_library(linr INTERFACE)
target_include_directories(linr INTERFACE include)
target_compile_features(linr INTERFACE cxx_std_20)
set_target_properties(linr PROPERTIES CXX_EXTENSIONS OFF)

if(LINUX)
  target_compile_definitions(linr INTERFACE LINR_ENABLE_GETLINE)
endif()
