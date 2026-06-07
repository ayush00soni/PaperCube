// PaperCube.cpp : Defines the entry point for the application.
#include <array>
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <vector>

namespace papercube {

	/// Move Configurations
	// Axis of Move ― X | Y | Z
	// X (0) -> Axis from face 5 (-X, left) to face 2 (+X, right)
	// Y (1) -> Axis from face 3 (-Y, down) to face 0 (+Y, up)
	// Z (2) -> Axis from face 1 (-Z, front) to face 4 (+Z, back)
	enum class Axis : std::uint8_t { X = 0, Y = 1, Z = 2 };

	// Direction of Move ― Clockwise | Counter-Clockwise
	enum class Direction : signed char { CCW = 1, CW = -1 };

	enum class Color : std::uint8_t { WHITE = 0, BLUE = 1, ORANGE = 2, YELLOW = 3, GREEN = 4, RED = 5 };
	enum class Face : std::uint8_t { UP = 0, FRONT = 1, RIGHT = 2, DOWN = 3, BACK = 4, LEFT = 5 };

	struct Move {
		const Axis axis;
		const Direction direction;
		const std::size_t layer;

		Move(
			const Axis axis,
			const Direction direction,
			const std::size_t layer
		) : axis(axis), direction(direction), layer(layer) {
		}

		// Action index = Layer + Direction (0:CW or 1:CCW) * N + Axis * 2N 
		static Move from_action_index(std::size_t action_idx, std::size_t N) {
			if (!is_valid_action_idx(action_idx, N)) throw std::out_of_range("Action index out of range!");
			return Move(
				static_cast<Axis>(action_idx / (2 * N)),
				((action_idx / N) % 2 == 0) ? Direction::CW : Direction::CCW,
				action_idx % N
			);
		}

		static bool is_valid_action_idx(std::size_t action_idx, std::size_t N) { return action_idx < 6 * N; }

		Move inverse() {
			return Move(
				this->axis,
				(this->direction == Direction::CCW) ? Direction::CW : Direction::CCW,
				this->layer
			);
		}
	};

	class Cube {
	private:
		const std::size_t N;

		// Index for each face of the Cube is same as index of initial color on that face
		// Colors are arranged such that opposite faces have a distance of 3
		static constexpr std::array<char, 6> COLOR_MAP = { 'W','B','O','Y','G','R' };

		// Check if two faces are opposite or two color belong to opposite faces
		static constexpr bool is_opposite(std::uint8_t c0, std::uint8_t c1) {
			assert((c0 < 6 && c1 < 6) && "Invalid color code!");
			return (c0 == ((c1 + 3) % 6));
		}

		// A mapping for each corner to three faces of the cube
		static constexpr std::array<std::array<std::uint8_t, 3>, 8> CORNER_FACE_MAP = { {
			{0,1,2},{0,2,4},{0,4,5},{0,5,1},
			{3,1,5},{3,5,4},{3,4,2},{3,2,1}
		} };

		static constexpr std::array<std::array<std::uint8_t, 3>, 8> CORNER_POS_MAP = []() {
			std::array<std::array<std::uint8_t, 3>, 8> temp{};
			for (int i = 0; i < 8; i++) {
				for (int j = 0; j < 3; j++) {
					// f0 is the current face in which the color has to be assigned
					const std::uint8_t f0 = Cube::CORNER_FACE_MAP[i][j];

					// f1 and f2 are the adjacent faces to determine which corner of the face the color has to be assigned
					const std::uint8_t f1 = Cube::CORNER_FACE_MAP[i][(j + 1) % 3];
					const std::uint8_t f2 = Cube::CORNER_FACE_MAP[i][(j + 2) % 3];

					// Cycle direction
					signed char dir = ((f0 % 2) == 0) ? -1 : 1;

					std::uint8_t face1 = (f0 + ((dir == 1) ? 4 : 1)) % 6;
					// Cycle through each corner of the face to check that is the correct corner
					for (int k = 0; k < 4; k++) {
						if ((f0 == ((face1 + 3) % 6)) || face1 == f0) face1 = (face1 + 6 + dir) % 6;

						std::uint8_t face2 = (face1 + 6 + dir) % 6;
						if ((face2 == ((f0 + 3) % 6)) || face2 == f0) face2 = (face2 + 6 + dir) % 6;
						if (((face1 == f1) && (face2 == f2)) || ((face1 == f2) && (face2 == f1))) {
							temp[i][j] = k;
							break;
						}
						face1 = (face1 + 6 + dir) % 6;
					}
				}
			}
			return temp;
			}();

