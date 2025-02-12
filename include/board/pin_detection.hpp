#ifndef CHESS_PIN_DETECTION_HPP
#define CHESS_PIN_DETECTION_HPP

#include "types.hpp"
#include "board/board.hpp"

namespace chess
{
    class PinDetection
    {
    public:
        static Bitboard getPinnedPieces(const GameState &state, Color side) noexcept;
    };
}

#endif