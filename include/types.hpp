#ifndef CHESS_TYPES_HPP
#define CHESS_TYPES_HPP

#include <cstdint>
#include <utility>
#include <array>
#include <string>
namespace chess
{

    // Core type definitions
    using Bitboard = uint64_t;
    using Score = int16_t;
    using Depth = int8_t;
    
    constexpr int MAX_SQUARES = 64;
    constexpr size_t MAX_MOVES = 256;
    
    enum class Piece : uint8_t
    {
        EMPTY = 0,
        W_PAWN = 1,
        W_KNIGHT = 2,
        W_BISHOP = 3,
        W_ROOK = 4,
        W_QUEEN = 5,
        W_KING = 6,
        B_PAWN = 9, // Note the gap to allow for bit manipulation
        B_KNIGHT = 10,
        B_BISHOP = 11,
        B_ROOK = 12,
        B_QUEEN = 13,
        B_KING = 14
    };

    enum class Color : uint8_t
    {
        WHITE = 0,
        BLACK = 1
    };

    constexpr Color operator~(Color c)
    {
        return c == Color::WHITE ? Color::BLACK : Color::WHITE;
    }
    constexpr Color toColor(bool is_white) noexcept
    {
        return is_white ? Color::WHITE : Color::BLACK;
    }

    enum class Square : uint8_t
    {
        A1 = 0,
        B1 = 1,
        C1 = 2,
        D1 = 3,
        E1 = 4,
        F1 = 5,
        G1 = 6,
        H1 = 7,
        A2 = 8,
        B2 = 9,
        C2 = 10,
        D2 = 11,
        E2 = 12,
        F2 = 13,
        G2 = 14,
        H2 = 15,
        A3 = 16,
        B3 = 17,
        C3 = 18,
        D3 = 19,
        E3 = 20,
        F3 = 21,
        G3 = 22,
        H3 = 23,
        A4 = 24,
        B4 = 25,
        C4 = 26,
        D4 = 27,
        E4 = 28,
        F4 = 29,
        G4 = 30,
        H4 = 31,
        A5 = 32,
        B5 = 33,
        C5 = 34,
        D5 = 35,
        E5 = 36,
        F5 = 37,
        G5 = 38,
        H5 = 39,
        A6 = 40,
        B6 = 41,
        C6 = 42,
        D6 = 43,
        E6 = 44,
        F6 = 45,
        G6 = 46,
        H6 = 47,
        A7 = 48,
        B7 = 49,
        C7 = 50,
        D7 = 51,
        E7 = 52,
        F7 = 53,
        G7 = 54,
        H7 = 55,
        A8 = 56,
        B8 = 57,
        C8 = 58,
        D8 = 59,
        E8 = 60,
        F8 = 61,
        G8 = 62,
        H8 = 63,
        NO_SQUARE = 64
    };
    inline std::string squareToString(Square sq)
    {
        char file = 'a' + (static_cast<int>(sq) % 8);
        char rank = '1' + (static_cast<int>(sq) / 8);
        return std::string(1, file) + std::string(1, rank);
    }
    inline std::string getPieceName(Piece p)
    {
        switch (p)
        {
        case Piece::W_PAWN:
            return "P";
        case Piece::W_KNIGHT:
            return "N";
        case Piece::W_BISHOP:
            return "B";
        case Piece::W_ROOK:
            return "R";
        case Piece::W_QUEEN:
            return "Q";
        case Piece::W_KING:
            return "K";
        case Piece::B_PAWN:
            return "p";
        case Piece::B_KNIGHT:
            return "n";
        case Piece::B_BISHOP:
            return "b";
        case Piece::B_ROOK:
            return "r";
        case Piece::B_QUEEN:
            return "q";
        case Piece::B_KING:
            return "k";
        default:
            return "?";
        }
    }
    // Square helper functions
    constexpr Square operator+(Square s, int offset) noexcept
    {
        return static_cast<Square>(static_cast<uint8_t>(s) + offset);
    }

    constexpr Square &operator++(Square &s) noexcept
    {
        s = static_cast<Square>(static_cast<uint8_t>(s) + 1);
        return s;
    }

    constexpr Square operator++(Square &s, int) noexcept
    {
        Square tmp = s;
        s = static_cast<Square>(static_cast<uint8_t>(s) + 1);
        return tmp;
    }
    constexpr Square operator-(Square s, int offset) noexcept
    {
        return static_cast<Square>(static_cast<uint8_t>(s) - offset);
    }
    constexpr Square &operator+=(Square &s, int offset) noexcept
    {
        s = static_cast<Square>(static_cast<uint8_t>(s) + offset);
        return s;
    }

    constexpr Square &operator-=(Square &s, int offset) noexcept
    {
        s = static_cast<Square>(static_cast<uint8_t>(s) - offset);
        return s;
    }
    constexpr uint64_t operator<<(uint64_t value, Square square) noexcept
    {
        return value << static_cast<uint8_t>(square);
    }
    constexpr uint64_t toBitboard(Square s) noexcept
    {
        return 1ULL << static_cast<uint8_t>(s);
    }

    enum class File : uint8_t
    {
        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H
    };

    // Prefix increment
    constexpr File &operator++(File &f) noexcept
    {
        f = static_cast<File>(static_cast<int>(f) + 1);
        return f;
    }

    // Postfix increment
    constexpr File operator++(File &f, int) noexcept
    {
        File temp = f;
        f = static_cast<File>(static_cast<int>(f) + 1);
        return temp;
    }

    enum class Rank : uint8_t
    {
        RANK_1,
        RANK_2,
        RANK_3,
        RANK_4,
        RANK_5,
        RANK_6,
        RANK_7,
        RANK_8
    };

    // Common bitboard patterns
    namespace patterns
    {
        constexpr Bitboard FILE_A = 0x0101010101010101ULL;
        constexpr Bitboard FILE_B = 0x0202020202020202ULL;
        constexpr Bitboard FILE_C = 0x0404040404040404ULL;
        constexpr Bitboard FILE_D = 0x0808080808080808ULL;
        constexpr Bitboard FILE_E = 0x1010101010101010ULL;
        constexpr Bitboard FILE_F = 0x2020202020202020ULL;
        constexpr Bitboard FILE_G = 0x4040404040404040ULL;
        constexpr Bitboard FILE_H = 0x8080808080808080ULL;

        constexpr Bitboard RANK_1 = 0x00000000000000FFULL;
        constexpr Bitboard RANK_2 = 0x000000000000FF00ULL;
        constexpr Bitboard RANK_3 = 0x0000000000FF0000ULL;
        constexpr Bitboard RANK_4 = 0x00000000FF000000ULL;
        constexpr Bitboard RANK_5 = 0x000000FF00000000ULL;
        constexpr Bitboard RANK_6 = 0x0000FF0000000000ULL;
        constexpr Bitboard RANK_7 = 0x00FF000000000000ULL;
        constexpr Bitboard RANK_8 = 0xFF00000000000000ULL;
        // Center control masks
        constexpr Bitboard CENTER_SQUARES = 0x0000001818000000ULL;
        constexpr Bitboard EXTENDED_CENTER = 0x00003C3C3C3C0000ULL;
        
        // Precomputed adjacent files masks
        constexpr std::array<Bitboard, 8> ADJACENT_FILES = {
            FILE_B,
            FILE_A | FILE_C,
            FILE_B | FILE_D,
            FILE_C | FILE_E,
            FILE_D | FILE_F,
            FILE_E | FILE_G,
            FILE_F | FILE_H,
            FILE_G};
    }

} // namespace chess
#endif // CHESS_TYPES_HPP