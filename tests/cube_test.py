import pytest
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

def test_order_four_property():
    """M^4 = I: Applying a 90-degree move 4 times restores the state."""
    c3 = papercube.Cube(3)
    # Using the enums we bound earlier!
    move = papercube.Move(papercube.Axis.X, papercube.Direction.CW, 0)
    
    for _ in range(4):
        c3.apply_move(move)
        
    assert c3.state().is_solved() == True

def test_move_inverse_property():
    """M * M^-1 = I: A move followed by its inverse does nothing."""
    c4 = papercube.Cube(4)
    move = papercube.Move(papercube.Axis.Y, papercube.Direction.CCW, 1)
    
    c4.apply_move(move)
    assert c4.state().is_solved() == False
    
    c4.apply_move(move.inverse())
    assert c4.state().is_solved() == True

def test_sequence_reversibility():
    """Reversing the order of an inverted sequence restores the state."""
    c3 = papercube.Cube(3)
    c3.scramble(20)
    assert c3.state().is_solved() == False
    
    history = c3.get_move_history()
    # Python's built-in reversed() perfectly handles the C++ vector Pybind gave us
    for move in reversed(history):
        c3.apply_move(move.inverse())
        
    assert c3.state().is_solved() == True

def test_state_hashing():
    """Verify that Python's hash() successfully hooks into the C++ custom hash."""
    c = papercube.Cube(3)
    state1 = c.state()
    
    c.apply_move(papercube.Move(papercube.Axis.Z, papercube.Direction.CW, 0))
    c.apply_move(papercube.Move(papercube.Axis.Z, papercube.Direction.CCW, 0))
    state2 = c.state()
    
    # The states should be identical, which means their hashes must match
    assert state1 == state2
    assert hash(state1) == hash(state2)

def test_state_printing(capsys):
    """Verify that the C++ print and print_face functions output correctly to standard out."""
    c2 = papercube.Cube(2)
    
    # 1. Test print_face() for Face 0 (WHITE)
    c2.state().print_face(papercube.Cube.Face.UP) 
    
    # Change capfd to capsys here too
    out, err = capsys.readouterr()
    
    assert "W W" in out
    assert out.count("W") == 4
    
    # 2. Test full print()
    c2.state().print()
    out, err = capsys.readouterr()
    
    assert "W W" in out
    assert "B B" in out
    assert "O O" in out
    assert "Y Y" in out
    assert "G G" in out
    assert "R R" in out
    
    assert out.count("B") == 4
    assert out.count("R") == 4