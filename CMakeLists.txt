cmake_minimum_required(VERSION 3.15)
project(pyopenalsoft VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 REQUIRED)

file(GLOB SRC_FILES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

add_library(pyopenalsoft MODULE ${SRC_FILES})

# Python extension naming
set_target_properties(pyopenalsoft PROPERTIES
    PREFIX ""
    SUFFIX ".pyd"   # Windows; ignored on Linux/macOS
)

target_include_directories(pyopenalsoft PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${Python3_INCLUDE_DIRS}
)

target_link_libraries(pyopenalsoft PRIVATE
    pybind11::module
)

# Needed for dlopen on Linux
if(UNIX AND NOT APPLE)
    target_link_libraries(pyopenalsoft PRIVATE dl)
endif()
