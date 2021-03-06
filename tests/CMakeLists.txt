set(SUITABLE_STRUCT_NEED_GTEST OFF)
set(SUITABLE_STRUCT_NEED_BENCHMARK OFF)

if (NOT SUITABLE_STRUCT_ENABLE_TESTS AND NOT SUITABLE_STRUCT_ENABLE_BENCHMARK)
    return()
endif()

if (SUITABLE_STRUCT_ENABLE_TESTS)
    set(SUITABLE_STRUCT_NEED_GTEST ON)
endif()

if (SUITABLE_STRUCT_ENABLE_BENCHMARK)
    set(SUITABLE_STRUCT_NEED_GTEST ON)
    set(SUITABLE_STRUCT_NEED_BENCHMARK ON)
endif()

if (SUITABLE_STRUCT_NEED_GTEST AND NOT TARGET gtest)
    find_package(GTest QUIET)

    if (GTest_FOUND)
        message("GTest found!")
    else()
        message("GTest NOT found! Going to download...")

        set(INSTALL_GTEST OFF CACHE BOOL "Do not install gtest" FORCE)
        set(INSTALL_GMOCK OFF CACHE BOOL "Do not install gmock" FORCE)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

        message("Preparing GTest download 1/2")

        configure_file(CMakeLists-gtest.txt.in
            "${CMAKE_CURRENT_BINARY_DIR}/googletest-download/CMakeLists.txt")

        message("Preparing GTest download 2/2")

        execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" -S "${CMAKE_CURRENT_BINARY_DIR}/googletest-download" -B "${CMAKE_CURRENT_BINARY_DIR}/googletest-download2" RESULT_VARIABLE download_status)

        if (NOT "${download_status}" STREQUAL "0")
            message(FATAL_ERROR "Preparing GTest download - failed")
        endif()

        message("GTest download...")

        execute_process(COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_CURRENT_BINARY_DIR}/googletest-download2" RESULT_VARIABLE download_status2)

        if (NOT "${download_status2}" STREQUAL "0")
            message(FATAL_ERROR "GTest download - failed")
        endif()

        message("GTest download done")

        add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
            ${CMAKE_CURRENT_BINARY_DIR}/googletest-build)
    endif()
endif()

if (SUITABLE_STRUCT_NEED_BENCHMARK AND NOT TARGET benchmark)
    find_package(benchmark QUIET)
    
    if (benchmark_FOUND)
        message("Google benchmark found!")
    else()
        message("Google benchmark NOT found! Going to download...")

        set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
        set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
        set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)

        message("Preparing google-benchmark download 1/2")

        configure_file(CMakeLists-benchmark.txt.in
            "${CMAKE_CURRENT_BINARY_DIR}/google-benchmark-download/CMakeLists.txt")

        message("Preparing google-benchmark download 2/2")

        execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" -S "${CMAKE_CURRENT_BINARY_DIR}/google-benchmark-download" -B "${CMAKE_CURRENT_BINARY_DIR}/google-benchmark-download2" RESULT_VARIABLE download_status)

        if (NOT "${download_status}" STREQUAL "0")
            message(FATAL_ERROR "Preparing google-benchmark download - failed")
        endif()

        message("google-benchmark download...")

        execute_process(COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_CURRENT_BINARY_DIR}/google-benchmark-download2" RESULT_VARIABLE download_status2)

        if (NOT "${download_status2}" STREQUAL "0")
            message(FATAL_ERROR "google-benchmark download - failed")
        endif()

        message("google-benchmark download done")

        add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/google-benchmark-src
            ${CMAKE_CURRENT_BINARY_DIR}/google-benchmark-build)
    endif()
endif()

enable_testing()

if (SUITABLE_STRUCT_ENABLE_TESTS)
    add_subdirectory(test)
endif()

if (SUITABLE_STRUCT_ENABLE_BENCHMARK)
    add_subdirectory(benchmark)
endif()
