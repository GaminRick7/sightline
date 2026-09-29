add_executable(sightline_tests
    config_test.cpp
)

target_link_libraries(sightline_tests PRIVATE sightline_shared GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(sightline_tests)