		// A mapping for each edge to two faces of the cube
		static constexpr std::array<std::array<std::uint8_t, 2>, 12> EDGE_FACE_MAP = []() {
			std::array<std::array<std::uint8_t, 2>, 12> temp_edges{};
			int k = 0;
			for (std::uint8_t i = 0; i < 6; i++)
				for (std::uint8_t j = 0; j < 2; j++) {
					assert(k < 12 && "EDGE_FACE_MAP initialization index out of bounds");
					temp_edges[k++] = std::array<std::uint8_t, 2>{ { i, static_cast<std::uint8_t>((i + j + 1) % 6) } };
				}
			return temp_edges;
			}();

		static constexpr std::array<std::array<std::uint8_t, 2>, 12> EDGE_POS_MAP = []() {
			std::array<std::array<std::uint8_t, 2>, 12> temp{};
			for (int i = 0; i < 12; i++) {
				for (int j = 0; j < 2; j++) {

					// f0 is the current face in which the color has to be assigned
					const std::uint8_t& f0 = Cube::EDGE_FACE_MAP[i][j];

					// f1 is the adjacent face to determine which edge of the face the color has to be assigned
					const std::uint8_t& f1 = Cube::EDGE_FACE_MAP[i][(j + 1) % 2];

					// Cycle through each edge-group of the face to check that is the correct edge-group
					std::uint8_t face1 = (f0 + 5) % 6;
					// Cycle direction
					signed char dir = ((f0 % 2) == 0) ? -1 : 1;
					for (std::uint8_t k = 0; k < 4; k++) {
						if ((f0 == ((face1 + 3) % 6)) || face1 == f0) face1 = (face1 + 6 + dir) % 6;
						if (face1 == f1) {
							temp[i][j] = k;
							break;
						}
						face1 = (face1 + 6 + dir) % 6;
					}
				}
			}
			return temp;
			}();

		// Edges are ordered such that the ordering match the direction of the axes
		// Ord 0: Edges are ordered right to left or bottom to top
		// Ord 1: Edges are ordered left to right or top to bottom
		static constexpr std::array<std::array<bool, 4>, 6> EDGE_GRP_ORD = { {
			{1, 1, 1, 1}, {1, 0, 1, 0}, {1, 1, 1, 1},
			{1, 0, 1, 0}, {1, 1, 1, 1}, {1, 0, 1, 0},
		} };

		// Faces along the axes
		static constexpr std::array<std::array<std::uint8_t, 2>, 3> AXIAL_FACE_PAIRS = []() {
			std::array<std::array<std::uint8_t, 2>, 3> temp{};
			for (std::uint8_t axis = 0; axis < 3; axis++) {
				temp[axis] = {
					static_cast<std::uint8_t>((5 - 2 * axis) % 6),
					static_cast<std::uint8_t>((8 - 2 * axis) % 6)
				};
			}
			return temp;
			} ();

		// Faces parallel to the axes
		static constexpr std::array < std::array<std::uint8_t, 4>, 3> PARALLEL_FACE_GRPS = []() {
			std::array < std::array<std::uint8_t, 4>, 3> temp{};
			for (std::uint8_t axis = 0; axis < 3; axis++) {
				const auto& axial_faces = AXIAL_FACE_PAIRS[axis];
				std::uint8_t k = 0;
				for (std::uint8_t& face : temp[axis]) {
					if ((k == axial_faces[0]) || (k == axial_faces[1]))  k++;
					face = k++;
				}
			}
			return temp;
			} ();

		/// Internal Data Structures for Cube pieces
		struct Corner {
			std::uint8_t color;
			Corner() : color(0) {}
			explicit Corner(const std::array<std::uint8_t, 3>& color) : color(36 * color[2] + 6 * color[1] + color[0]) {
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

			std::uint8_t get_color(std::uint8_t index) const {
				assert((index < 3) && "Index out of range!");
				std::uint8_t result = color;
				for (int i = 0; i < index; i++) result /= 6; // Right shift in base 6
				return result % 6;
			}

			void rotate() {

				this->color = ((this->color * 6) % 216 + (this->color / 36)); // (c0 c1 c2) -> (c2 c0 c1)
			}
		};

		struct Edge {
			std::uint8_t color;
			Edge() : color(0) {}
			explicit Edge(const std::array<std::uint8_t, 2>& color) : color(6 * color[1] + color[0]) {
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

			std::uint8_t get_color(std::uint8_t index) const {
				assert((index < 2) && "Index out of range!");
				std::uint8_t result = color;
				for (int i = 0; i < index; i++) result /= 6;
				return result % 6;
			}
		};

