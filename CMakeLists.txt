cmake_minimum_required(VERSION 3.20)
project(Minesweeper)

set(CMAKE_CXX_STANDARD 20)

set(SFML_DIR "C:/SFML/lib/cmake/SFML")
find_package(SFML 3.0.2 COMPONENTS Graphics Window System REQUIRED)

add_executable(Minesweeper main.cpp)
target_link_libraries(Minesweeper SFML::Graphics SFML::Window SFML::System)

enable_testing()

add_executable(minesweeper_tests tests/test_map.cpp)
target_include_directories(minesweeper_tests PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/tests
)

add_test(NAME minesweeper_tests COMMAND minesweeper_tests)