# CMakeList.txt : C++ sources for deep-pricer.
#
# Add source to this project's executable.
add_executable (deep-pricer "deep-pricer.cpp" "deep-pricer.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET deep-pricer PROPERTY CXX_STANDARD 20)
endif()

add_subdirectory(tests)

# TODO: Add install targets if needed.