		struct Center {
			std::uint8_t color;
			Center() : color(0) {}
			explicit Center(std::uint8_t color) : color(color) { assert((color < 6) && "Invalid color code!"); }
		};

		/// Helper functions
		// Throw an exception if the input size N is less than 2
		static std::size_t validate_size(std::size_t N) {
			if (N < 2) throw std::invalid_argument("Minimum size of cube is 2");
			return N;
		}

		// Check if a given 2D array of facelets is of a solved Cube
		static bool facelets_solved(const std::vector<std::uint8_t>& facelets, std::size_t n) {
			for (int face = 0; face < 6; face++) // For each face
				for (int facelet = 0; facelet < n * n - 1; facelet++) // For each facelet in a face
					if (facelets[n * n * face + facelet] != facelets[n * n * face + facelet + 1])
						return false; // If current facelet != next facelet : return false;
			return true;
		}

		// Convert corners, edges, and centers arrays to the flattened array (facelets)
		static std::vector<std::uint8_t> get_facelets(
			std::size_t N,
			const std::array<Corner, 8>& corners,
			const std::unique_ptr<Edge[]>& edges,
			const std::unique_ptr<Center[]>& centers
		) {
			std::vector<std::uint8_t> stickers(6 * N * N, 0);
			// TODO: Replace these loops with faster lookup tables

			// Assign centers in the stickers array
			for (std::uint8_t face = 0; face < 6; face++)
				for (std::size_t i = 0; i < (N - 2); i++)
					for (std::size_t j = 0; j < (N - 2); j++)
						stickers[(face * N * N) + (i + 1) * N + (j + 1)] =
						centers[face * (N - 2) * (N - 2) + i * (N - 2) + j].color;

			// Assign edges to stickers array
			for (int i = 0; i < 12; i++) {
				for (int j = 0; j < 2; j++) {
					std::size_t x = N, y = N;

					switch (EDGE_POS_MAP[i][j]) {
					case 0:
						x = 1, y = 0;
						break;
					case 1:
						x = N - 1, y = 1;
						break;
					case 2:
						x = 1, y = N - 1;
						break;
					case 3:
						x = 0, y = 1;
						break;
					}
					assert((x < N && y < N && (x == 1 || y == 1)) && "Position assignment failed");
					auto& loop = (x == 1) ? x : y;
					for (loop = 1; loop < N - 1; loop++) {
						stickers[EDGE_FACE_MAP[i][j] * N * N + y * N + x] =
							edges[i * (N - 2) + ((EDGE_GRP_ORD[EDGE_FACE_MAP[i][j]][EDGE_POS_MAP[i][j]]) ?
								(loop - 1) : (N - 2 - loop))].get_color(j);
					}
				}
			}
			// Assign corners to stickers array
			for (int i = 0; i < 8; i++) {
				for (int j = 0; j < 3; j++) {
					std::size_t x = N, y = N;

					switch (CORNER_POS_MAP[i][j]) {
					case 0:
						x = 0, y = 0;
						break;
					case 1:
						x = N - 1, y = 0;
						break;
					case 2:
						x = N - 1, y = N - 1;
						break;
					case 3:
						x = 0, y = N - 1;
						break;
					}
					assert((x < N && y < N) && "Position assingnment failed");
					stickers[CORNER_FACE_MAP[i][j] * N * N + y * N + x] = corners[i].get_color(j);
				}
			}
			return stickers;
		}


		/// Piece arrays
		std::array<Corner, 8> corners;
		std::unique_ptr<Edge[]> edges;
		std::unique_ptr<Center[]> centers;

		// To store every move made on the cube
		std::vector<Move> move_history;

	public:
		Cube(std::size_t N) :
			N(validate_size(N)),
			centers(std::make_unique<Center[]>(6 * (N - 2) * (N - 2))),
			edges(std::make_unique<Edge[]>(12 * (N - 2))) {
			// Initialize centers
			for (std::uint8_t face = 0; face < 6; face++) {
				for (std::size_t facelet = 0; facelet < (N - 2) * (N - 2); facelet++) {
					centers[(N - 2) * (N - 2) * face + facelet] = Center(face);
				}
			}

			// Initialize Edges
			std::size_t k = 0;
			for (const auto& edge : EDGE_FACE_MAP) {
				for (std::size_t i = 0; i < N - 2; i++) {
					assert(k < (12 * (N - 2)) && "edges initialization, index out of bounds!");
					this->edges[k++] = Edge(edge);
				}
			}
			// Initialize Corners
			for (int i = 0; i < 8; i++) {
				this->corners[i] = Corner(CORNER_FACE_MAP[i]);
			}
		}

