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

	class Cube {
	public:
		// Move configuration enums
		enum class Axis : BYTE { X = 0, Y = 1, Z = 2 }; // Axis of Move/Rotation ― X | Y | Z

		// Direction of Move/Rotation ― Clockwise | Counter-Clockwise
		enum class Direction : signed char { CCW = 1, CW = -1 };

		struct Move; // Declaration for the Move struct

	private:
		SIZE N;

		// Colors are arranged such that opposite faces have a distance of 3
		static constexpr char COLORS[6] = { 'W','B','O','Y','G','R' };

		// (|ci - cj| != 3 => (ci^2 + cj^2 - 2*ci*cj != 9), this is done to avoid negative integers
		static constexpr bool is_opposite(BYTE c0, BYTE c1) {
			assert((c0 < 6 && c1 < 6) && "Invalid color code!");
			return ((c0 * c0 + c1 * c1 - 2 * c0 * c1) == 9);
		}

		struct Corner {
			BYTE color;
			Corner() : color(0) {}
			explicit Corner(const std::array<BYTE, 3>& color) : color(36 * color[2] + 6 * color[1] + color[0]) {
				for (int i = 0; i < 3; i++)
					assert((color[i] < 6) && "Invalid color code!");
				for (int i = 0; i < 3; i++)
					for (int j = i + 1; j < 3; j++) {
						// (|ci - cj| != 3 => (ci^2 + cj^2 - 2*ci*cj != 9), this is done to avoid negative integers
						assert(((color[i] * color[i] + color[j] * color[j] - 2 * color[i] * color[j]) != 9) && "Opposite faces cannot be on same corners");
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
		std::unique_ptr<Edge[]> edges;
		std::unique_ptr<Center[]> centers;

		std::vector<Move> move_history;

	public:
		Cube(SIZE N) :
			N(N),
			centers(std::make_unique<Center[]>(6 * (N - 2) * (N - 2))),
			edges(std::make_unique<Edge[]>(12 * (N - 2))) {
			if (N < 2) throw std::invalid_argument("Minimum size of cube is 2");

			// Initialize centers
			for (int face = 0; face < 6; face++) {
				for (int facelet = 0; facelet < (N - 2) * (N - 2); facelet++) {
					centers[(N - 2) * (N - 2) * face + facelet] = Center(face);
				}
			}

			// Initialize Edges
			for (BYTE i = 0; i < 2; i++) {
				for (BYTE j = 0; j < 6; j++) {
					for (SIZE k = 0; k < N - 2; k++) {
						this->edges[static_cast<SIZE>(6 * i + j) * (N - 2) + k] = Edge(std::array<BYTE, 2>{j, static_cast<BYTE>((i + j) % 6)});
					}
				}
			}

			// Initialize Corners
			static const std::array<std::array<BYTE, 3>, 8> CORNER_COLORS = { {
				{0,1,2},{2,3,4},{4,5,0},{0,2,4},
				{5,4,3},{3,2,1},{1,0,5},{5,3,1}
			} };
			for (int i = 0; i < 8; i++) {
				this->corners[i] = Corner(CORNER_COLORS[i]);
			}
		}


		struct Move {
			Axis axis;
			Direction direction;
			SIZE layer;

			Move(Axis axis, Direction direction, SIZE layer) : axis(axis), direction(direction), layer(layer) {}
		};

		class State {
		private:
			SIZE N;
			std::unique_ptr<BYTE[]> facelets; // Flattened array to store the color values (indices from COLORS array) of the 6 * N * N facelets

			State(
				SIZE N,
				const std::array<Corner, 8> &corners,
				const std::unique_ptr<Edge[]>& edges,
				const std::unique_ptr<Center[]>& centers
			) : N(N) {
				// TODO: Write logic to covert corners, edges, and centers arrays to State object
			}
			friend class Cube;
		public:
			char at(SIZE face, SIZE row, SIZE col) const {
				assert((face < 6) && (row < N) && (col < N) && "Index out of range!");
				if (!((face < 6) && (row < N) && (col < N)))
					throw std::out_of_range("Cube::State::at - Index out of range!");
				return Cube::COLORS[facelets[col + N * row + N * N * face]];
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
			if (!(move.layer < this->N)) throw std::invalid_argument("Cube::apply_move - Move.layer should be less than size of cube!");
			move_history.push_back(move);
			// TODO: Write the logic for applying the move
		}

		State get_state() const { return State(N, corners, edges, centers); }

		bool is_solved() const { return (this->get_state()).is_solved(); } // TODO: Try to optimize so you don't need to call get state to check if the cube is solved
	};
}

int main() {
	papercube::Cube cube(3);
	std::cout << "Hello PaperCube." << std::endl;
	return 0;
}