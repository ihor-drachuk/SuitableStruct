file(GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

foreach( testsourcefile ${SOURCES} )
    string( REPLACE ".cpp" "" testname ${testsourcefile} )

    add_executable( benchmark-${testname} ${testsourcefile} )
    target_compile_features(benchmark-${testname} PRIVATE cxx_std_17)
    target_link_libraries(benchmark-${testname} gtest benchmark SuitableStruct)

    add_test(NAME benchmark-${testname}-runner COMMAND benchmark-${testname})
endforeach( testsourcefile ${APP_SOURCES} )
