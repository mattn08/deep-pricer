# Top-level CMake project for deep-pricer.
#
# 3.25 is needed for the SYSTEM argument to FetchContent_Declare, which keeps
# third-party warnings out of our build (see cpp/tests/CMakeLists.txt).
cmake_minimum_required (VERSION 3.25)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("deep-pricer")

option(DEEPPRICER_WERROR "Treat compiler warnings as errors" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Warnings are carried on an INTERFACE target rather than in CMAKE_CXX_FLAGS so
# they apply to our targets only. A global -Werror would also be applied to
# GoogleTest's own sources, where a warning we cannot fix would break the build.
add_library(deep_pricer_warnings INTERFACE)
target_compile_options(deep_pricer_warnings INTERFACE
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall;-Wextra;-Wpedantic>
  $<$<AND:$<BOOL:${DEEPPRICER_WERROR}>,$<CXX_COMPILER_ID:MSVC>>:/WX>
  $<$<AND:$<BOOL:${DEEPPRICER_WERROR}>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-Werror>
)

enable_testing()

add_subdirectory(cpp)
