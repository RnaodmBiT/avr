project(tank_gui)
cmake_minimum_required(VERSION 3.10.0)


add_subdirectory(SDL2-2.0.14)


add_executable(gui main.cpp)
target_link_libraries(gui SDL2)
target_include_directories(gui PRIVATE SDL2-2.0.14/include)
