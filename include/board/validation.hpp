#ifndef CHESS_BOARD_VALIDATION_HPP
#define CHESS_BOARD_VALIDATION_HPP

#include <cstdint>
#include "../types.hpp"
#include "../bitboard/bitboard.hpp"
#include "../movegen/move_types.hpp"
#include "state.hpp"

namespace chess
{
    class MoveValidation
    {
    public:
        [[nodiscard]] static std::vector<chess::ScoredMove> getLegalMoves(const GameState &state) noexcept;
        [[nodiscard]] static bool isLegalMove(const GameState &state, const Move &move) noexcept;
        [[nodiscard]] static bool isInCheck(const GameState &state) noexcept;
        [[nodiscard]] static bool isSquareAttacked(const GameState &state, Square square) noexcept;
        [[nodiscard]] static bool isDraw(const GameState &state, const StateManager &manager) noexcept;
        static bool isInsufficientMaterial(const GameState &state) noexcept;
        [[nodiscard]] static bool verifyPosition(const GameState &state) noexcept;

        // Move type checks remain unchanged as they only deal with Move objects
        [[nodiscard]] static constexpr bool isCapture(const Move &move) noexcept
        {
            return move.getFlags() == Move::CAPTURE_FLAG;
        }
        [[nodiscard]] static constexpr bool isCastle(const Move &move) noexcept { return move.getFlags() == 4; }
        [[nodiscard]] static constexpr bool isEnPassant(const Move &move) noexcept { return move.getFlags() == 3; }
        [[nodiscard]] static constexpr bool isPromotion(const Move &move) noexcept { return move.promotion() != Piece::EMPTY; }
        [[nodiscard]] static bool isDiscoveredCheck(const GameState &state, Square from, Square to, Square enemyKing) noexcept;

    private:
        [[nodiscard]] static bool canCastleKingside(const GameState &state, Color side) noexcept;
        [[nodiscard]] static bool canCastleQueenside(const GameState &state, Color side) noexcept;
        [[nodiscard]] static bool validateCastle(const GameState &state, const Move &move) noexcept;
        [[nodiscard]] static bool validateKingsideCastleSquares(const GameState &state, Color side) noexcept;
        [[nodiscard]] static bool validateQueensideCastleSquares(const GameState &state, Color side) noexcept;
    };

} // namespace chess

#endif // CHESS_BOARD_VALIDATION_HPP