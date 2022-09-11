set (CMAKE_AUTOMOC ON)

FILE(GLOB_RECURSE SOURCES *.cpp)

set(PROJECT_TEST_NAME test-${PROJECT_NAME})

add_executable(${PROJECT_TEST_NAME} ${SOURCES})
target_compile_features(${PROJECT_TEST_NAME} PRIVATE cxx_std_17)

target_link_libraries(${PROJECT_TEST_NAME} gtest gmock_main ${PROJECT_NAME})

add_test(NAME ${PROJECT_TEST_NAME}-runner COMMAND ${PROJECT_TEST_NAME})
