cmake_minimum_required(VERSION 3.21)
project(DatingApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

qt_add_executable(DatingApp WIN32
    src/main.cpp
)

target_link_libraries(DatingApp PRIVATE Qt6::Widgets)
target_compile_definitions(DatingApp PRIVATE APP_VERSION="${PROJECT_VERSION}")
