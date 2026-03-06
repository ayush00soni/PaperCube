// PaperCube.cpp : Defines the entry point for the application.
#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace papercube {

	using BYTE = std::uint8_t;
	using SIZE = std::size_t;

	template <SIZE N>
	class Cube {
	public:
		// Move configuration enums
		enum class Axis : BYTE { // Axis of Move/Rotation ― X | Y | Z
			X = 0, Y = 1, Z = 2
		};

		enum class Direction : signed char { // Direction of Move/Rotation ― Clockwise | Counter-Clockwise
			CCW = 1, CW = -1
		};

		struct Move; // Declaration for the Move struct

	private:
		static_assert(N >= 2, "Minimum size of Cube can be 2");

		// Colors are arranged such that opposite faces have a distance of 3
		static constexpr char COLORS[6] = { 'W', 'R', 'B', 'Y', 'O', 'G' };

		struct Corner {
			BYTE color;
			Corner() : color(0) {}
			explicit Corner(const std::array<BYTE, 3>& color) : color(36 * color[2] + 6 * color[1] + color[0]) {
				for (int i = 0; i < 3; i++)
					assert((color[i] < 6) && "Invalid color code!");
				for (int i = 0; i < 3; i++)
					for (int j = i + 1; j < 3; j++) {
						// (|ci - cj| != 3 => (ci^2 + cj^2 - 2*ci*cj != 9), this is done to avoid negative integers
						assert(((color[i] * color[i] + color[j] * color[j] - 2 * color[i] * color[j]) !=9) && "Opposite faces cannot be on same corners");
					}
			}

			void rotate(Direction dir) {
				this->color = (dir == Direction::CCW) ?
					((this->color * 6) % 216 + (this->color / 36)) : // If dir == CCW: (c0 c1 c2) -> (c2 c0 c1)
					(36 * (this->color % 6) + (this->color / 6));    // If dir == CW: (c0 c1 c2) -> (c1 c2 c0)
			}
		};

		struct Edge {
			BYTE color;
			Edge() : color(0) {}
			explicit Edge(const std::array<BYTE, 2>& color) : color(6 * color[1] + color[0]) {
				for (int i = 0; i < 2; i++)
					assert((color[i] < 6) && "Invalid color code!");
				// (|c0 - c1| != 3 => (c0^2 + c1^2 - 2*c0*c1 != 9), this is done to avoid negative integers
				assert(((color[0] * color[0] + color[1] * color[1] - 2 * color[0] * color[1]) != 9) && "Opposite faces cannot be on same corners");
			}

			void flip() {
				this->color = ((this->color * 6) % 36 + (this->color / 6)); // (c0 c1) -> (c1 c0)
			}
		};

		struct Center {
			BYTE color;
			Center() : color(0) {}
			explicit Center(BYTE color) : color(color) { assert((color < 6) && "Invalid color code!"); }
		};

		std::array<Corner, 8> corners;
		std::array<Edge, 12 * (N - 2)> edges;
		std::array<Center, 6 * (N - 2) * (N - 2) > centers;

		std::vector<Move> move_history;

	public:
		Cube() {
			// Initialize faces
			for (int face = 0; face < 6; face++) {
				for (int facelet = 0; facelet < (N - 2) * (N - 2); facelet++) {
					centers[(N - 2) * (N - 2) * face + facelet] = Center(face);
				}
			}
			// TODO: Initialize corners and edges array for a solved cube
		}


		struct Move {
			Axis axis;
			Direction direction;
			SIZE layer;

			Move(Axis axis, Direction direction, SIZE layer) : axis(axis), direction(direction), layer(layer) {
				assert((layer < N) && "Layer index out of range!");
				if (!(layer < N)) throw std::out_of_range("Cube::Move - Layer index out of range!");
			}
		};

		class State {
		private:
			std::array<BYTE, 6 * N * N> facelets; // Flattened array to store the color values (indices from COLORS array) of the 6 * N * N facelets
			State(const std::array<Corner, 8>& corners,
				const std::array<Edge, 12 * (N - 2)>& edges,
				const std::array<Center, 6 * (N - 2) * (N - 2)>& centers) {
				// TODO: Write logic to covert corners, edges, and centers arrays to State object
			}
			friend class Cube;
		public:
			char at(SIZE face, SIZE row, SIZE col) const {
				assert((face < 6) && (row < N) && (col < N) && "Index out of range!");
				if (!((face < 6) && (row < N) && (col < N)))
					throw std::out_of_range("Cube::State::at - Index out of range!");
				return Cube<N>::COLORS[facelets[col + N * row + N * N * face]];
			}
			void print_cube() const {} // TODO: Write logic to print the complete cube, one face at a time
			void print_face(int face) const {} // TODO: Write logic to print specific face
			bool is_solved() const {
				for (int face = 0; face < 6; face++) // For each face
					for (int facelet = 0; facelet < N * N - 1; facelet++) // For each facelet in a face
						if (facelets[N * N * face + facelet] != facelets[N * N * face + facelet + 1]) 
							return false; // If current facelet != next facelet : return false;
				return true;
			}
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
	papercube::Cube<3> cube;
	std::cout << "Hello PaperCube." << std::endl;
	return 0;
}