		// Deep copy constructor
		Cube(const Cube& other) :
			N(validate_size(other.N)),
			centers(std::make_unique<Center[]>(6 * (N - 2) * (N - 2))),
			edges(std::make_unique<Edge[]>(12 * (N - 2))),
			corners(other.corners),
			move_history(other.move_history) {
			// Copy center pieces
			std::copy(other.centers.get(), other.centers.get() + (6 * (N - 2) * (N - 2)), this->centers.get());
			// Copy edge pieces
			std::copy(other.edges.get(), other.edges.get() + (12 * (N - 2)), this->edges.get());
		}

		class State {
		private:
			const std::size_t N;
			const std::vector<std::uint8_t> facelets; // Flattened array to store the color values (indices from COLORS array) of the 6 * N * N facelets

			explicit State(
				std::size_t N,
				const std::array<Corner, 8>& corners,
				const std::unique_ptr<Edge[]>& edges,
				const std::unique_ptr<Center[]>& centers
			) : N(N), facelets(Cube::get_facelets(N, corners, edges, centers)) {
			}

			friend class Cube;
		public:
			bool operator== (const State& other) const {
				return this->N==other.N && this->facelets == other.facelets;
			}

			bool operator!= (const State& other) const {
				return !(*this == other);
			}

			char at(std::size_t face, std::size_t row, std::size_t col) const {
				assert((face < 6) && (row < N) && (col < N) && "Index out of range!");
				if (!((face < 6) && (row < N) && (col < N)))
					throw std::out_of_range("Cube::State::at - Index out of range!");
				return Cube::COLOR_MAP[facelets[col + N * row + N * N * face]];
			}

			const std::vector<std::uint8_t>& get_raw_data() const { return this->facelets; }

			bool is_solved() const {
				return Cube::facelets_solved(this->facelets, this->N);
			}

			// Print specific face
			void print_face(int face) const {
				for (std::size_t i = 0; i < N; i++) {
					for (std::size_t j = 0; j < N; j++)
						std::cout << this->at(face, i, j) << " ";
					std::cout << std::endl;
				}
			}

			// Print the complete cube
			void print() const {
				for (std::uint8_t face = 0; face < 6; face++) {
					for (std::size_t i = 0; i < N; i++) {
						for (std::size_t j = 0; j < N; j++)
							std::cout << this->at(face, i, j) << " ";
						std::cout << std::endl;
					}
					std::cout << std::endl;
				}
			}

			std::size_t size() const { return N; }
		};

		bool is_valid_move(const Move& move) const { return (move.layer < this->N); }

		bool is_valid_action_idx(std::size_t action_idx) const {
			return Move::is_valid_action_idx(action_idx, this->N);
		}

