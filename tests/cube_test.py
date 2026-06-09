import sys
import os
import pytest

# Dynamically point Python to the MSVC CMake build output directory
# This allows you to run this script from anywhere!
current_dir = os.path.dirname(__file__)
build_dir = os.path.abspath(os.path.join(current_dir, '../out/build/x64-debug/python'))
sys.path.append(build_dir)

# Now Python can find the .pyd file
import papercube

def test_cube_initialization():
    """Verify that a cube initializes correctly and is solved."""
    c3 = papercube.Cube(3)
    assert c3.size() == 3
    assert c3.state().is_solved() == True

    c10 = papercube.Cube(10)
    assert c10.size() == 10
    assert c10.state().is_solved() == True

def test_scramble_and_history():
    """Verify that scrambling works and move history is tracked."""
    c = papercube.Cube(4)
    c.scramble(5)
    
    assert c.state().is_solved() == False
    
    history = c.get_move_history()
    assert len(history) == 5

def test_reset_mechanics():
    """Verify that resetting clears the history and solves the cube."""
    c = papercube.Cube(3)
    c.scramble(10)
    assert c.state().is_solved() == False
    
    c.reset()
    assert c.state().is_solved() == True
    assert len(c.get_move_history()) == 0

def test_invalid_size_throws():
    """Verify that the C++ invalid_argument exception passes through to Python."""
    with pytest.raises(ValueError):
        # Pybind11 automatically converts std::invalid_argument to Python's ValueError
        c1 = papercube.Cube(1)