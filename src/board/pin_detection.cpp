#include "board/pin_detection.hpp"
#include "board/board.hpp"
#include "board/piece_ops.hpp"
#include "../include/bitboard/magic.hpp"

namespace chess
{

    Bitboard PinDetection::getPinnedPieces(const GameState &state, Color side) noexcept
    {
        Square king_sq = PieceOperations::getKingSquare(state, side);
        Color enemy = static_cast<Color>(!static_cast<bool>(side));

        Bitboard diagonal_attackers = PieceOperations::getPieces(state, enemy == Color::WHITE ? Piece::W_BISHOP : Piece::B_BISHOP) |
                                      PieceOperations::getPieces(state, enemy == Color::WHITE ? Piece::W_QUEEN : Piece::B_QUEEN);

        Bitboard orthogonal_attackers = PieceOperations::getPieces(state, enemy == Color::WHITE ? Piece::W_ROOK : Piece::B_ROOK) |
                                        PieceOperations::getPieces(state, enemy == Color::WHITE ? Piece::W_QUEEN : Piece::B_QUEEN);

        Bitboard pinned = 0;
        Bitboard own_pieces = PieceOperations::getPiecesOfColor(state, side);
        Bitboard all_pieces = PieceOperations::getAllPieces(state);

        while (diagonal_attackers)
        {
            Square attacker_sq = static_cast<Square>(getLSB(diagonal_attackers));
            Bitboard between = getBishopAttacksForSquare(attacker_sq, all_pieces) &
                               getBishopAttacksForSquare(king_sq, all_pieces);

            if (popCount(between & own_pieces) == 1 && (getBishopAttacksForSquare(attacker_sq, 0) & (1ULL << king_sq)))
            {
                pinned |= between & own_pieces;
            }
            diagonal_attackers = clearLSB(diagonal_attackers);
        }

        while (orthogonal_attackers)
        {
            Square attacker_sq = static_cast<Square>(getLSB(orthogonal_attackers));
            Bitboard between = getRookAttacksForSquare(attacker_sq, all_pieces) &
                               getRookAttacksForSquare(king_sq, all_pieces);

            if (popCount(between & own_pieces) == 1 && (getRookAttacksForSquare(attacker_sq, 0) & (1ULL << king_sq)))
            {
                pinned |= between & own_pieces;
            }
            orthogonal_attackers = clearLSB(orthogonal_attackers);
        }

        return pinned;
    }

} // namespace chess