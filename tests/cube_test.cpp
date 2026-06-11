#include <gtest/gtest.h>
#include <papercube/cube.hpp>
#include <stdexcept>

// Test Suite: CubeInitialization
TEST(CubeInitialization, RejectsInvalidSize) {
    // Expect the constructor to throw an invalid_argument when size is < 2
    EXPECT_THROW(papercube::Cube c1(1), std::invalid_argument);
}

TEST(CubeInitialization, CreatesSolvedCube) {
    papercube::Cube c3(3);
    EXPECT_EQ(c3.size(), 3);
    EXPECT_TRUE(c3.state().is_solved());

    papercube::Cube c10(10);
    EXPECT_EQ(c10.size(), 10);
    EXPECT_TRUE(c10.state().is_solved());
}

// Test Suite: CubeState
TEST(CubeState, StateEquality) {
    papercube::Cube c3(3);
    papercube::Cube c4(4);

    // Different sized cubes should not be equal
    EXPECT_NE(c3.state(), c4.state());
}

// Test Suite: CubeMechanics
TEST(CubeMechanics, DeepCopyMaintainsState) {
    papercube::Cube c4(4);
    c4.scramble(5); // Change the state

    // Perform deep copy
    papercube::Cube c4_copy(c4);

    // Check that states match exactly
    EXPECT_EQ(c4.state(), c4_copy.state());

    // Check that move history was copied correctly
    EXPECT_EQ(c4.get_move_history().size(), c4_copy.get_move_history().size());
}

TEST(CubeMechanics, ResetRestoresSolvedState) {
    papercube::Cube c4(4);
    c4.scramble(10);

    EXPECT_FALSE(c4.state().is_solved());
    EXPECT_GT(c4.get_move_history().size(), 0);

    c4.reset();

    EXPECT_TRUE(c4.state().is_solved());
    EXPECT_EQ(c4.get_move_history().size(), 0);
}

// Test Suite: CubeVisualization
// We use this just to visually inspect the cube in the console output
TEST(CubeVisualization, PrintScrambledState) {
    papercube::Cube c4(4);

    std::cout << "\n[--- Solved 4x4x4 Cube ---]\n";
    c4.state().print();

    std::cout << "\n[--- Scrambled 4x4x4 Cube (5 moves) ---]\n";
    c4.scramble(5);

    // Fetch and print the scramble sequence
    auto history = c4.get_move_history();
    for (const auto& move : history) {
        std::cout << "Axis: " << "XYZ"[static_cast<int>(move.axis)]
            << " | Dir: " << (((static_cast<int>(move.direction) + 1) / 2) ? "CCW" : "CW ")
            << " | Layer: " << move.layer << std::endl;
    }

    std::cout << "\n[--- Scrambled State ---]\n";
    c4.state().print();

    std::cout << "\n[--- Solution Sequence (Inverse) ---]\n";
    // Iterate backwards through the history and invert each move
    for (auto it = history.rbegin(); it != history.rend(); ++it) {
        auto inv_move = it->inverse();
        std::cout << "Axis: " << "XYZ"[static_cast<int>(inv_move.axis)]
            << " | Dir: " << (((static_cast<int>(inv_move.direction) + 1) / 2) ? "CCW" : "CW ")
            << " | Layer: " << inv_move.layer << std::endl;
    }

    // We add a basic assertion just so GTest counts it as a valid test
    EXPECT_FALSE(c4.state().is_solved());
}

// Test Suite: MathematicalConsistency
TEST(MathematicalConsistency, OrderFourProperty) {
    // Property: M^4 = I (Applying a 90-degree move 4 times restores the state)
    papercube::Cube c3(3);
    papercube::Move move(papercube::Axis::X, papercube::Direction::CW, 0);

    for (int i = 0; i < 4; i++) {
        c3.apply_move(move);
    }

    EXPECT_TRUE(c3.state().is_solved());
}

TEST(MathematicalConsistency, MoveInverseProperty) {
    // Property: M * M^-1 = I
    papercube::Cube c4(4);

    // Pick an inner layer on a 4x4 to ensure middle-layer logic is tested
    papercube::Move move(papercube::Axis::Y, papercube::Direction::CCW, 1);

    c4.apply_move(move);
    EXPECT_FALSE(c4.state().is_solved());

    c4.apply_move(move.inverse());
    EXPECT_TRUE(c4.state().is_solved());
}

TEST(MathematicalConsistency, SequenceReversibility) {
    // Property: Reversing the order of an inverted sequence restores the state
    papercube::Cube c3(3);

    // Apply 20 random moves
    c3.scramble(20);
    EXPECT_FALSE(c3.state().is_solved());

    // Retrieve the history and apply the inverse of each move in reverse order
    auto history = c3.get_move_history();
    for (auto it = history.rbegin(); it != history.rend(); ++it) {
        c3.apply_move(it->inverse());
    }

    EXPECT_TRUE(c3.state().is_solved());
}

TEST(MathematicalConsistency, ParallelLayerCommutativity) {
    // Property: A * B = B * A (Only if A and B are parallel, non-overlapping layers)
    papercube::Cube c1(3);
    papercube::Cube c2(3);

    // Left face (layer 0) and Right face (layer 2) along the X axis
    papercube::Move L(papercube::Axis::X, papercube::Direction::CW, 0);
    papercube::Move R(papercube::Axis::X, papercube::Direction::CCW, 2);

    // Engine 1 applies L then R
    c1.apply_move(L);
    c1.apply_move(R);

    // Engine 2 applies R then L
    c2.apply_move(R);
    c2.apply_move(L);

    // The resulting states must be mathematically identical
    EXPECT_EQ(c1.state(), c2.state());
}