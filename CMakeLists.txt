cmake_minimum_required(VERSION 3.12)

project(
  cppread
  VERSION 0.0.0
  LANGUAGES CXX
  DESCRIPTION "Simple console input library")

add_library(cppread INTERFACE)
target_include_directories(cppread INTERFACE include)
target_compile_features(cppread INTERFACE cxx_std_20)
set_target_properties(cppread PROPERTIES CXX_EXTENSIONS OFF)
