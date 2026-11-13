cmake_minimum_required(VERSION 3.28)
project(cpu-emulator CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(
  userver
  COMPONENTS universal
  REQUIRED
)

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")

list(FILTER SOURCES EXCLUDE REGEX ".*(_test|_benchmark)\\.cpp$")

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_link_libraries(${PROJECT_NAME}
  PRIVATE userver::universal
)

target_include_directories(
          ${PROJECT_NAME} PUBLIC
          $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
          $<INSTALL_INTERFACE:include> PRIVATE
          ${CMAKE_CURRENT_SOURCE_DIR}/src
)


option(CPU_EMULATOR_BUILD_SAMPLES "build samples" OFF)

if(CPU_EMULATOR_BUILD_SAMPLES)
  add_subdirectory(samples)
endif()
