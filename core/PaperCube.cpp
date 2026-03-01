// PaperCube.cpp : Defines the entry point for the application.
#include <array>
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <iostream>
#include <vector>

namespace papercube {

	using BYTE = std::uint8_t;
	using SIZE = std::size_t;

	template <SIZE N>
	class Cube {
	public:
		struct Move; // Declaration for the Move struct

	private:
		static_assert(N >= 2, "Minimum size of Cube can be 2");

		static constexpr char COLORS[6] = { 'W', 'B', 'O', 'G', 'R', 'Y' };

		template <BYTE Mod>
		struct Orientation {
			BYTE val;
			explicit Orientation(int val = 0) : val((((val) % Mod) + Mod) % Mod) {}
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
			explicit Corner(BYTE color = 0, int orientation = 0) : color(color), orientation(orientation) {}
		};

		struct Edge {
			BYTE color;
			Orientation<2> orientation;
			explicit Edge(BYTE color = 0, int orientation = 0) : color(color), orientation(orientation) {}
		};

		struct Center {
			BYTE color;
			explicit Center(BYTE color = 0) : color(color) {}
		};

		std::array<Corner, 8> corners;
		std::array<Edge, 12 * (N - 2)> edges;
		std::array<Center, 6 * (N - 2) * (N - 2) > centers;

		std::vector<Move> move_history;

	public:
		Cube() {
			// TODO: Initialize corners, edges and centers array for a solved cube
		}

		// Move configuration enums
		enum class Axis : BYTE { // Axis of Move/Rotation ― X | Y | Z
			X = 0, Y = 1, Z = 2
		};

		enum class Direction : signed char { // Direction of Move/Rotation ― Clockwise | Counter-Clockwise
			CCW = 1, CW = -1
		};

		struct Move {
			Axis axis;
			Direction direction;
			SIZE layer;

			Move(Axis axis, Direction direction, SIZE layer) : axis(axis), direction(direction), layer(layer) {
				assert((layer < N) && "Layer cannot be greater that or equal to size of cube");
			}
		};

		class State {
		private:
			std::array<BYTE, 6 * N * N> stickers; // Flattened array to store the color values (indices from COLORS array) of the 6 * N * N stickers
			State(const std::array<Corner, 8>& corners,
				const std::array<Edge, 12 * (N - 2)>& edges,
				const std::array<Center, 6 * (N - 2) * (N - 2)>& centers) {
				// TODO: Write logic to covert corners, edges, and centers arrays to State object
			}
			friend class Cube;
		public:
			char at(SIZE face, SIZE row, SIZE col) const {
				assert((face < 6) && "Value of face cannot be greater than or equal to 6");
				assert((row < N) && "Value of row cannot be greater that or equal to N");
				assert((col < N) && "Value of column cannot be greater than or equal to N");
				return Cube<N>::COLORS[stickers[col + N * row + N * N * face]];
			}
			void print_cube() const {} // TODO: Write logic to print the complete cube, one face at a time
			void print_face(int face) const {} // TODO: Write logic to print specific face
			bool is_solved() const {} // TODO: Write logic to check if the state is solved state
		};

		void apply_move(const Move& move) {
			move_history.push_back(move);
			// TODO: Write the logic for applying the move
		}

		State get_state() const { return State(corners, edges, centers); }

		bool is_solved() const { return (this->get_state()).is_solved(); } // TODO: Try to optimize so you don't need to call get state to check if the cube is solved
	};
}

int main() {
	std::cout << "Hello PaperCube." << std::endl;
	papercube::Cube<3> cube;
	return 0;
}