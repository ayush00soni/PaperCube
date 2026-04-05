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
		const SIZE N;

		// Throw an exception if the input size N is less than 2
		static SIZE validate_size(SIZE N) {
			if (N < 2) throw std::invalid_argument("Minimum size of cube is 2");
			return N;
		}

		// Index for each face of the Cube is same as index of initial color on that face
		// Colors are arranged such that opposite faces have a distance of 3
		static constexpr std::array<char, 6> COLORS = { 'W','B','O','Y','G','R' };
		// TODO: Use an enum for colors instead of BYTE and COLORS array

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
						assert(!(is_opposite(color[i], color[j])) &&
							"Opposite faces cannot be on same corners");
						assert((color[i] != color[j]) && 
							"Two different faces cannot have same color on a corner");
					}
			}

			BYTE get_color(BYTE index) const {
				assert((index < 3) && "Index out of range!");
				BYTE result = color;
				for (int i = 0; i < index; i++) result /= 6; // Right shift in base 6
				return result % 6;
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
				assert(!(is_opposite(color[0], color[1])) &&
					"Opposite faces cannot be on same corners");
				assert((color[0] != color[1]) && 
					"Two different faces cannot have same color on a edge");
			}

			void flip() {
				this->color = ((this->color * 6) % 36 + (this->color / 6)); // (c0 c1) -> (c1 c0)
			}

			BYTE get_color(BYTE index) const {
				assert((index < 2) && "Index out of range!");
				BYTE result = color;
				for (int i = 0; i < index; i++) result /= 6;
				return result % 6;
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

		// A mapping for each corner to three faces of the cube
		static constexpr std::array<std::array<BYTE, 3>, 8> CORNER_FACE_MAP = { {
			{0,1,2},{2,3,4},{4,5,0},{0,2,4},
			{5,4,3},{3,2,1},{1,0,5},{5,3,1}
		} };
		// A mapping for each edge to three faces of the cube
		//static constexpr std::array<std::array<BYTE, 2>, 12> EDGE_FACE_MAP;

	public:
		Cube(SIZE N) :
			N(validate_size(N)),
			centers(std::make_unique<Center[]>(6 * (N - 2) * (N - 2))),
			edges(std::make_unique<Edge[]>(12 * (N - 2))) {

			// Initialize centers
			for (BYTE face = 0; face < 6; face++) {
				for (SIZE facelet = 0; facelet < (N - 2) * (N - 2); facelet++) {
					centers[(N - 2) * (N - 2) * face + facelet] = Center(face);
				}
			}

			// Initialize Edges
			for (BYTE i = 0; i < 2; i++) {
				for (BYTE j = 0; j < 6; j++) {
					for (SIZE k = 0; k < N - 2; k++) {
						this->edges[static_cast<SIZE>(6 * i + j) * (N - 2) + k] = Edge(std::array<BYTE, 2>{j, static_cast<BYTE>((i + j + 1) % 6)});
					}
				}
			}

			// Initialize Corners
			for (int i = 0; i < 8; i++) {
				this->corners[i] = Corner(CORNER_FACE_MAP[i]);
			}
		}


		struct Move {
			const Axis axis;
			const Direction direction;
			const SIZE layer;

			Move(const Axis axis, const Direction direction, const SIZE layer) : axis(axis), direction(direction), layer(layer) {}
		};

		class State {
		private:
			const SIZE N;
			const std::vector<BYTE> facelets; // Flattened array to store the color values (indices from COLORS array) of the 6 * N * N facelets

			// TODO: Write logic to covert corners, edges, and centers arrays to the flattened array (facelets)
			static std::vector<BYTE> init_facelets(
				SIZE N,
				const std::array<Corner, 8>& corners,
				const std::unique_ptr<Edge[]>& edges,
				const std::unique_ptr<Center[]>& centers
			) {
				std::vector<BYTE> stickers(6 * N * N);

				// Assign centers in the stickers array
				for (BYTE face = 0; face < 6; face++)
					for (SIZE i = 0; i < (N - 2); i++)
						for (SIZE j = 0; j < (N - 2); j++)
							stickers[(face * N * N) + (i + 1) * N + (j + 1)] = 
							centers[face * (N - 2) * (N - 2) + i * (N - 2) + j].color;

				// TODO: Assign edges to stickers array
				 
				 
				// TODO: Assign corners to stickers array
				for (int i = 0; i < 8; i++) {
					for (int j = 0; j < 3; j++) {
						// f0 is the current face in which the color has to be assigned
						const BYTE f0 = Cube::CORNER_FACE_MAP[i][j];

						// f1 and f2 are the adjacent faces to determine which corner of the face the color has to be assigned
						const BYTE f1 = Cube::CORNER_FACE_MAP[i][(j+1)%3];
						const BYTE f2 = Cube::CORNER_FACE_MAP[i][(j+2)%3];

						int x = -1, y = -1;
						// Cycle through each corner of the face to check that is the correct corner
						for (int k = 0; k < 4; k++) {
							BYTE face1 = (f0 + k + 1) % 6;
							if (Cube::is_opposite(face1, f0)) face1 = (face1 + 1) % 6;
							BYTE face2 = (face1 + 1) % 6;
							if (Cube::is_opposite(face2, f0)) face2 = (face2 + 1) % 6;
							if ((face1 == f1) && (face2 == f2)) {
								switch (k) {
								case 0:
									x = 0, y = 0;
									break;
								case 1:
									x = 0, y = N - 1;
									break;
								case 2:
									x = N - 1, y = N - 1;
									break;
								case 3:
									x = N - 1, y = 0;
									break;
								}
								break; 
							}
						}
						assert((x != -1 && y != -1) 
							&& "Corner Mapping Failed");

						// Assign the corner the correct color
						stickers[f0 * N * N + y * N + x] = corners[i].get_color(j);
					}
				}

				return stickers;
			}

			State(
				SIZE N,
				const std::array<Corner, 8>& corners,
				const std::unique_ptr<Edge[]>& edges,
				const std::unique_ptr<Center[]>& centers
			) : N(N), facelets(init_facelets(N, corners, edges, centers)) {
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
			SIZE size() const { return N; }
		};

		void apply_move(const Move& move) {
			if (!(move.layer < this->N)) throw std::invalid_argument("Cube::apply_move - Move.layer should be less than size of cube!");
			move_history.push_back(move);

			// TODO: Write the logic for applying the move

		}

		State state() const { return State(N, corners, edges, centers); }

		SIZE size() const { return N; }

		// TODO: Try to optimize so you don't need to call get state to check if the cube is solved
		bool is_solved() const { return (this->state()).is_solved(); } 
	};
}

// For testing and debugging only, should be removed in the finished project.
int main() {
	papercube::Cube c3(3);
	assert(c3.size() == 3);
	std::cout << "Hello PaperCube." << std::endl;
	try {
		papercube::Cube c1(1);
		assert(false && "Expected invalid_argument exception, but none thrown");
	}
	catch (const std::invalid_argument& e) {
		std::cout << "Cube of size 1 not created" << std::endl;
	}
	catch (...) {
		assert(false && "Wrong exception thrown");
	}
	return 0;
}