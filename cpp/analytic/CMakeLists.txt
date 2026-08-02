# Closed-form prices. A library rather than part of the executable, so the tests
# can link against it — a test binary cannot link against a target with main().
add_library(deep_pricer_analytic
  "analytic.cpp"
  "analytic.h"
)

target_include_directories(deep_pricer_analytic PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(deep_pricer_analytic PRIVATE deep_pricer_warnings)
