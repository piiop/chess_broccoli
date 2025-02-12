#include "../include/bitboard/magic.hpp"
#include "../include/bitboard/core_ops.hpp"
#include <array>
#include <iostream>

namespace chess
{
    // Magic lookup table definitions
    std::array<Bitboard, MAX_SQUARES> rookMagics;
    std::array<Bitboard, MAX_SQUARES> bishopMagics;
    std::array<std::array<Bitboard, 1 << ROOK_MAGIC_BITS>, MAX_SQUARES> rookAttacks;
    std::array<std::array<Bitboard, 1 << BISHOP_MAGIC_BITS>, MAX_SQUARES> bishopAttacks;

    namespace
    {
        // Pre-computed magic numbers for rooks and bishops
        constexpr std::array<Bitboard, 64> ROOK_MAGIC_NUMBERS = {
            0x0080001020400080ULL, 0x0040001000200040ULL, 0x0080081000200080ULL, 0x0080040800100080ULL,
            0x0080020400080080ULL, 0x0080010200040080ULL, 0x0080008001000200ULL, 0x0080002040800100ULL,
            0x0000800020400080ULL, 0x0000400020005000ULL, 0x0000801000200080ULL, 0x0000800800100080ULL,
            0x0000800400080080ULL, 0x0000800200040080ULL, 0x0000800100020080ULL, 0x0000800040800100ULL,
            0x0000208000400080ULL, 0x0000404000201000ULL, 0x0000808010002000ULL, 0x0000808008001000ULL,
            0x0000808004000800ULL, 0x0000808002000400ULL, 0x0000010100020004ULL, 0x0000020000408104ULL,
            0x0000208080004000ULL, 0x0000200040005000ULL, 0x0000100080200080ULL, 0x0000080080100080ULL,
            0x0000040080080080ULL, 0x0000020080040080ULL, 0x0000010080800200ULL, 0x0000800080004100ULL,
            0x0000204000800080ULL, 0x0000200040401000ULL, 0x0000100080802000ULL, 0x0000080080801000ULL,
            0x0000040080800800ULL, 0x0000020080800400ULL, 0x0000020001010004ULL, 0x0000800040800100ULL,
            0x0000204000808000ULL, 0x0000200040008080ULL, 0x0000100020008080ULL, 0x0000080010008080ULL,
            0x0000040008008080ULL, 0x0000020004008080ULL, 0x0000010002008080ULL, 0x0000004081020004ULL,
            0x0000204000800080ULL, 0x0000200040008080ULL, 0x0000100020008080ULL, 0x0000080010008080ULL,
            0x0000040008008080ULL, 0x0000020004008080ULL, 0x0000800100020080ULL, 0x0000800041000080ULL,
            0x00FFFCDDFCED714AULL, 0x007FFCDDFCED714AULL, 0x003FFFCDFFD88096ULL, 0x0000040810002101ULL,
            0x0001000204080011ULL, 0x0001000204000801ULL, 0x0001000082000401ULL, 0x0001FFFAABFAD1A2ULL};

