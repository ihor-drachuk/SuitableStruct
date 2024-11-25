cmake_minimum_required(VERSION 3.16)
PROJECT(SuitableStruct LANGUAGES CXX)

option(SUITABLE_STRUCT_ENABLE_TESTS     "SuitableStruct: Enable tests" OFF)
option(SUITABLE_STRUCT_ENABLE_BENCHMARK "SuitableStruct: Enable benchmark" OFF)

set(SUITABLE_STRUCT_QT_SEARCH_MODE "Auto" CACHE STRING "SuitableStruct: Set Qt search mode")
set_property(CACHE SUITABLE_STRUCT_QT_SEARCH_MODE PROPERTY STRINGS "Auto" "Force" "Skip")

set(SUITABLE_STRUCT_GTEST_SEARCH_MODE "Auto" CACHE STRING "SuitableStruct: Set GTest search mode")
set_property(CACHE SUITABLE_STRUCT_GTEST_SEARCH_MODE PROPERTY STRINGS "Auto" "Force" "Skip")

FILE(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp src/*.h headers/*.h)

add_library(SuitableStruct STATIC ${SOURCES})
target_include_directories(SuitableStruct PUBLIC headers)
target_compile_features(SuitableStruct PUBLIC cxx_std_17)

if (NOT ${SUITABLE_STRUCT_QT_SEARCH_MODE} STREQUAL "Skip")

    if (${SUITABLE_STRUCT_QT_SEARCH_MODE} STREQUAL "Auto")
        set(SUITABLE_STRUCT__INTERNAL_QT_SEARCH_MODE QUIET)
    else()
        set(SUITABLE_STRUCT__INTERNAL_QT_SEARCH_MODE REQUIRED)
    endif()

    find_package(Qt6 COMPONENTS Core Gui QUIET)
    if (NOT Qt6_FOUND)
        find_package(Qt5 5.15 COMPONENTS Core Gui ${SUITABLE_STRUCT__INTERNAL_QT_SEARCH_MODE})
    endif()
endif()

if(Qt5_FOUND OR Qt6_FOUND)
    target_link_libraries(SuitableStruct
        PUBLIC
            Qt::Core
        PRIVATE
            Qt::Gui
    )

    target_compile_definitions(SuitableStruct PUBLIC SUITABLE_STRUCT_HAS_QT_LIBRARY)
endif()

if(MSVC)
    target_link_options(SuitableStruct PRIVATE "/ignore:4221")
    set_target_properties(SuitableStruct PROPERTIES STATIC_LIBRARY_OPTIONS "/ignore:4221")
    target_compile_options(SuitableStruct PRIVATE "/WX")

    # MSVC 2022 + Qt5 + QVector::operator== warning mitigation
    if(Qt5_FOUND AND MSVC_VERSION GREATER_EQUAL 1929)
        target_compile_definitions(SuitableStruct PUBLIC _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS=1 _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING=1)
    endif()
else()
    target_compile_options(SuitableStruct PRIVATE "-Werror")
endif()

add_subdirectory(tests)
