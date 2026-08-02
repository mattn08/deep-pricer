# CMakeList.txt : C++ sources for deep-pricer.
#
add_subdirectory(analytic)

# Add source to this project's executable.
add_executable (deep-pricer "deep-pricer.cpp" "deep-pricer.h")
target_link_libraries(deep-pricer PRIVATE deep_pricer_analytic deep_pricer_warnings)

add_subdirectory(tests)

# TODO: Add install targets if needed.
