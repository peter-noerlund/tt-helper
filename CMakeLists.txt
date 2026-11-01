cmake_minimum_required(VERSION 3.31)

list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR})

project(tt-helper LANGUAGES CXX)

find_package(asio CONFIG REQUIRED)
find_package(lyra CONFIG REQUIRED)
find_package(PkgConfig)

if(PkgConfig_FOUND)
    pkg_check_modules(libudev IMPORTED_TARGET "libudev>=221")
endif()

add_executable(tt-helper
    src/base64_encode.cpp
    src/find_device.cpp
    src/flash_writer.cpp
    src/install_fpga.cpp
    src/main.cpp
    src/micro_python_device.cpp
)
target_compile_features(tt-helper PRIVATE cxx_std_23)
target_include_directories(tt-helper PRIVATE src)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(tt-helper PRIVATE -Wall -W -Wextra -Wpedantic)
endif()
target_link_libraries(tt-helper PRIVATE asio::asio lyra::lyra)
if(PkgConfig_FOUND AND libudev_FOUND)
    target_link_libraries(tt-helper PRIVATE PkgConfig::libudev)
    target_compile_definitions(tt-helper PRIVATE HAVE_LIBUDEV)
endif()
