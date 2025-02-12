#ifndef CHESS_MOVE_TYPES_HPP
#define CHESS_MOVE_TYPES_HPP

#include <vector>
#include <cstdint>
#include "types.hpp"

namespace chess
{

    struct Move
    {
        uint16_t data;

        // Flag constants
        static constexpr uint8_t QUIET_MOVE = 0;     // 00
        static constexpr uint8_t CAPTURE_FLAG = 1;   // 01
        static constexpr uint8_t PROMOTION_FLAG = 2; // 10
        static constexpr uint8_t SPECIAL_FLAG = 3;   // 11
        static constexpr uint8_t CASTLING_FLAG = 4;  // 100

        constexpr Move() : data(0) {}

        constexpr Move(Square from, Square to, Color c, Piece p,
                       Piece promotion = Piece::EMPTY, uint8_t special = 0)
            : data((static_cast<uint16_t>(from) & 0x3F) |
                   ((static_cast<uint16_t>(to) & 0x3F) << 6) |
                   ((static_cast<uint16_t>(p) & 0x7) << 12) |
                   ((static_cast<uint16_t>(c) & 0x1) << 15))
        {
            if (promotion != Piece::EMPTY)
            {
                data |= (static_cast<uint16_t>(promotion) & 0x7) << 12;
                data |= (special & 0x3) << 14;
            }
        }

        constexpr explicit Move(uint16_t moveData) : data(moveData) {}

        constexpr Square from() const { return static_cast<Square>(data & 0x3F); }
        constexpr Square to() const { return static_cast<Square>((data >> 6) & 0x3F); }
        constexpr Piece piece() const { return static_cast<Piece>((data >> 12) & 0x7); }
        constexpr Color color() const { return static_cast<Color>((data >> 15) & 0x1); }
        constexpr Piece promotion() const { return static_cast<Piece>((data >> 12) & 0x7); }
        constexpr uint8_t getFlags() const { return (data >> 14) & 0x7; }

        static constexpr Piece getPawnForColor(Color side)
        {
            return (side == Color::WHITE) ? Piece::W_PAWN : Piece::B_PAWN;
        }

        constexpr bool operator==(const Move &other) const noexcept
        {
            return data == other.data;
        }
        constexpr bool operator!=(const Move &other) const noexcept
        {
            return data != other.data;
        }
    };

    struct ScoredMove
    {
        Move move;
        int16_t score;

        constexpr bool operator>(const ScoredMove &other) const noexcept
        {
            return score > other.score;
        }

        constexpr bool operator<(const ScoredMove &other) const noexcept
        {
            return score < other.score;
        }
    };

    using MoveList = std::vector<ScoredMove>;

} // namespace chess

#endif // CHESS_MOVE_TYPES_HPP