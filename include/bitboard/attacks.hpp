#ifndef CHESS_ATTACKS_HPP
#define CHESS_ATTACKS_HPP

#include <cstdint>
#include <array>
#include "../types.hpp"
#include "magic.hpp"
#include "bitboard.hpp"

namespace chess
{
    class Board;

    // Non-sliding piece attacks
    Bitboard getKnightAttacks(Square square) noexcept;
    Bitboard getKingAttacks(Square square) noexcept;

    // Pawn attacks and moves
    Bitboard getPawnAttacks(Bitboard pawns, Color color) noexcept;
    Bitboard getPawnAdvances(Bitboard pawns, Bitboard empty, bool is_white) noexcept;

    inline Bitboard getPawnAttacksToSquare(Square sq, Color attackerColor) noexcept
    {
        if (attackerColor == Color::WHITE)
        {
            return ((toBitboard(sq) & ~patterns::FILE_A) >> 7) |
                   ((toBitboard(sq) & ~patterns::FILE_H) >> 9);
        }
        else
        {
            return ((toBitboard(sq) & ~patterns::FILE_A) << 9) |
                   ((toBitboard(sq) & ~patterns::FILE_H) << 7);
        }
    }
}

#endif // CHESS_ATTACKS_HPP