        constexpr std::array<Bitboard, 64> BISHOP_MAGIC_NUMBERS = {
            0x0002020202020200ULL, 0x0002020202020000ULL, 0x0004010202000000ULL, 0x0004040080000000ULL,
            0x0001104000000000ULL, 0x0000821040000000ULL, 0x0000410410400000ULL, 0x0000104104104000ULL,
            0x0000040404040400ULL, 0x0000020202020200ULL, 0x0000040102020000ULL, 0x0000040400800000ULL,
            0x0000011040000000ULL, 0x0000008210400000ULL, 0x0000004104104000ULL, 0x0000002082082000ULL,
            0x0004000808080800ULL, 0x0002000404040400ULL, 0x0001000202020200ULL, 0x0000800802004000ULL,
            0x0000800400A00000ULL, 0x0000200100884000ULL, 0x0000400082082000ULL, 0x0000200041041000ULL,
            0x0002080010101000ULL, 0x0001040008080800ULL, 0x0000208004010400ULL, 0x0000404004010200ULL,
            0x0000840000802000ULL, 0x0000404002011000ULL, 0x0000808001041000ULL, 0x0000404000820800ULL,
            0x0001041000202000ULL, 0x0000820800101000ULL, 0x0000104400080800ULL, 0x0000020080080080ULL,
            0x0000404040040100ULL, 0x0000808100020100ULL, 0x0001010100020800ULL, 0x0000808080010400ULL,
            0x0000820820004000ULL, 0x0000410410002000ULL, 0x0000082088001000ULL, 0x0000002011000800ULL,
            0x0000080100400400ULL, 0x0001010101000200ULL, 0x0002020202000400ULL, 0x0001010101000200ULL,
            0x0000410410400000ULL, 0x0000208208200000ULL, 0x0000002084100000ULL, 0x0000000020880000ULL,
            0x0000001002020000ULL, 0x0000040408020000ULL, 0x0004040404040000ULL, 0x0002020202020000ULL,
            0x0000104104104000ULL, 0x0000002082082000ULL, 0x0000000020841000ULL, 0x0000000000208800ULL,
            0x0000000010020200ULL, 0x0000000404080200ULL, 0x0000040404040400ULL, 0x0002020202020200ULL};
    }

    Bitboard getRookMask(Square square) noexcept
    {
        const int sq = static_cast<int>(square);
        const int rank = sq >> 3;
        const int file = sq & 7;
        Bitboard mask = 0;

        // Generate edges-excluded rank and file attacks
        for (int r = rank + 1; r < 7; ++r)
            mask |= (1ULL << (r * 8 + file));
        for (int r = rank - 1; r > 0; --r)
            mask |= (1ULL << (r * 8 + file));
        for (int f = file + 1; f < 7; ++f)
            mask |= (1ULL << (rank * 8 + f));
        for (int f = file - 1; f > 0; --f)
            mask |= (1ULL << (rank * 8 + f));

        return mask;
    }

    Bitboard getBishopMask(Square square) noexcept
    {
        const int sq = static_cast<int>(square);
        const int rank = sq >> 3;
        const int file = sq & 7;
        Bitboard mask = 0;

        // Generate edges-excluded diagonal attacks
        for (int r = rank + 1, f = file + 1; r < 7 && f < 7; ++r, ++f)
            mask |= (1ULL << (r * 8 + f));
        for (int r = rank + 1, f = file - 1; r < 7 && f > 0; ++r, --f)
            mask |= (1ULL << (r * 8 + f));
        for (int r = rank - 1, f = file + 1; r > 0 && f < 7; --r, ++f)
            mask |= (1ULL << (r * 8 + f));
        for (int r = rank - 1, f = file - 1; r > 0 && f > 0; --r, --f)
            mask |= (1ULL << (r * 8 + f));

        return mask;
    }

    Bitboard computeRookAttacks(Square square, Bitboard blockers) noexcept
    {
        const int sq = static_cast<int>(square);
        const int rank = sq >> 3;
        const int file = sq & 7;
        Bitboard attacks = 0;

        // Generate attacks in all four directions
        for (int r = rank + 1; r < 8; ++r)
        {
            attacks |= (1ULL << (r * 8 + file));
            if (blockers & (1ULL << (r * 8 + file)))
                break;
        }
        for (int r = rank - 1; r >= 0; --r)
        {
            attacks |= (1ULL << (r * 8 + file));
            if (blockers & (1ULL << (r * 8 + file)))
                break;
        }
        for (int f = file + 1; f < 8; ++f)
        {
            attacks |= (1ULL << (rank * 8 + f));
            if (blockers & (1ULL << (rank * 8 + f)))
                break;
        }
        for (int f = file - 1; f >= 0; --f)
        {
            attacks |= (1ULL << (rank * 8 + f));
            if (blockers & (1ULL << (rank * 8 + f)))
                break;
        }

        return attacks;
    }

