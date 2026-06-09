#pragma once

#include <array>
#include <cassert>
#include <compare>
#include <cstdint>
#include <functional>
#include <memory>
#include <span>
#include <stdexcept>
#include <string_view>
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
		// Index for each face of the Cube is same as index of initial color on that face
		// Colors are arranged such that opposite faces have a distance of 3
		static constexpr std::array<char, 6> COLOR_MAP = { 'W','B','O','Y','G','R' };

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

		/// Helper functions
		// Throw an exception if the input size N is less than 2
		static std::size_t constexpr validate_size(std::size_t N) {
			if (N < 2) throw std::invalid_argument("Minimum size of cube is 2");
			return N;
		}

		// Check if two faces are opposite or two color belong to opposite faces
		static constexpr bool is_opposite(std::uint8_t c0, std::uint8_t c1) {
			assert((c0 < 6 && c1 < 6) && "Invalid color code!");
			return (c0 == ((c1 + 3) % 6));
		}

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

		// Check if a given 2D array of facelets is of a solved Cube
		static bool facelets_solved(std::span<const std::uint8_t> facelets, std::size_t n);

		// Convert corners, edges, and centers arrays to the flattened array (facelets)
		static std::vector<std::uint8_t> get_facelets(
			std::size_t N,
			const std::array<Corner, 8>& corners,
			const std::unique_ptr<Edge[]>& edges,
			const std::unique_ptr<Center[]>& centers
		);

		// Size of cube
		const std::size_t N;

		/// Piece arrays
		std::array<Corner, 8> corners;
		std::unique_ptr<Edge[]> edges;
		std::unique_ptr<Center[]> centers;

		// To store every move made on the cube
		std::vector<Move> move_history;

	public:
		Cube(std::size_t N);

		// Deep copy constructor
		Cube(const Cube& other);

		class State {
			friend class Cube;
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

		public:
			bool operator== (const State& other) const = default;

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
			void print_face(int face) const;

			// Print the complete cube
			void print() const;

			constexpr std::size_t size() const { return N; }
		};

		/// Cube geometry configurations
		enum class Color : std::uint8_t
		{
			WHITE = 0, BLUE = 1, ORANGE = 2, YELLOW = 3, GREEN = 4, RED = 5
		};
		enum class Face : std::uint8_t
		{
			UP = 0, FRONT = 1, RIGHT = 2, DOWN = 3, BACK = 4, LEFT = 5
		};

		void apply_move(const Move& move);

		void apply_move(const std::size_t& action_idx);

		bool is_valid_move(const Move& move) const { return (move.layer < this->N); }

		bool is_valid_action_idx(std::size_t action_idx) const {
			return Move::is_valid_action_idx(action_idx, this->N);
		}

		void apply_move_sequence(std::span<const Move> move_seq);

		void apply_move_sequence(std::span<const std::size_t> action_idx_seq);

		const std::vector<Move>& get_move_history() const { return move_history; }

		void reset();

		void scramble(std::size_t num);

		State state() const { return State(N, corners, edges, centers); }

		constexpr std::size_t size() const { return N; }

	};
}

template<>
struct std::hash<papercube::Cube::State> {
	std::size_t operator()(papercube::Cube::State const& state) const noexcept {
		const auto& data = state.get_raw_data();
		// Cast the raw byte vector to a string_view for blazing fast standard hashing
		std::string_view view(reinterpret_cast<const char*>(data.data()), data.size());
		return std::hash<std::string_view>{}(view);
	}
};
