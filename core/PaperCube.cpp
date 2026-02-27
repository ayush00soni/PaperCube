// PaperCube.cpp : Defines the entry point for the application.
#include <iostream>
#include <vector>
#include <cstdint>

using BYTE = std::uint8_t;

class Cube {
private:
	int n;
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

public:
	Cube(int n) :
		n(n), 
		corners(8), 
		edges(12 * static_cast<size_t>(n - 2)),
		centers(6 * static_cast<size_t>(n - 2) * static_cast<size_t>(n - 2)) {
	}

	class State {
	private:
		std::vector<BYTE> cube;
		State(int n) : cube(6 * static_cast<size_t>(n) * n) {}
	public:
		BYTE at(int face, int row, int col) {}
		void printCube() {}
		void printFace(int face) {}

	};
};

int main()
{
	std::cout << "Hello CMake." << std::endl;
	return 0;
}