		void apply_move(const Move& move) {
			if (!(this->is_valid_move(move))) throw std::invalid_argument("Cube::apply_move - Move.layer should be less than size of cube!");
			move_history.push_back(move);


			// Direction: Clockwise and Anti-clockwise direction is taken along the given axis using right-hand thumb rule

			const std::uint8_t axis = static_cast<std::uint8_t>(move.axis);
			const signed char dir = static_cast<signed char>(move.direction);

			// Axial faces: the two faces through which the axis is passing through
			const auto& axial_faces = AXIAL_FACE_PAIRS[axis];

			// Parallel faces: includes all 4 faces parallel to the axis of rotation			
			const auto& parallel_faces = PARALLEL_FACE_GRPS[axis];

			// End Layers
			if (move.layer == 0 || move.layer == N - 1) {
				// affected face is the face which is being rotated along the axis
				const std::uint8_t affected_face = (move.layer == 0) ? axial_faces[0] : axial_faces[1];

				/// Rotate the corners

				// Get the indices of the corners that are supposed to change the position
				const std::array<std::size_t, 4> affected_corners = [affected_face] {
					std::array<std::size_t, 4> temp_corners{ {0,0,0,0} };
					int k = 0;
					for (int i = 0; i < CORNER_FACE_MAP.size(); i++) {
						for (const auto& face : Cube::CORNER_FACE_MAP[i]) {
							if (face == affected_face) {
								temp_corners[k++] = i;
								break;
							}
						}
					}
					return temp_corners;
					} ();

				// Pointers to current corner and the next corner
				std::uint8_t next_corner_idx = 1, current_corner_idx = 0;
				// Buffer to hold the value of the corner before changing it
				auto buffer_corner = this->corners[affected_corners[current_corner_idx]];

				// Switch the positions of the corners and rotate them simultaneously
				for (std::uint8_t i = 0; i < 4; i++) {
					// Search for the correct next corner
					for (int j = 0; j < 3; j++) {
						// Find the index of common face in the next corner
						std::uint8_t common_face = 0;
						while (CORNER_FACE_MAP[affected_corners[next_corner_idx]][common_face] != affected_face)
							common_face++;

						// Get the face mapping of the current corner
						auto current_corner = CORNER_FACE_MAP[affected_corners[current_corner_idx]];

						// Store the buffer before changing it
						auto temp_corner = buffer_corner;

						// Rotate current corner and buffer corner until the indices of the common face match
						while (current_corner[common_face] != affected_face) {
							buffer_corner.rotate();
							std::rotate(current_corner.rbegin(), current_corner.rbegin() + 1, current_corner.rend());
						}

						// Color of the faces of the next corner 
						std::uint8_t face1 = current_corner[(common_face + 1) % 3],
							face2 = current_corner[(common_face + 2) % 3];

						// Determine face1 of next corner
						do {
							face1 = (face1 + dir + 6) % 6;
						} while (face1 == affected_face || face1 == (affected_face + 3) % 6);

						// Determine face2 of next corner
						do {
							face2 = (face2 + dir + 6) % 6;
						} while (face2 == affected_face || face2 == (affected_face + 3) % 6 || face1 == face2);

						// Check if the next corner is at the correct index
						if (CORNER_FACE_MAP[affected_corners[next_corner_idx]][(common_face + 1) % 3] == face1 &&
							CORNER_FACE_MAP[affected_corners[next_corner_idx]][(common_face + 2) % 3] == face2) break;

						// If not increment next corner
						next_corner_idx = (next_corner_idx + 1) % 4;
						// Restore buffer
						buffer_corner = temp_corner;
					}

					// Swap buffer corner with next corner
					assert((next_corner_idx < 4) && "Corner Index out of bounds");
					auto temp = this->corners[affected_corners[next_corner_idx]];
					this->corners[affected_corners[next_corner_idx]] = buffer_corner;
					buffer_corner = temp;

					// Update current corner idx
					current_corner_idx = next_corner_idx;
					// Increment next corner idx
					next_corner_idx = (current_corner_idx + 1) % 4;
				}

				/// Rotate the edges

				// Get indices of affected edges
				const std::array<std::size_t, 4> affected_edges = [affected_face] {
					std::array<std::size_t, 4> temp_edges{ {0,0,0,0} };
					int k = 0;
					for (int i = 0; i < EDGE_FACE_MAP.size(); i++) {
						for (const auto& face : Cube::EDGE_FACE_MAP[i]) {
							if (face == affected_face) {
								temp_edges[k++] = i;
								break;
							}
						}
					}
					return temp_edges;
					} ();

				// Buffer to hold the value of an edge grp before changing it
				std::unique_ptr<Edge[]> buffer_edge_grp = std::make_unique<Edge[]>(N - 2);
				for (int i = 0; i < N - 2; i++) {
					buffer_edge_grp[i] = this->edges[affected_edges[0] * (N - 2) + i];
				}

				// Pointer to current edge and next edge
				std::uint8_t next_edge_idx = 1, current_edge_idx = 0;

				for (std::uint8_t i = 0; i < 4; i++) {
					// To check if flip in the edge is required
					bool flip = false;

					std::uint8_t common_face_idx = 0;
					// Search for the correct next edge
					for (int j = 0; j < 3; j++) {
						// Find the index of common face in the next edge
						common_face_idx = (EDGE_FACE_MAP[affected_edges[next_edge_idx]][0] == affected_face) ? 0 : 1;

						// Get the face mapping of the current edge
						auto current_edge = EDGE_FACE_MAP[affected_edges[current_edge_idx]];


						// Flip current edge and set flip flag to true
						if (current_edge[common_face_idx] != affected_face) {
							flip = true;
							std::rotate(current_edge.begin(), current_edge.begin() + 1, current_edge.end());
						}

						// Color of the adjacent face of the next edge 
						std::uint8_t adjacent_face = current_edge[(common_face_idx + 1) % 2];

						// Determine adjacent_face of next edge
						do {
							adjacent_face = (adjacent_face + dir + 6) % 6;
						} while (adjacent_face == affected_face || adjacent_face == (affected_face + 3) % 6);

						// Check if the next edge is at the correct index
						if (EDGE_FACE_MAP[affected_edges[next_edge_idx]][(common_face_idx + 1) % 2] == adjacent_face) break;

						// If not increment next edge
						next_edge_idx = (next_edge_idx + 1) % 4;
						// Restore flip flag
						flip = false;
					}
					// Position of the next edge on the common face
					const auto& pos_next = EDGE_POS_MAP[affected_edges[next_edge_idx]][common_face_idx];
					std::uint8_t curr_common_idx = (EDGE_FACE_MAP[affected_edges[current_edge_idx]][0] == affected_face) ? 0 : 1;
					const auto pos_curr = EDGE_POS_MAP[affected_edges[current_edge_idx]][curr_common_idx];

					// If order of current edge group is not same as order of next edge group reverse the buffer
					if ((pos_curr + pos_next == 3) ^
						(EDGE_GRP_ORD[affected_face][pos_curr] != EDGE_GRP_ORD[affected_face][pos_next]))
						std::reverse(buffer_edge_grp.get(), buffer_edge_grp.get() + (N - 2));

					for (std::size_t j = 0; j < N - 2; j++) {
						assert((next_edge_idx < 4) && "Edge index out of bounds");

						// Flip if the affected edge do not match in position for buffer edge and next edge
						if (flip)
							buffer_edge_grp[j].flip();

						// Swap every edge in next edge group with edge in the buffer edge group
						std::swap(this->edges[affected_edges[next_edge_idx] * (this->N - 2) + j], buffer_edge_grp[j]);
					}

					// Update current edge idx
					current_edge_idx = next_edge_idx;
					// Increment next edge idx
					next_edge_idx = (next_edge_idx + 1) % 4;
				}

				/// Rotate the Center Pieces
				// Find the location of affected face in centers array
				const auto face_offset = affected_face * (this->N - 2) * (this->N - 2);
				// Transpose the center piece matrix
				for (int i = 0; i < this->N - 2; i++) {
					for (int j = i + 1; j < this->N - 2; j++) {
						std::swap(this->centers[face_offset + (N - 2) * i + j],
							this->centers[face_offset + (N - 2) * j + i]);
					}
				}

				// The rotation direction along the face is opposite to that along the axis for layer 0
				const auto face_dir = (move.layer == 0) ? -dir : dir;

				for (int i = 0; i < this->N - 2; i++) {
					for (int j = 0; j < (this->N - 2) / 2; j++) {
						// For CCW direction reverse columns, else reverse rows
						const auto swap_idx = ((face_dir == 1) ? ((N - 2) * (N - 3 - j) + i) : ((N - 2) * i + N - 3 - j)),
							curr_idx = ((face_dir == 1) ? ((N - 2) * j + i) : ((N - 2) * i + j));
						std::swap(this->centers[face_offset + curr_idx], this->centers[face_offset + swap_idx]);
					}
				}

			}

			// Middle layers
			else {
				/// Rotate/swap the edges
				// Get indices of affected edges
				const std::array<std::size_t, 4> affected_edges = [parallel_faces] {
					std::array<std::size_t, 4> temp_edges{ {0,0,0,0} };
					int k = 0;
					for (int i = 0; i < EDGE_FACE_MAP.size(); i++) {
						// Flag to check if this is the affected edge
						bool affectedEdge = true;
						for (const auto& face : EDGE_FACE_MAP[i])
							affectedEdge = affectedEdge &&
							(std::find(parallel_faces.begin(), parallel_faces.end(), face) != parallel_faces.end());
						// If this edge is the affected edge add it to the array
						if (affectedEdge) {
							assert((k < 4) && "Index out of bounds");
							temp_edges[k++] = i;
						}
					}
					return temp_edges;
					} ();

				// Buffer to hold the value of an edge before changing it
				auto buffer_edge = this->edges[affected_edges[0] * (N - 2) + move.layer - 1];

				// Pointer to current edge and next edge
				std::uint8_t next_edge_grp_idx = 1, current_edge_grp_idx = 0;

				for (int i = 0; i < 4; i++) {
					// Flag to check if edge flip is required
					bool flip = false;

					// Search for the correct next edge
					for (int j = 0; j < 3; j++) {
						const auto& current_edge_grp = EDGE_FACE_MAP[affected_edges[current_edge_grp_idx]];
						const auto& next_edge_grp = EDGE_FACE_MAP[affected_edges[next_edge_grp_idx]];

						// Faces of next edge
						std::uint8_t face0 = (current_edge_grp[0] + dir + 6) % 6, face1 = (current_edge_grp[1] + dir + 6) % 6;
						while (face0 == axial_faces[0] || face0 == axial_faces[1]) face0 = (face0 + dir + 6) % 6;
						while (face1 == axial_faces[0] || face1 == axial_faces[1] || face0 == face1) face1 = (face1 + dir + 6) % 6;

						// If next edge pointer matches the correct next edge
						if (face0 == next_edge_grp[0] && face1 == next_edge_grp[1]) break;

						// If next edge pointed matces the correct next edge after flipping
						if (face1 == next_edge_grp[0] && face0 == next_edge_grp[1]) {
							// Set flip flag to true
							flip = true;
							break;
						}

						// Increment next edge pointer
						next_edge_grp_idx = (next_edge_grp_idx + 1) % 4;
					}

					// Flip the buffer edge if flip flag is true
					if (flip) buffer_edge.flip();

					// Swap next edge with the buffer edge
					assert((next_edge_grp_idx < 4) && "Edge index out of bounds");
					std::swap(
						this->edges[affected_edges[next_edge_grp_idx] * (N - 2) + move.layer - 1],
						buffer_edge
					);

					// Update current edge idx
					current_edge_grp_idx = next_edge_grp_idx;
					// Increment next edge idx
					next_edge_grp_idx = (next_edge_grp_idx + 1) % 4;
				}

				/// Rotate/swap the center pieces
				// Buffer to hold the center pieces before changing them
				std::unique_ptr<Center[]> buffer_center_pieces = std::make_unique<Center[]>((this->N - 2));

				// The top face would be the face that comes before the current face
				auto curr_top_face = (parallel_faces[0] + 5) % 6;
				int i, j;
				i = j = move.layer - 1;
				// If the top face is the one in which is on the end layer (+ve side of the axis)
				// Then the non-looping index will be taken from the opposite side
				if (curr_top_face == axial_faces[1]) i = j = N - 2 - move.layer;

				// If top face is NOT axial select column else select row
				int* loop = (std::find(axial_faces.begin(), axial_faces.end(), curr_top_face) == axial_faces.end()) ? &i : &j;

				// Initialize buffer
				for (*loop = 0; *loop < N - 2; (*loop)++) {
					buffer_center_pieces[*loop] = this->centers[parallel_faces[0] * (this->N - 2) * (this->N - 2)
						+ (this->N - 2) * i + j];
				}

				std::uint8_t next_face_idx = 0, curr_face_idx = 0;
				for (int k = 0; k < 4; k++) {
					curr_face_idx = next_face_idx;
					next_face_idx = (next_face_idx + 4 + dir) % 4;
					auto& next_face = parallel_faces[next_face_idx];
					auto& curr_face = parallel_faces[curr_face_idx];
					i = j = move.layer - 1;
					auto next_top_face = (next_face + 5) % 6;
					curr_top_face = (parallel_faces[curr_face_idx] + 5) % 6;

					// If the top face is the one in which is on the end layer (+ve side of the axis)
					// Then the non-looping index will be taken from the opposite side
					if (next_top_face == axial_faces[1]) i = j = N - 2 - move.layer;

					// If top face is axial select column else select row
					loop = (std::find(axial_faces.begin(), axial_faces.end(), next_top_face) == axial_faces.end()) ? &i : &j;

					// For odd face reverse if next face IS above the current face &
					// For even face reverse if next face IS NOT above the current
					if ((curr_face % 2 == 0) ^
						(next_face == ((curr_face + 5) % 6)))
						std::reverse(buffer_center_pieces.get(), buffer_center_pieces.get() + (this->N - 2));

					// Swap buffer with next center pieces
					for (*loop = 0; *loop < N - 2; (*loop)++) {
						std::swap(
							buffer_center_pieces[*loop],
							this->centers[next_face * (this->N - 2) * (this->N - 2)
							+ (this->N - 2) * i + j]
						);
					}

				}
			}

		}

