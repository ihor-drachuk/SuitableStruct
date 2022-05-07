cmake_minimum_required(VERSION 3.16)
PROJECT(SuitableStruct)

option(SUITABLE_STRUCT_ENABLE_TESTS     "SuitableStruct: Enable tests" OFF)
option(SUITABLE_STRUCT_ENABLE_BENCHMARK "SuitableStruct: Enable benchmark" OFF)

FILE(GLOB_RECURSE SOURCES src/*.cpp src/*.h headers/*.h)

add_library(SuitableStruct STATIC ${SOURCES})
target_include_directories(SuitableStruct PUBLIC headers)
target_compile_features(SuitableStruct PUBLIC cxx_std_17)

find_package(QT NAMES Qt6 Qt5 OPTIONAL_COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} QUIET OPTIONAL_COMPONENTS COMPONENTS Core)

if(Qt5_FOUND OR Qt6_FOUND)
    target_link_libraries(SuitableStruct PUBLIC Qt${QT_VERSION_MAJOR}::Core)
    target_compile_definitions(SuitableStruct PUBLIC SUITABLE_STRUCT_HAS_QT_LIBRARY)
endif()

if(MSVC)
    target_link_options(SuitableStruct PRIVATE "/ignore:4221")
    set_target_properties(SuitableStruct PROPERTIES STATIC_LIBRARY_OPTIONS "/ignore:4221")
endif()

add_subdirectory(tests)
