#include <papercube/cube.hpp>
#include <pybind11/iostream.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

PYBIND11_MODULE(papercube, m) {
    m.doc() = "A mathematically consistent N×N×N Rubik's Cube simulation library designed primarily for algorithmic research and development.";

    // ==========================================
    // 1. Bind Global Enums
    // ==========================================
    py::enum_<papercube::Axis>(m, "Axis", "Defines the three spatial axes for cube rotations.")
        .value("X", papercube::Axis::X)
        .value("Y", papercube::Axis::Y)
        .value("Z", papercube::Axis::Z);

    py::enum_<papercube::Direction>(m, "Direction", "Defines the rotational direction of a move.")
        .value("CCW", papercube::Direction::CCW)
        .value("CW", papercube::Direction::CW);

    // ==========================================
    // 2. Bind the Move Struct
    // ==========================================
    py::class_<papercube::Move>(m, "Move", "Represents a single, physical rotation applied to the cube.")
        .def(py::init<papercube::Axis, papercube::Direction, std::size_t>(),
            py::arg("axis"), py::arg("direction"), py::arg("layer"),
            "Constructs a move given an axis, direction, and target layer.")
        .def_readonly("axis", &papercube::Move::axis)
        .def_readonly("direction", &papercube::Move::direction)
        .def_readonly("layer", &papercube::Move::layer)
        .def_static("from_action_index", &papercube::Move::from_action_index, py::arg("action_idx"), py::arg("N"),
            "Decodes an integer action index into a specific Move structure.")
        .def_static("is_valid_action_idx", &papercube::Move::is_valid_action_idx, py::arg("action_idx"), py::arg("N"),
            "Validates if an action index is within the bounds of the cube.")
        .def("inverse", &papercube::Move::inverse,
            "Generates the inverse of the current move (reversing the direction).");

    // ==========================================
    // 3. Bind the Main Cube Class
    // ==========================================
    py::class_<papercube::Cube> cube(m, "Cube", "The core mathematical engine for simulating an N x N x N Rubik's Cube.");

    cube.def(py::init<std::size_t>(), py::arg("N"), "Constructs a solved cube of the specified dimension.")
        .def(py::init<const papercube::Cube&>(), py::arg("other"), "Deep copy constructor.")

        .def("apply_move", static_cast<void (papercube::Cube::*)(const papercube::Move&)>(&papercube::Cube::apply_move), py::arg("move"),
            "Applies a specific physical rotation to the cube's internal state.")
        .def("apply_move", static_cast<void (papercube::Cube::*)(const std::size_t&)>(&papercube::Cube::apply_move), py::arg("action_idx"),
            "Applies a physical rotation using an encoded action index.")

        .def("is_valid_move", &papercube::Cube::is_valid_move, py::arg("move"), "Validates if a specific move is legally possible.")
        .def("is_valid_action_idx", &papercube::Cube::is_valid_action_idx, py::arg("action_idx"), "Validates if an action index corresponds to a legal move.")

        .def("apply_move_sequence", static_cast<void (papercube::Cube::*)(std::span<const papercube::Move>)>(&papercube::Cube::apply_move_sequence), py::arg("move_seq"),
            "Applies a sequence of Move structures consecutively.")
        .def("apply_move_sequence", static_cast<void (papercube::Cube::*)(std::span<const std::size_t>)>(&papercube::Cube::apply_move_sequence), py::arg("action_idx_seq"),
            "Applies a sequence of encoded action indices consecutively.")

        .def("get_move_history", &papercube::Cube::get_move_history, "Retrieves the chronological sequence of all moves applied.")
        .def("reset", &papercube::Cube::reset, "Instantly resets the cube to its solved state and clears the move history.")
        .def("scramble", &papercube::Cube::scramble, py::arg("num"), "Randomly applies a specified number of valid moves.")
        .def("state", &papercube::Cube::state, "Generates an immutable, hashable snapshot of the cube's current configuration.")
        .def("size", &papercube::Cube::size, "Retrieves the spatial dimension (N) of the cube.");

    // ==========================================
    // 4. Bind Nested Enums (Color & Face)
    // ==========================================
    py::enum_<papercube::Cube::Color>(cube, "Color", "Represents the standard colors mapped to the cube's faces.")
        .value("WHITE", papercube::Cube::Color::WHITE)
        .value("BLUE", papercube::Cube::Color::BLUE)
        .value("ORANGE", papercube::Cube::Color::ORANGE)
        .value("YELLOW", papercube::Cube::Color::YELLOW)
        .value("GREEN", papercube::Cube::Color::GREEN)
        .value("RED", papercube::Cube::Color::RED);

    py::enum_<papercube::Cube::Face>(cube, "Face", "Represents the physical faces of the cube.")
        .value("UP", papercube::Cube::Face::UP)
        .value("FRONT", papercube::Cube::Face::FRONT)
        .value("RIGHT", papercube::Cube::Face::RIGHT)
        .value("DOWN", papercube::Cube::Face::DOWN)
        .value("BACK", papercube::Cube::Face::BACK)
        .value("LEFT", papercube::Cube::Face::LEFT);

    // ==========================================
    // 5. Bind Nested State Class
    // ==========================================
    py::class_<papercube::Cube::State>(cube, "State", "An immutable, hashable snapshot of a specific cube configuration.")
       .def("at", static_cast<papercube::Cube::Color (papercube::Cube::State::*)(papercube::Cube::Face, std::size_t, std::size_t) const>(&papercube::Cube::State::at), 
            py::arg("face"), py::arg("row"), py::arg("col"),
            "Looks up the color enum at a specific coordinate on the cube.")
        .def("get_raw_data", &papercube::Cube::State::get_raw_data, "Exposes the underlying flattened 1D array of color bytes.")
        .def("is_solved", &papercube::Cube::State::is_solved, "Evaluates whether this specific state represents a solved cube.")

        .def("print_face", &papercube::Cube::State::print_face, py::arg("face"),
            py::call_guard<py::scoped_ostream_redirect>(),
            "Outputs a single face of the cube to standard output. Expects a papercube.Cube.Face enum.")
        .def("print", &papercube::Cube::State::print,
            py::call_guard<py::scoped_ostream_redirect>(),
            "Outputs the complete layout of all 6 faces to standard output.")

        .def("size", &papercube::Cube::State::size, "Retrieves the spatial dimension (N) of the cube state.")
        .def("__eq__", [](const papercube::Cube::State& a, const papercube::Cube::State& b) { return a == b; },
            "Checks if two cube states are identical.")
        .def("__hash__", [](const papercube::Cube::State& s) { return std::hash<papercube::Cube::State>{}(s); },
            "Computes the hash of the state, enabling its use in Python sets and as dictionary keys.");
}