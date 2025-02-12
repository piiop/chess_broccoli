#ifndef CHESS_CORE_OPS_HPP
#define CHESS_CORE_OPS_HPP

#include <cstdint>
#include "../types.hpp"

namespace chess
{
    using Bitboard = uint64_t;

    // Get the index of the least significant bit set
    inline int getLSB(Bitboard bb) noexcept
    {
        return __builtin_ctzll(bb); // GCC/Clang
    }

    // Clear the least significant bit
    inline Bitboard clearLSB(Bitboard bb) noexcept
    {
        return bb & (bb - 1);
    }

    // Count the number of bits set
    inline int popCount(Bitboard bb) noexcept
    {
        return __builtin_popcountll(bb); // GCC/Clang
    }
}

#endif // CHESS_CORE_OPS_HPP
