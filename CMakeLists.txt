cmake_minimum_required(VERSION 3.14)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

project(Chip8Emulator CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/utf-8)
endif()


find_package(SFML 2.5 COMPONENTS graphics window system audio REQUIRED)


include(FetchContent)
FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest
    GIT_TAG v2.4.11
)
FetchContent_MakeAvailable(doctest)


add_library(Chip8Lib src/Chip8.cpp)
target_include_directories(Chip8Lib PUBLIC include)


add_executable(chip8_emu src/main.cpp)
target_link_libraries(chip8_emu PRIVATE Chip8Lib sfml-graphics sfml-window sfml-system sfml-audio)


enable_testing()
add_executable(chip8_tests tests/test_chip8.cpp)
target_link_libraries(chip8_tests PRIVATE Chip8Lib doctest::doctest)

add_test(NAME Chip8Tests COMMAND chip8_tests)


find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_LATEX NO)
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs)
    doxygen_add_docs(doxygen_docs ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src COMMENT "Generate Doxygen documentation")
endif()
