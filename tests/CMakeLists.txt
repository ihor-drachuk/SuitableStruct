set(SUITABLE_STRUCT_NEED_GTEST OFF)
set(SUITABLE_STRUCT_NEED_BENCHMARK OFF)

if (NOT SUITABLE_STRUCT_ENABLE_TESTS AND NOT SUITABLE_STRUCT_ENABLE_BENCHMARK)
    return()
endif()

include(FetchContent)

if (SUITABLE_STRUCT_ENABLE_TESTS)
    set(SUITABLE_STRUCT_NEED_GTEST ON)
endif()

if (SUITABLE_STRUCT_ENABLE_BENCHMARK)
    set(SUITABLE_STRUCT_NEED_GTEST ON)
    set(SUITABLE_STRUCT_NEED_BENCHMARK ON)
endif()

if (NOT ${SUITABLE_STRUCT_GTEST_SEARCH_MODE} STREQUAL "Skip")
    if (${SUITABLE_STRUCT_GTEST_SEARCH_MODE} STREQUAL "Auto")
        set(SUITABLE_STRUCT__INTERNAL_GTEST_SEARCH_MODE QUIET)
    else()
        set(SUITABLE_STRUCT__INTERNAL_GTEST_SEARCH_MODE REQUIRED)
    endif()
endif()

if (SUITABLE_STRUCT_NEED_GTEST AND NOT TARGET gtest)
    if (NOT ${SUITABLE_STRUCT_GTEST_SEARCH_MODE} STREQUAL "Skip")
        find_package(GTest ${SUITABLE_STRUCT__INTERNAL_GTEST_SEARCH_MODE})
    endif()

    if (GTest_FOUND)
        message("GTest found!")
    else()
        message("GTest not found or skipped! Adding external (web) source...")

        set(INSTALL_GTEST OFF CACHE BOOL "Do not install gtest" FORCE)
        set(INSTALL_GMOCK OFF CACHE BOOL "Do not install gmock" FORCE)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

        FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.tar.gz
        )

        FetchContent_MakeAvailable(googletest)
    endif()
endif()

if (SUITABLE_STRUCT_NEED_BENCHMARK AND NOT TARGET benchmark)
    if (NOT ${SUITABLE_STRUCT_GTEST_SEARCH_MODE} STREQUAL "Skip")
        find_package(benchmark ${SUITABLE_STRUCT__INTERNAL_GTEST_SEARCH_MODE})
    endif()

    if (benchmark_FOUND)
        message("Google benchmark found!")
    else()
        message("Google benchmark not found or skipped! Adding external (web) source...")

        set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
        set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
        set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)

        FetchContent_Declare(
            google-benchmark
            URL https://github.com/google/benchmark/archive/refs/tags/v1.8.3.tar.gz
        )

        FetchContent_MakeAvailable(google-benchmark)
    endif()
endif()

enable_testing()

if (SUITABLE_STRUCT_ENABLE_TESTS)
    add_subdirectory(test)
endif()

if (SUITABLE_STRUCT_ENABLE_BENCHMARK)
    add_subdirectory(benchmark)
endif()
