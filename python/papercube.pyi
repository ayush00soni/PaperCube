"""
A mathematically consistent N×N×N Rubik's Cube simulation library designed primarily for algorithmic research and development.
"""
from __future__ import annotations
import typing
__all__: list[str] = ['Axis', 'Cube', 'Direction', 'Move']
class Axis:
    """
    Defines the three spatial axes for cube rotations.
    
    Members:
    
      X
    
      Y
    
      Z
    """
    X: typing.ClassVar[Axis]  # value = <Axis.X: 0>
    Y: typing.ClassVar[Axis]  # value = <Axis.Y: 1>
    Z: typing.ClassVar[Axis]  # value = <Axis.Z: 2>
    __members__: typing.ClassVar[dict[str, Axis]]  # value = {'X': <Axis.X: 0>, 'Y': <Axis.Y: 1>, 'Z': <Axis.Z: 2>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: int) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: int) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class Cube:
    """
    The core mathematical engine for simulating an N x N x N Rubik's Cube.
    """
    class Color:
        """
        Represents the standard colors mapped to the cube's faces.
        
        Members:
        
          WHITE
        
          BLUE
        
          ORANGE
        
          YELLOW
        
          GREEN
        
          RED
        """
        BLUE: typing.ClassVar[Cube.Color]  # value = <Color.BLUE: 1>
        GREEN: typing.ClassVar[Cube.Color]  # value = <Color.GREEN: 4>
        ORANGE: typing.ClassVar[Cube.Color]  # value = <Color.ORANGE: 2>
        RED: typing.ClassVar[Cube.Color]  # value = <Color.RED: 5>
        WHITE: typing.ClassVar[Cube.Color]  # value = <Color.WHITE: 0>
        YELLOW: typing.ClassVar[Cube.Color]  # value = <Color.YELLOW: 3>
        __members__: typing.ClassVar[dict[str, Cube.Color]]  # value = {'WHITE': <Color.WHITE: 0>, 'BLUE': <Color.BLUE: 1>, 'ORANGE': <Color.ORANGE: 2>, 'YELLOW': <Color.YELLOW: 3>, 'GREEN': <Color.GREEN: 4>, 'RED': <Color.RED: 5>}
        def __eq__(self, other: typing.Any) -> bool:
            ...
        def __getstate__(self) -> int:
            ...
        def __hash__(self) -> int:
            ...
        def __index__(self) -> int:
            ...
        def __init__(self, value: int) -> None:
            ...
        def __int__(self) -> int:
            ...
        def __ne__(self, other: typing.Any) -> bool:
            ...
        def __repr__(self) -> str:
            ...
        def __setstate__(self, state: int) -> None:
            ...
        def __str__(self) -> str:
            ...
        @property
        def name(self) -> str:
            ...
        @property
        def value(self) -> int:
            ...
    class Face:
        """
        Represents the physical faces of the cube.
        
        Members:
        
          UP
        
          FRONT
        
          RIGHT
        
          DOWN
        
          BACK
        
          LEFT
        """
        BACK: typing.ClassVar[Cube.Face]  # value = <Face.BACK: 4>
        DOWN: typing.ClassVar[Cube.Face]  # value = <Face.DOWN: 3>
        FRONT: typing.ClassVar[Cube.Face]  # value = <Face.FRONT: 1>
        LEFT: typing.ClassVar[Cube.Face]  # value = <Face.LEFT: 5>
        RIGHT: typing.ClassVar[Cube.Face]  # value = <Face.RIGHT: 2>
        UP: typing.ClassVar[Cube.Face]  # value = <Face.UP: 0>
        __members__: typing.ClassVar[dict[str, Cube.Face]]  # value = {'UP': <Face.UP: 0>, 'FRONT': <Face.FRONT: 1>, 'RIGHT': <Face.RIGHT: 2>, 'DOWN': <Face.DOWN: 3>, 'BACK': <Face.BACK: 4>, 'LEFT': <Face.LEFT: 5>}
        def __eq__(self, other: typing.Any) -> bool:
            ...
        def __getstate__(self) -> int:
            ...
        def __hash__(self) -> int:
            ...
        def __index__(self) -> int:
            ...
        def __init__(self, value: int) -> None:
            ...
        def __int__(self) -> int:
            ...
        def __ne__(self, other: typing.Any) -> bool:
            ...
        def __repr__(self) -> str:
            ...
        def __setstate__(self, state: int) -> None:
            ...
        def __str__(self) -> str:
            ...
        @property
        def name(self) -> str:
            ...
        @property
        def value(self) -> int:
            ...
    class State:
        """
        An immutable, hashable snapshot of a specific cube configuration.
        """
        def __eq__(self, arg0: Cube.State) -> bool:
            """
            Checks if two cube states are identical.
            """
        def __hash__(self) -> int:
            """
            Computes the hash of the state, enabling its use in Python sets and as dictionary keys.
            """
        def at(self, face: Cube.Face, row: int, col: int) -> Cube.Color:
            """
            Looks up the color enum at a specific coordinate on the cube.
            """
        def get_raw_data(self) -> list[int]:
            """
            Exposes the underlying flattened 1D array of color bytes.
            """
        def is_solved(self) -> bool:
            """
            Evaluates whether this specific state represents a solved cube.
            """
        def print(self) -> None:
            """
            Outputs the complete layout of all 6 faces to standard output.
            """
        def print_face(self, face: Cube.Face) -> None:
            """
            Outputs a single face of the cube to standard output. Expects a papercube.Cube.Face enum.
            """
        def size(self) -> int:
            """
            Retrieves the spatial dimension (N) of the cube state.
            """
    @staticmethod
    @typing.overload
    def apply_move_sequence(*args, **kwargs) -> None:
        """
        Applies a sequence of Move structures consecutively.
        """
    @staticmethod
    @typing.overload
    def apply_move_sequence(*args, **kwargs) -> None:
        """
        Applies a sequence of encoded action indices consecutively.
        """
    @typing.overload
    def __init__(self, N: int) -> None:
        """
        Constructs a solved cube of the specified dimension.
        """
    @typing.overload
    def __init__(self, other: Cube) -> None:
        """
        Deep copy constructor.
        """
    @typing.overload
    def apply_move(self, move: Move) -> None:
        """
        Applies a specific physical rotation to the cube's internal state.
        """
    @typing.overload
    def apply_move(self, action_idx: int) -> None:
        """
        Applies a physical rotation using an encoded action index.
        """
    def get_move_history(self) -> list[Move]:
        """
        Retrieves the chronological sequence of all moves applied.
        """
    def is_valid_action_idx(self, action_idx: int) -> bool:
        """
        Validates if an action index corresponds to a legal move.
        """
    def is_valid_move(self, move: Move) -> bool:
        """
        Validates if a specific move is legally possible.
        """
    def reset(self) -> None:
        """
        Instantly resets the cube to its solved state and clears the move history.
        """
    def scramble(self, num: int) -> None:
        """
        Randomly applies a specified number of valid moves.
        """
    def size(self) -> int:
        """
        Retrieves the spatial dimension (N) of the cube.
        """
    def state(self) -> ...:
        """
        Generates an immutable, hashable snapshot of the cube's current configuration.
        """
class Direction:
    """
    Defines the rotational direction of a move.
    
    Members:
    
      CCW
    
      CW
    """
    CCW: typing.ClassVar[Direction]  # value = <Direction.CCW: 1>
    CW: typing.ClassVar[Direction]  # value = <Direction.CW: -1>
    __members__: typing.ClassVar[dict[str, Direction]]  # value = {'CCW': <Direction.CCW: 1>, 'CW': <Direction.CW: -1>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: int) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: int) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class Move:
    """
    Represents a single, physical rotation applied to the cube.
    """
    @staticmethod
    def from_action_index(action_idx: int, N: int) -> Move:
        """
        Decodes an integer action index into a specific Move structure.
        """
    @staticmethod
    def is_valid_action_idx(action_idx: int, N: int) -> bool:
        """
        Validates if an action index is within the bounds of the cube.
        """
    def __init__(self, axis: Axis, direction: Direction, layer: int) -> None:
        """
        Constructs a move given an axis, direction, and target layer.
        """
    def inverse(self) -> Move:
        """
        Generates the inverse of the current move (reversing the direction).
        """
    @property
    def axis(self) -> Axis:
        ...
    @property
    def direction(self) -> Direction:
        ...
    @property
    def layer(self) -> int:
        ...
