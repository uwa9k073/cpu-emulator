add_executable(basic_sample main.cpp)

target_link_libraries(
      basic_sample
    PRIVATE cpu-emulator userver::universal
)

target_include_directories(basic_sample PRIVATE
                           ../../include)
