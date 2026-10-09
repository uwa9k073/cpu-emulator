cmake_minimum_required(VERSION 3.24)
project(ProcessorEmulator CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_MODULE_EXTENSIONS cppm)

# FTXUI dependency
include(FetchContent)
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
  GIT_TAG v5.0.0
)
FetchContent_MakeAvailable(ftxui)

# Main executable
add_executable(processor-emulator
  src/main.cpp
)

# Enable C++20 modules
set_target_properties(processor-emulator PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  CXX_MODULE_STD_MODULES ON
)

target_link_libraries(processor-emulator
  PRIVATE ftxui::component
  PRIVATE ftxui::dom
  PRIVATE ftxui::screen
)

# Module sources
target_sources(processor-emulator
  PRIVATE
    src/modules/architecture.cppm
    src/modules/memory.cppm
    src/modules/cpu.cppm
    src/modules/instruction.cppm
    src/modules/assembler.cppm
    src/modules/emulator.cppm
)
