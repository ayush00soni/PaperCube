# Build the core engine as a static library
add_library(papercube_core STATIC 
    src/cube.cpp
)

# Tell consumers of this library where to find the header files
target_include_directories(papercube_core PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# Tell CMake to compile this target with -fPIC for Linux compatibility
set_target_properties(papercube_core PROPERTIES POSITION_INDEPENDENT_CODE ON)