		void apply_move(const std::size_t& action_idx) {
			if (!this->is_valid_action_idx(action_idx)) throw std::out_of_range("Action index out of range!");
			this->apply_move(Move::from_action_index(action_idx, this->N));
		}

		void apply_move_sequence(const std::vector<Move>& move_seq) {
			for (const auto& move : move_seq) this->apply_move(move);
		}

		void apply_move_sequence(const std::vector<std::size_t>& action_idx_seq) {
			for (const auto& action_idx : action_idx_seq) this->apply_move(action_idx);
		}

		const std::vector<Move>& get_move_history() const { return move_history; }

		void reset() {
			this->move_history.clear();
			this->move_history.shrink_to_fit();
			// Reset centers
			for (std::uint8_t face = 0; face < 6; face++) {
				for (std::size_t facelet = 0; facelet < (N - 2) * (N - 2); facelet++) {
					centers[(N - 2) * (N - 2) * face + facelet] = Center(face);
				}
			}

			// Reset Edges
			std::size_t k = 0;
			for (const auto& edge : EDGE_FACE_MAP) {
				for (std::size_t i = 0; i < N - 2; i++) {
					assert(k < (12 * (N - 2)) && "edges initialization, index out of bounds!");
					this->edges[k++] = Edge(edge);
				}
			}
			// Reset Corners
			for (int i = 0; i < 8; i++) {
				this->corners[i] = Corner(CORNER_FACE_MAP[i]);
			}
		}

