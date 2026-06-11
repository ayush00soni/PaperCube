# PaperCube

A mathematically consistent N×N×N Rubik's Cube simulation library designed primarily for algorithmic research and development.

Built in modern C++20, PaperCube provides a highly optimized, zero-allocation math engine for Reinforcement Learning (RL) and search algorithms, coupled with native Python bindings via Pybind11.

## Core API Features

The library exposes a minimal, highly optimized surface area tailored specifically for algorithmic manipulation and state space exploration:

* **`Cube::State` & High-Performance Hashing:** Provides an immutable, zero-allocation snapshot of the cube's permutation. States are inherently hashable out-of-the-box (utilizing blazing-fast `std::string_view` casting in C++ and native `__hash__` in Python). This allows state objects to be dropped directly into standard unordered maps, sets, or Python dictionaries for graph search algorithms (A*, BFS) with zero friction.
* **The `Cube` Engine:** The central mutating simulation class. It supports any physical dimension (NxNxN) and handles in-place physical rotations (`apply_move`), bulk sequence execution (`apply_move_sequence`), and rapid randomization (`scramble`).
* **`Move` & Action Space Mapping:** Represents physical rotations with strict geometric parameters (`Axis`, `Direction`, `Layer`). Includes built-in ML utility functions like `from_action_index` to seamlessly map flat 1D integer outputs (e.g., from a neural network) directly to 3D physical actions.
* **Chronological History Tracking:** The engine natively tracks every mutation applied to the cube (`get_move_history`), allowing algorithms to instantly extract the exact move sequence that led to a discovered solution state without manual bookkeeping.

## Project Structure
* `/core`: The standalone C++ library and mathematical engine.
* `/python`: Pybind11 wrapper generating the `.pyd` module.
* `/tests`: Pytest and GoogleTest suites verifying mathematical consistency and Python interop.

## Installation & Build

PaperCube can be consumed purely as a Python library for algorithmic research, or compiled from source as a C++20 library.

### Python (pip)
For Python users, PaperCube is available directly from PyPI. The installation includes pre-generated `.pyi` stubs, providing full IntelliSense and autocomplete support in modern IDEs.

```bash
pip install papercube

```

*(Note: As a source distribution, `pip` requires a C++ compiler installed on your system to build the engine locally).*

### C++ (CMake)

PaperCube uses standard CMake. The build process automatically fetches Pybind11 and GoogleTest, requiring zero manual external dependencies.

**Prerequisites:**

* A C++20 compatible compiler (MSVC, GCC, or Clang)
* CMake 3.15+

```bash
# 1. Clone the repository
git clone https://github.com/ayush00soni/PaperCube.git PaperCube
cd PaperCube

# 2. Build with CMake
mkdir build && cd build
cmake ..
cmake --build . --config Release

```

## Python Quickstart

You can import `papercube` directly into your Python scripts or Jupyter Notebooks to begin simulating state spaces.

```python
import papercube

# Initialize a standard 3x3x3 cube
c = papercube.Cube(3)

# Verify the initial state is solved
print(f"Is solved? {c.state().is_solved()}")

# Apply a sequence of 20 random valid moves
c.scramble(20)

# Check the chronological history of mutations
history = c.get_move_history()
print(f"Total moves applied: {len(history)}")

# Reset the mechanics to a solved state
c.reset()

```

## C++ Quickstart

You can easily link the core math engine (`papercube_core`) to your own C++ targets.

```cpp
#include <papercube/cube.hpp>
#include <iostream>

int main() {
    papercube::Cube c(4); // 4x4x4 Cube

    // Apply a sequence using encoded action indices
    c.apply_move(4);
    c.apply_move(12);

    if (!c.state().is_solved()) {
        std::cout << "Cube successfully scrambled.\n";
    }

    c.reset();
    return 0;
}

```

## Testing

PaperCube includes a rigorous, mathematically verified test suite across both environments.

```bash
# Run C++ GoogleTests
cd build
ctest --output-on-failure

# Run Python Pytests
cd ..
pytest tests/

```

## Upcoming Features (Future Versions)

* **GUI Visualization Layer:** A decoupled, secondary desktop application designed to read state data and visually render the N-dimensional cube. This will allow developers to animate and debug algorithmic solutions in real-time.
* **Standard Notation String Parser:** Core API support for parsing standard Singmaster move notation (e.g., `U R2 F' B D2`) directly into executable `papercube::Move` sequences.
* **Direct Piece Array Accessors**: Exposing zero-copy, read-only getters for the internal `corners`, `edges`, and `centers` arrays. This will allow advanced coordinate-based solvers (like Kociemba's two-phase algorithm) to directly query exact piece permutations and orientations, bypassing the need to decode the flattened 1D facelet state.