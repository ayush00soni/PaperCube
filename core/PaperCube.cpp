// PaperCube.cpp : Defines the entry point for the application.
#include <iostream>
#include <vector>
#include <cstdint>

using BYTE = std::uint8_t;

template <unsigned int N>
class Cube {
public:
	struct Move; // Declaration for the Move struct

private:
	static_assert(N >= 2, "Minimum size of Cube can be 2");

	const char COLORS[6] = { 'W', 'B', 'O', 'G', 'R', 'Y' };

	template <BYTE Mod>
	struct Orientation {
		BYTE val;
		Orientation(int val) : val((((val) % Mod) + Mod) % Mod) {}
		Orientation& operator+= (int val2) {
			val = (((val + val2) % Mod) + Mod) % Mod;
			return *this;
		}
		Orientation& operator-= (int val2) {
			val = (((val - val2) % Mod) + Mod) % Mod;
			return *this;
		}
	};

	struct Corner {
		BYTE color;
		Orientation<3> orientation;
		Corner(BYTE color, int orientation) : color(color), orientation(orientation) {}
	};

	struct Edge {
		BYTE color;
		Orientation<2> orientation;
		Edge(BYTE color, int orientation) : color(color), orientation(orientation) {}
	};

	struct Center {
		BYTE color;
		Center(BYTE color) : color(color) {}
	};

	std::vector<Corner> corners;
	std::vector<Edge> edges;
	std::vector<Center> centers;

	std::vector<Move> move_history;

public:
	Cube() :
		corners(8),
		edges(12 * (N - 2)),
		centers(6 * (N - 2) * (N - 2)) {
		// TODO: Initialize corners, edges and centers array for a solved cube
	}

	// Move configuration enums
	enum class Axis : BYTE { // Axis of Move/Rotation ― X | Y | Z
		X = 0, Y = 1, Z = 2
	};

	enum class Direction : signed char { // Direction of Move/Rotation ― Clockwise | Counter-Clockwise
		CCW = -1, CW = 1
	};

	struct Move {
		Axis axis;
		Direction direction;
		unsigned int layer;

		Move(Axis axis, Direction direction, unsigned int layer) : axis(axis), direction(direction), layer(layer% N) {}
	};

	class State {
	private:
		std::vector<BYTE> cube;
		State() : cube(6 * N * N) {}
		friend class Cube;
	public:
		BYTE at(int face, int row, int col) { return col + N * row + N * N * face; }
		void printCube() {}
		void printFace(int face) {}

	};

	void apply_move(Move move) {
		move_history.push_back(move);
		// TODO: Write the logic for applying the move
	}

	State get_state() {
		State state();
		// TODO: Write logic to covert corners, edges, and centers arrays to State object

		return state;
	}
};

int main()
{
	std::cout << "Hello PaperCube." << std::endl;
	return 0;
}
