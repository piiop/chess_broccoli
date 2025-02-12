#ifndef CHESS_EVAL_HPP
#define CHESS_EVAL_HPP

#include "types.hpp"
#include "board/board.hpp"
#include "board/pst.hpp"
#include "board/state.hpp"

namespace chess
{
    // Material values (centipawns)
    constexpr Score PAWN_VALUE = 100;
    constexpr Score KNIGHT_VALUE = 320;
    constexpr Score BISHOP_VALUE = 330;
    constexpr Score ROOK_VALUE = 500;
    constexpr Score QUEEN_VALUE = 900;

    constexpr std::array<Score, 15> PIECE_VALUES = {
        0,            // EMPTY
        PAWN_VALUE,   // W_PAWN
        KNIGHT_VALUE, // W_KNIGHT
        BISHOP_VALUE, // W_BISHOP
        ROOK_VALUE,   // W_ROOK
        QUEEN_VALUE,  // W_QUEEN
        0,            // W_KING
        0, 0,         // Gap
        PAWN_VALUE,   // B_PAWN
        KNIGHT_VALUE, // B_KNIGHT
        BISHOP_VALUE, // B_BISHOP
        ROOK_VALUE,   // B_ROOK
        QUEEN_VALUE,  // B_QUEEN
        0             // B_KING
    };
    
    // Phase constants for tapered evaluation
    constexpr uint16_t TOTAL_PHASE = 24;
    constexpr uint16_t PHASE_VALUES[] = {0,    // EMPTY
                                         0,    // W_PAWN
                                         1,    // W_KNIGHT
                                         1,    // W_BISHOP
                                         2,    // W_ROOK
                                         4,    // W_QUEEN
                                         0,    // W_KING
                                         0, 0, // Gap for bit manipulation
                                         0,    // B_PAWN
                                         1,    // B_KNIGHT
                                         1,    // B_BISHOP
                                         2,    // B_ROOK
                                         4,    // B_QUEEN
                                         0};   // B_KING

    // Evaluation cache structure
    struct EvalCache
    {
        Score materialMg;
        Score materialEg;
        Score positional;
        uint16_t phase;
        bool pawnHashValid;

        EvalCache() : materialMg(0), materialEg(0), positional(0),
                      phase(TOTAL_PHASE), pawnHashValid(false) {}
    };

    // Main evaluation interface
    [[nodiscard]] Score evaluatePosition(const GameState &state) noexcept;

    namespace terms
    {
        // Pawn structure evaluation terms
        constexpr Score DOUBLED_PAWN_PENALTY_MG = -10;
        constexpr Score DOUBLED_PAWN_PENALTY_EG = -20;
        constexpr Score ISOLATED_PAWN_PENALTY_MG = -20;
        constexpr Score ISOLATED_PAWN_PENALTY_EG = -10;
        constexpr Score BACKWARD_PAWN_PENALTY = -8;
        constexpr Score PASSED_PAWN_BONUS[8] = {0, 5, 10, 20, 35, 55, 80, 0};

        // Piece mobility and positioning terms
        constexpr Score MOBILITY_BONUS[4] = {2, 3, 4, 5}; // Knight to Queen
        constexpr Score KING_SHIELD_BONUS = 5;
    }

    namespace detail
    {
        // Score interpolation
        [[nodiscard]] Score interpolateScore(Score mgScore, Score egScore, int phase) noexcept;

        // Phase calculation
        [[nodiscard]] int calculateGamePhase(const GameState &state) noexcept;

        // Core evaluation components
        [[nodiscard]] Score evaluateMaterial(const GameState &state) noexcept;
        [[nodiscard]] Score evaluatePositionalScore(const GameState &state) noexcept;
        [[nodiscard]] Score evaluatePawnStructure(const GameState &state) noexcept;
        [[nodiscard]] Score evaluateKingSafety(const GameState &state, Color side_to_evaluate) noexcept;

        // Mobility evaluation
        [[nodiscard]] Score evaluatePieceMobility(const GameState &state, Square sq, Piece pt) noexcept;

        // Pawn structure analysis
        [[nodiscard]] bool isPassedPawn(const GameState &state, Square sq, Color c) noexcept;
        [[nodiscard]] bool isIsolatedPawn(const GameState &state, Square sq) noexcept;
        [[nodiscard]] bool isBackwardPawn(const GameState &state, Square sq, Color c) noexcept;
        [[nodiscard]] bool isDoubledPawn(const GameState &state, Square sq, Color c) noexcept;

        // Evaluation helpers
        [[nodiscard]] Score evaluateKingPawnShield(const GameState &state, Square kingSquare, Color c) noexcept;
        [[nodiscard]] Score evaluatePieceActivity(const GameState &state, Color c) noexcept;
        [[nodiscard]] Score evaluateRookPlacement(const GameState &state, Square sq, Color c) noexcept;
        [[nodiscard]] Score evaluateBishopPair(const GameState &state, Color c) noexcept;
    }

} // namespace chess

#endif // CHESS_EVAL_HPP