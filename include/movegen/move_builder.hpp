#ifndef MOVE_BUILDER_HPP
#define MOVE_BUILDER_HPP

#include "../types.hpp"
#include "move_types.hpp"
#include <vector>

namespace chess
{
    inline void addMove(MoveList &moves, Square from, Square to,
                        Color side, Piece piece,
                        Piece promotion = Piece::EMPTY,
                        uint8_t special = Move::QUIET_MOVE) noexcept
    {
        moves.push_back({Move(from, to, side, piece, promotion, special), 0});
    }

    inline void addPromotionMove(MoveList &moves, Square from, Square to,
                                 Color side, Piece promotion, bool isCapture = false) noexcept
    {
        uint8_t flags = Move::PROMOTION_FLAG | (isCapture ? Move::CAPTURE_FLAG : 0);
        addMove(moves, from, to, side, Move::getPawnForColor(side), promotion, flags);
    }
} // namespace chess

#endif // MOVE_BUILDER_HPP