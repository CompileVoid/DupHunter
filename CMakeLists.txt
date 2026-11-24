cmake_minimum_required(VERSION 3.25)
project(DupHunter VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)
option(DUPHUNTER_ENABLE_SANITIZERS "Enable ASan/UBSan" OFF)

add_library(dh_warnings INTERFACE)
if(MSVC)
    target_compile_options(dh_warnings INTERFACE /W4 /permissive- /WX)
else()
    target_compile_options(dh_warnings INTERFACE
            -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror)
endif()

add_library(dh_sanitizers INTERFACE)
if(DUPHUNTER_ENABLE_SANITIZERS)
    if(MSVC)
        target_compile_options(dh_sanitizers INTERFACE /fsanitize=address)
    else()
        target_compile_options(dh_sanitizers INTERFACE
                -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(dh_sanitizers INTERFACE -fsanitize=address,undefined)
    endif()
endif()

add_library(duphunter_core src/core.cpp src/scanner.cpp src/thread_pool.cpp)
target_include_directories(duphunter_core PUBLIC include)
target_link_libraries(duphunter_core PUBLIC dh_sanitizers PRIVATE dh_warnings)

add_executable(duphunter src/main.cpp)
target_link_libraries(duphunter PRIVATE duphunter_core dh_warnings)

if(BUILD_TESTING)
    find_package(GTest CONFIG REQUIRED)
    add_executable(duphunter_tests tests/test_smoke.cpp tests/test_scanner.cpp)
    target_link_libraries(duphunter_tests PRIVATE duphunter_core dh_warnings GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(duphunter_tests)
endif()