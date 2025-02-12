#ifndef CHESS_BOARD_PIECE_OPS_HPP
#define CHESS_BOARD_PIECE_OPS_HPP

#include <cstdint>
#include "../types.hpp"
#include "../bitboard/bitboard.hpp"
#include "../movegen/move_types.hpp"
#include "state.hpp"

namespace chess
{
    class PieceOperations
    {
    public:
        [[nodiscard]] static Piece getPiece(const GameState &state, Square square) noexcept;
        [[nodiscard]] static Bitboard getPieces(const GameState &state, Piece piece) noexcept;
        [[nodiscard]] static Bitboard getAllPieces(const GameState &state) noexcept;
        [[nodiscard]] static Bitboard getPiecesOfColor(const GameState &state, Color color) noexcept;
        [[nodiscard]] static Square getKingSquare(const GameState &state, Color color) noexcept;
        [[nodiscard]] static bool isWhitePiece(const GameState &state, Square square) noexcept;
        [[nodiscard]] static Piece getMovingPiece(const GameState &state, const Move &move) noexcept;
        [[nodiscard]] static Piece getCapturedPiece(const GameState &state, const Move &move) noexcept;

        static void setPiece(GameState &state, Square square, Piece piece) noexcept;
        static void clearSquare(GameState &state, Square square) noexcept;
        static void movePiece(GameState &state, Square from, Square to) noexcept;

        [[nodiscard]] static constexpr Piece getPieceLUT(char c) noexcept
        {
            switch (c)
            {
            case 'P':
                return Piece::W_PAWN;
            case 'N':
                return Piece::W_KNIGHT;
            case 'B':
                return Piece::W_BISHOP;
            case 'R':
                return Piece::W_ROOK;
            case 'Q':
                return Piece::W_QUEEN;
            case 'K':
                return Piece::W_KING;
            case 'p':
                return Piece::B_PAWN;
            case 'n':
                return Piece::B_KNIGHT;
            case 'b':
                return Piece::B_BISHOP;
            case 'r':
                return Piece::B_ROOK;
            case 'q':
                return Piece::B_QUEEN;
            case 'k':
                return Piece::B_KING;
            default:
                return Piece::EMPTY;
            }
        }
    };
} // namespace chess
#endif // CHESS_BOARD_PIECE_OPS_HPP