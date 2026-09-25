cmake_minimum_required(VERSION 3.21)
project(DatingApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SODIUM REQUIRED IMPORTED_TARGET libsodium)

qt_standard_project_setup()

qt_add_library(dating_core STATIC
    src/core/Compatibility.cpp
    src/core/PasswordPolicy.cpp
    src/core/PasswordHasher.cpp
)

target_include_directories(dating_core PUBLIC src)
target_link_libraries(dating_core PUBLIC Qt6::Core PRIVATE PkgConfig::SODIUM)


qt_add_executable(DatingApp WIN32
    src/main.cpp
)

target_link_libraries(DatingApp PRIVATE dating_core Qt6::Widgets)
target_compile_definitions(DatingApp PRIVATE APP_VERSION="${PROJECT_VERSION}")

find_package(Qt6 REQUIRED COMPONENTS Test)
enable_testing()
qt_add_executable(test_core tests/test_core.cpp)
target_link_libraries(test_core PRIVATE dating_core Qt6::Test)
add_test(NAME test_core COMMAND test_core)
