find_package(Qt6 COMPONENTS Core Gui QUIET)
if (NOT Qt6_FOUND)
    find_package(Qt5 5.15 COMPONENTS Core Gui QUIET)
endif()

set (CMAKE_AUTOMOC ON)

FILE(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.cpp)

set(PROJECT_TEST_NAME test-${PROJECT_NAME})

add_executable(${PROJECT_TEST_NAME} ${SOURCES})
target_compile_features(${PROJECT_TEST_NAME} PRIVATE cxx_std_17)

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_TEST_NAME} gtest gmock_main Threads::Threads ${PROJECT_NAME})

add_test(NAME ${PROJECT_TEST_NAME}-runner COMMAND ${PROJECT_TEST_NAME})

if(Qt5_FOUND OR Qt6_FOUND)
    target_link_libraries(${PROJECT_TEST_NAME} Qt::Core Qt::Gui)
endif()

if(MSVC)
    target_link_options(${PROJECT_TEST_NAME} PRIVATE "/ignore:4221")
    set_target_properties(${PROJECT_TEST_NAME} PROPERTIES STATIC_LIBRARY_OPTIONS "/ignore:4221")
    target_compile_options(${PROJECT_TEST_NAME} PRIVATE "/WX")
else()
    target_compile_options(${PROJECT_TEST_NAME} PRIVATE "-Werror")
endif()
