#ifndef CHESS_MOVEGEN_HPP
#define CHESS_MOVEGEN_HPP

#include <iostream>
#include "../types.hpp"
#include "../bitboard/bitboard.hpp"
#include "move_types.hpp"
#include "piece_moves.hpp"
#include "move_builder.hpp"

namespace chess
{

    class Board;

    // Main move generation functions
    void generatePseudoLegalMoves(MoveList &moves, const GameState &state) noexcept;
    void generateCaptures(MoveList &moves, const GameState &state, bool includeChecks = false) noexcept;
    void generateQuiescenceMoves(MoveList &moves, const GameState &state) noexcept;

} // namespace chess

#endif // CHESS_MOVEGEN_HPP