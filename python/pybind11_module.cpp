#include <papercube/cube.hpp>
#include <pybind11/iostream.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> 

namespace py = pybind11;

PYBIND11_MODULE(papercube, m) {
    m.doc() = "Python bindings for the PaperCube C++ engine";

    // ==========================================
    // 1. Bind Global Enums
    // ==========================================
    py::enum_<papercube::Axis>(m, "Axis")
        .value("X", papercube::Axis::X)
        .value("Y", papercube::Axis::Y)
        .value("Z", papercube::Axis::Z);

    py::enum_<papercube::Direction>(m, "Direction")
        .value("CCW", papercube::Direction::CCW)
        .value("CW", papercube::Direction::CW);

    // ==========================================
    // 2. Bind the Move Struct
    // ==========================================
    py::class_<papercube::Move>(m, "Move")
        .def(py::init<papercube::Axis, papercube::Direction, std::size_t>(),
            py::arg("axis"), py::arg("direction"), py::arg("layer"))
        .def_readonly("axis", &papercube::Move::axis)
        .def_readonly("direction", &papercube::Move::direction)
        .def_readonly("layer", &papercube::Move::layer)
        .def_static("from_action_index", &papercube::Move::from_action_index, py::arg("action_idx"), py::arg("N"))
        .def_static("is_valid_action_idx", &papercube::Move::is_valid_action_idx, py::arg("action_idx"), py::arg("N"))
        .def("inverse", &papercube::Move::inverse);

    // ==========================================
    // 3. Bind the Main Cube Class
    // ==========================================
    // We store the class object in a variable 'cube' so we can nest things inside it later
    py::class_<papercube::Cube> cube(m, "Cube");

    cube.def(py::init<std::size_t>(), py::arg("N"))
        .def(py::init<const papercube::Cube&>(), py::arg("other")) // Copy constructor

        // Handling Overloaded Functions: We must explicitly cast the function pointers
        .def("apply_move", static_cast<void (papercube::Cube::*)(const papercube::Move&)>(&papercube::Cube::apply_move), py::arg("move"))
        .def("apply_move", static_cast<void (papercube::Cube::*)(const std::size_t&)>(&papercube::Cube::apply_move), py::arg("action_idx"))

        .def("is_valid_move", &papercube::Cube::is_valid_move, py::arg("move"))
        .def("is_valid_action_idx", &papercube::Cube::is_valid_action_idx, py::arg("action_idx"))

        .def("apply_move_sequence", static_cast<void (papercube::Cube::*)(std::span<const papercube::Move>)>(&papercube::Cube::apply_move_sequence), py::arg("move_seq"))
        .def("apply_move_sequence", static_cast<void (papercube::Cube::*)(std::span<const std::size_t>)>(&papercube::Cube::apply_move_sequence), py::arg("action_idx_seq"))

        .def("get_move_history", &papercube::Cube::get_move_history)
        .def("reset", &papercube::Cube::reset)
        .def("scramble", &papercube::Cube::scramble, py::arg("num"))
        .def("state", &papercube::Cube::state)
        .def("size", &papercube::Cube::size);

    // ==========================================
    // 4. Bind Nested Enums (Color & Face)
    // ==========================================
    // Notice we pass 'cube' instead of 'm' so they become papercube.Cube.Color
    py::enum_<papercube::Cube::Color>(cube, "Color")
        .value("WHITE", papercube::Cube::Color::WHITE)
        .value("BLUE", papercube::Cube::Color::BLUE)
        .value("ORANGE", papercube::Cube::Color::ORANGE)
        .value("YELLOW", papercube::Cube::Color::YELLOW)
        .value("GREEN", papercube::Cube::Color::GREEN)
        .value("RED", papercube::Cube::Color::RED);

    py::enum_<papercube::Cube::Face>(cube, "Face")
        .value("UP", papercube::Cube::Face::UP)
        .value("FRONT", papercube::Cube::Face::FRONT)
        .value("RIGHT", papercube::Cube::Face::RIGHT)
        .value("DOWN", papercube::Cube::Face::DOWN)
        .value("BACK", papercube::Cube::Face::BACK)
        .value("LEFT", papercube::Cube::Face::LEFT);

    // ==========================================
    // 5. Bind Nested State Class
    // ==========================================
    py::class_<papercube::Cube::State>(cube, "State")
        // We do NOT bind an __init__ because State objects are only created internally by Cube::state()
        .def("at", &papercube::Cube::State::at, py::arg("face"), py::arg("row"), py::arg("col"))
        .def("get_raw_data", &papercube::Cube::State::get_raw_data)
        .def("is_solved", &papercube::Cube::State::is_solved)
        // Add the call_guard to both print functions
        .def("print_face", &papercube::Cube::State::print_face, py::arg("face"),
            py::call_guard<py::scoped_ostream_redirect>())
        .def("print", &papercube::Cube::State::print,
            py::call_guard<py::scoped_ostream_redirect>())
        .def("size", &papercube::Cube::State::size)
        // Python magic methods for equality and hashing
        .def("__eq__", [](const papercube::Cube::State& a, const papercube::Cube::State& b) { return a == b; })
        .def("__hash__", [](const papercube::Cube::State& s) {
        return std::hash<papercube::Cube::State>{}(s);
            });
}