		void scramble(std::size_t num) {
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<std::size_t> dist(0, 6 * N - 1);
			for (std::size_t i = 0; i < num; i++) {
				this->apply_move(dist(gen));
			}
		}

		State state() const { return State(N, corners, edges, centers); }

		std::size_t size() const { return N; }

	};
}

// For testing and debugging only, should be removed in the finished project.
int main() {
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
	std::cout << "\nCreating Cube of Size 3" << std::endl;
	papercube::Cube c3(3);
	assert(c3.size() == 3);
	std::cout << "Cube of Size 3, created successfully!" << std::endl;

	std::cout << "\nGetting Cube State" << std::endl;
	auto c3_state = c3.state();
	std::cout << "\nState of c3:" << std::endl;
	c3_state.print();
	assert(c3_state.is_solved()); 
	std::cout << "\nFace 2 of State of c3:" << std::endl;
	c3_state.print_face(2);

	std::cout << "\nCreating Cube of Size 10" << std::endl;
	papercube::Cube c10(10);
	assert(c10.size() == 10);
	std::cout << "Cube of Size 10, created successfully!" << std::endl;

	std::cout << "\nGetting Cube State" << std::endl;
	auto c10_state = c10.state();
	assert(c10_state.is_solved()); 
	std::cout << "\nState of c10:" << std::endl;
	c10_state.print();

	std::cout << "\nCreating Cube of Size 4" << std::endl;
	papercube::Cube c4(4);
	assert(c4.size() == 4);
	std::cout << "Cube of Size 4, created successfully!" << std::endl;
	std::cout << "\nGetting Cube State" << std::endl;
	auto c4_state = c4.state();
	assert(c4_state.is_solved()); 
	c4_state.print();
	std::cout
		<< "Test == and != operator" << std::endl
		<< "c4_state == c3_state " << (c4_state == c3_state) << std::endl
		<< "c4_state != c3_state " << (c4_state != c3_state) << std::endl;
	std::cout << "Deep Copy of c4: " << std::endl;
	auto c4_copy(c4);
	auto c4_copy_st = c4_copy.state();
	assert((c4_copy_st == c4_state) && "Deep copy failed!");
	c4_copy_st.print();

	std::cout << "Scramble c4" << std::endl;
	c4.scramble(5);
	c4.state().print();
	std::cout << "c4 move history" << std::endl;
	auto c4_move_history = c4.get_move_history();
	for (const auto& move : c4_move_history) {
		std::cout << "XYZ"[static_cast<int>(move.axis)] << " "
			<< (((static_cast<int>(move.direction) + 1) / 2) ? "CCW" : "CW") << " "
			<< move.layer << std::endl;
	}
	std::cout << "c4 move history size " << c4.get_move_history().size() << std::endl;
	std::cout << "c4 move history capacity " << c4.get_move_history().capacity() << std::endl;

	std::cout << "c4 move history inverse" << std::endl;
	std::for_each(c4_move_history.rbegin(), c4_move_history.rend(), [](auto move) {
		auto move_inv = move.inverse();
		std::cout << "XYZ"[static_cast<int>(move_inv.axis)] << " "
			<< (((static_cast<int>(move_inv.direction) + 1) / 2) ? "CCW" : "CW") << " "
			<< move_inv.layer << std::endl;
		});

	std::cout << "Test if c4_copy is changed" << std::endl
		<< "c4_copy.state() == c4.state() " << (c4_copy.state() == c4.state()) << std::endl
		<< "Print c4_copy.state()" << std::endl;
	c4_copy.state().print();

	std::cout << "Reset c4" << std::endl;
	c4.reset();
	c4.state().print();
	std::cout << "c4 move history size " << c4.get_move_history().size() << std::endl
		<< "c4 move history capacity " << c4.get_move_history().capacity() << std::endl
		<< "c4_copy.state() == c4.state() " << (c4_copy.state() == c4.state()) << std::endl;
	return 0;
}