#ifndef PIECE_MOVES_HPP
#define PIECE_MOVES_HPP

#include "../types.hpp"
#include "../bitboard/bitboard.hpp"
#include "move_types.hpp"
#include "move_builder.hpp"
#include "board/validation.hpp"
namespace chess
{
    struct GameState;

    // Pawn move generation
    void generatePawnMoves(MoveList &moves, const GameState &state,
                           Bitboard pawns, Bitboard empty, Bitboard enemies) noexcept;

    void generatePawnCaptures(MoveList &moves, const GameState &state,
                              Bitboard pawns, Bitboard enemies) noexcept;

    // Knight move generation
    void generateKnightMoves(MoveList &moves, const GameState &state,
                             Bitboard knights, Bitboard targets, Color sideColor, Piece knightPiece) noexcept;

    // Bishop move generation
    void generateBishopMoves(MoveList &moves, const GameState &state,
                             Bitboard bishops, Bitboard occupied, Bitboard targets,
                             Color sideColor, Piece bishopPiece) noexcept;

    // Rook move generation
    void generateRookMoves(MoveList &moves, const GameState &state,
                           Bitboard rooks, Bitboard occupied, Bitboard targets,
                           Color sideColor, Piece rookPiece) noexcept;

    // Queen move generation
    void generateQueenMoves(MoveList &moves, const GameState &state,
                            Bitboard queens, Bitboard occupied, Bitboard targets,
                            Color sideColor, Piece queenPiece) noexcept;

    // King move generation
    void generateKingMoves(MoveList &moves, const GameState &state,
                           Bitboard king, Bitboard targets,
                           Color sideColor, Piece kingPiece) noexcept;

    void generateCastlingMoves(MoveList &moves, const GameState &state) noexcept;

} // namespace chess

#endif // PIECE_MOVES_HPP