    Bitboard computeBishopAttacks(Square square, Bitboard blockers) noexcept
    {
        const int sq = static_cast<int>(square);
        const int rank = sq >> 3;
        const int file = sq & 7;
        Bitboard attacks = 0;

        // Generate attacks in all four diagonal directions
        for (int r = rank + 1, f = file + 1; r < 8 && f < 8; ++r, ++f)
        {
            attacks |= (1ULL << (r * 8 + f));
            if (blockers & (1ULL << (r * 8 + f)))
                break;
        }
        for (int r = rank + 1, f = file - 1; r < 8 && f >= 0; ++r, --f)
        {
            attacks |= (1ULL << (r * 8 + f));
            if (blockers & (1ULL << (r * 8 + f)))
                break;
        }
        for (int r = rank - 1, f = file + 1; r >= 0 && f < 8; --r, ++f)
        {
            attacks |= (1ULL << (r * 8 + f));
            if (blockers & (1ULL << (r * 8 + f)))
                break;
        }
        for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; --r, --f)
        {
            attacks |= (1ULL << (r * 8 + f));
            if (blockers & (1ULL << (r * 8 + f)))
                break;
        }

        return attacks;
    }

    Bitboard generateBlockers(Bitboard mask, int index) noexcept
    {
        Bitboard blockers = 0;
        int bits = popCount(mask);
        for (int i = 0; i < bits; i++)
        {
            int bit = getLSB(mask);
            mask &= mask - 1;
            if (index & (1 << i))
            {
                blockers |= (1ULL << bit);
            }
        }
        return blockers;
    }

    Bitboard getRookAttacksForSquare(Square square, Bitboard blockers) noexcept
    {
        const int sq = static_cast<int>(square);
        // std::cout << "Debug - Rook square: " << sq << std::endl;

        blockers &= getRookMask(square);
        // std::cout << "Debug - Masked blockers: " << blockers << std::endl;

        Bitboard magicIndex = ((blockers * rookMagics[sq]) >> (64 - ROOK_MAGIC_BITS));
        // std::cout << "Debug - Magic index: " << magicIndex << std::endl;

        return rookAttacks[sq][magicIndex];
    }

    Bitboard getBishopAttacksForSquare(Square square, Bitboard blockers) noexcept
    {
        const int sq = static_cast<int>(square);
        blockers &= getBishopMask(square);
        Bitboard magicIndex = ((blockers * bishopMagics[sq]) >> (64 - BISHOP_MAGIC_BITS));
        return bishopAttacks[sq][magicIndex];
    }

    void initializeMagicBitboards() noexcept
    {
        // Initialize magic numbers
        for (int sq = 0; sq < MAX_SQUARES; ++sq)
        {
            rookMagics[sq] = ROOK_MAGIC_NUMBERS[sq];
            bishopMagics[sq] = BISHOP_MAGIC_NUMBERS[sq];
        }

        // Initialize rook attack tables
        for (int sq = 0; sq < MAX_SQUARES; ++sq)
        {
            Square square = static_cast<Square>(sq);
            Bitboard rookMask = getRookMask(square);
            int rookBits = popCount(rookMask);

            for (int index = 0; index < (1 << rookBits); ++index)
            {
                Bitboard blockers = generateBlockers(rookMask, index);
                Bitboard magicIndex = ((blockers * ROOK_MAGIC_NUMBERS[sq]) >> (64 - ROOK_MAGIC_BITS));
                rookAttacks[sq][magicIndex] = computeRookAttacks(square, blockers);
            }
        }

        // Initialize bishop attack tables
        for (int sq = 0; sq < MAX_SQUARES; ++sq)
        {
            Square square = static_cast<Square>(sq);
            Bitboard bishopMask = getBishopMask(square);
            int bishopBits = popCount(bishopMask);

            for (int index = 0; index < (1 << bishopBits); ++index)
            {
                Bitboard blockers = generateBlockers(bishopMask, index);
                Bitboard magicIndex = ((blockers * BISHOP_MAGIC_NUMBERS[sq]) >> (64 - BISHOP_MAGIC_BITS));
                bishopAttacks[sq][magicIndex] = computeBishopAttacks(square, blockers);
            }
        }
    }
} // namespace chess