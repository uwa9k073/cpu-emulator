cmake_minimum_required(VERSION 3.28)
project(cpu-emulator CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# FTXUI dependency
# include(FetchContent)
# FetchContent_Declare(ftxui
#   GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
#   GIT_TAG v7.0.3
# )
# FetchContent_MakeAvailable(ftxui)

# Main executable
add_executable(${PROJECT_NAME}
)

# target_link_libraries(${PROJECT_NAME}
#   PRIVATE ftxui::component
#   PRIVATE ftxui::dom
#   PRIVATE ftxui::screen
# )

# Module sources
target_sources(${PROJECT_NAME}
    PRIVATE
        src/main.cpp # Обычный файл с точкой входа int main()
    PUBLIC FILE_SET CXX_MODULES FILES
        src/modules/types.cppm
        src/modules/architecture.cppm
        src/modules/alu.cppm
        src/modules/memory.cppm
        src/modules/cpu.cppm
        src/modules/instruction.cppm
        src/modules/assembler.cppm
        src/modules/emulator.cppm
)
