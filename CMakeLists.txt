cmake_minimum_required(VERSION 2.8)

project(difacto)

include_directories(
  "${PROJECT_SOURCE_DIR}"
  "${PROJECT_BINARY_DIR}"
  "${PROJECT_SOURCE_DIR}/include"
)

add_subdirectory(${PROJECT_SOURCE_DIR}/dmlc-core)
add_subdirectory(${PROJECT_SOURCE_DIR}/ps-lite)

FILE(GLOB SOURCES
  src/common/*.h
  src/data/*.h
  src/data/*.c
)

message(${SOURCES})