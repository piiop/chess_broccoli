#ifndef CHESS_MAGIC_HPP
#define CHESS_MAGIC_HPP

#include <cstdint>
#include "../types.hpp"

namespace chess
{
    class Board;

    // Magic bitboard constants
    constexpr int ROOK_MAGIC_BITS = 12;
    constexpr int BISHOP_MAGIC_BITS = 9;

    // Magic lookup tables
    extern std::array<Bitboard, MAX_SQUARES> rookMagics;
    extern std::array<Bitboard, MAX_SQUARES> bishopMagics;
    extern std::array<std::array<Bitboard, 1 << ROOK_MAGIC_BITS>, MAX_SQUARES> rookAttacks;
    extern std::array<std::array<Bitboard, 1 << BISHOP_MAGIC_BITS>, MAX_SQUARES> bishopAttacks;

    // Magic bitboard core functions
    Bitboard getRookMask(Square square) noexcept;
    Bitboard getBishopMask(Square square) noexcept;
    Bitboard getRookAttacksForSquare(Square square, Bitboard blockers) noexcept;
    Bitboard getBishopAttacksForSquare(Square square, Bitboard blockers) noexcept;
    Bitboard computeRookAttacks(Square square, Bitboard blockers) noexcept;
    Bitboard computeBishopAttacks(Square square, Bitboard blockers) noexcept;
    Bitboard generateBlockers(Bitboard mask, int index) noexcept;

    // Initialization function
    void initializeMagicBitboards() noexcept;
}

#endif // CHESS_MAGIC_HPP