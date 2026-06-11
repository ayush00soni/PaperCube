# Fetch Pybind11 directly from GitHub
include(FetchContent)
FetchContent_Declare(
  pybind11
  GIT_REPOSITORY https://github.com/pybind/pybind11.git
  GIT_TAG        v2.11.1
)
FetchContent_MakeAvailable(pybind11)

# Create the Python module named 'papercube'
pybind11_add_module(papercube 
    pybind11_module.cpp
)

# Link it to your core math engine
target_link_libraries(papercube PRIVATE papercube_core)

# Tell CMake to install the generated Python module
install(TARGETS papercube DESTINATION .)

# Install the IntelliSense stub file right next to it
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/papercube.pyi DESTINATION .)