# Tests for the C++ side.
#
# GoogleTest is pulled in with FetchContent and pinned to a release tag, so a
# clean checkout and CI resolve the same source. SYSTEM keeps its headers out of
# our warning surface.

include(FetchContent)

FetchContent_Declare(googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.17.0
  GIT_SHALLOW    TRUE
  SYSTEM
)

# Match the MSVC runtime of the rest of the build; skip GMock and installation.
set(gtest_force_shared_crt ON  CACHE BOOL "" FORCE)
set(BUILD_GMOCK           OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST         OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

include(GoogleTest)

add_executable(deep-pricer-tests
  "harness_test.cpp"
  "analytic_test.cpp"
  "golden_test.cpp"
  "golden.cpp"
  "golden.h"
  "tolerance.h"
)
target_link_libraries(deep-pricer-tests PRIVATE
  GTest::gtest_main
  deep_pricer_analytic
  deep_pricer_warnings
)

# Reference tables are read from the source tree, not copied into the build tree,
# so a regenerated table takes effect without reconfiguring.
target_compile_definitions(deep-pricer-tests PRIVATE
  DEEP_PRICER_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)

# Registers each TEST() as its own CTest entry, so a failure names itself.
gtest_discover_tests(deep-pricer-tests)

# End-to-end smoke: the executable runs, exits 0 and produces its expected output.
add_test(NAME smoke_runs COMMAND deep-pricer)
set_tests_properties(smoke_runs PROPERTIES
  PASS_REGULAR_EXPRESSION "Hello CMake\\."
)
