#include "../include/bitboard/attacks.hpp"
#include "../include/bitboard/magic.hpp"
#include <algorithm>
#include <iostream>
#include <bitset>
namespace
{
    // Direction vectors for piece movements
    constexpr int kingDirections[8][2] = {
        {0, 1},   // N
        {1, 1},   // NE
        {1, 0},   // E
        {1, -1},  // SE
        {0, -1},  // S
        {-1, -1}, // SW
        {-1, 0},  // W
        {-1, 1}   // NW
    };

    constexpr int knightMoves[8][2] = {
        {2, 1}, {1, 2}, {-1, 2}, {-2, 1}, {-2, -1}, {-1, -2}, {1, -2}, {2, -1}};
}

namespace chess
{
    // Non-sliding piece attacks
    Bitboard getKnightAttacks(Square square) noexcept
    {
        const int sq = static_cast<int>(square);
        const int rank = sq >> 3;
        const int file = sq & 7;
        Bitboard attacks = 0;

        for (const auto &move : knightMoves)
        {
            int r = rank + move[0];
            int f = file + move[1];
            if (r >= 0 && r < 8 && f >= 0 && f < 8)
            {
                attacks |= 1ULL << (r * 8 + f);
            }
        }
        return attacks;
    }

    Bitboard getKingAttacks(Square square) noexcept
    {
        const int sq = static_cast<int>(square);
        const int rank = sq >> 3;
        const int file = sq & 7;
        Bitboard attacks = 0;

        for (const auto &dir : kingDirections)
        {
            int r = rank + dir[0];
            int f = file + dir[1];
            if (r >= 0 && r < 8 && f >= 0 && f < 8)
            {
                attacks |= 1ULL << (r * 8 + f);
            }
        }
        return attacks;
    }

    // Pawn attacks
    Bitboard getPawnAttacks(Bitboard pawns, Color color) noexcept
    {
        return (color == Color::WHITE)
                   ? ((pawns << 7) & ~0x8080808080808080ULL) | ((pawns << 9) & ~0x0101010101010101ULL)
                   : ((pawns >> 7) & ~0x0101010101010101ULL) | ((pawns >> 9) & ~0x8080808080808080ULL);
    }

    Bitboard getPawnAdvances(Bitboard pawns, Bitboard empty, bool is_white) noexcept
    {
        //std::cout << "Pawns before shift: " << std::bitset<64>(pawns) << std::endl;
        //std::cout << "Empty squares: " << std::bitset<64>(empty) << std::endl;
        if (is_white)
        {
            return (pawns << 8) & empty; // Only single advances
        }
        else
        {
            return (pawns >> 8) & empty; // Only single advances
        }
    }
    
}