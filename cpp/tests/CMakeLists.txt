# Tests for the C++ side.
#
# No test framework yet — the choice between GoogleTest / Catch2 / doctest is
# still open, and CLAUDE.md asks for no new third-party dependency without
# agreement. Until then these are plain CTest tests, which need nothing beyond
# CMake itself. Swapping in a framework later does not change this wiring.

# End-to-end smoke: the executable runs, exits 0 and produces its expected output.
add_test(NAME smoke_runs COMMAND deep-pricer)
set_tests_properties(smoke_runs PROPERTIES
  PASS_REGULAR_EXPRESSION "Hello CMake\\."
)
