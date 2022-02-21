cmake_minimum_required(VERSION 3.16)
PROJECT(SuitableStruct)

option(SUITABLE_STRUCT_ENABLE_TESTS     "SuitableStruct: Enable tests" OFF)
option(SUITABLE_STRUCT_ENABLE_BENCHMARK "SuitableStruct: Enable benchmark" OFF)

FILE(GLOB_RECURSE SOURCES src/*.cpp src/*.h headers/*.h)

add_library(SuitableStruct STATIC ${SOURCES})
target_include_directories(SuitableStruct PUBLIC headers)
target_compile_features(SuitableStruct PUBLIC cxx_std_17)

find_package(Qt5 QUIET OPTIONAL_COMPONENTS Core)

if(Qt5_FOUND)
    target_link_libraries(SuitableStruct PRIVATE Qt5::Core)
    target_compile_definitions(SuitableStruct PUBLIC -DSUITABLE_STRUCT_HAS_QT_LIBRARY)
endif()

if(MSVC)
    target_link_options(SuitableStruct PRIVATE "/ignore:4221")
    set_target_properties(SuitableStruct PROPERTIES STATIC_LIBRARY_OPTIONS "/ignore:4221")
endif()

add_subdirectory(tests)
