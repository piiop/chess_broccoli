#include "board/pst.hpp"

namespace chess
{
    int16_t PieceSquareTables::getMgPST(size_t piece, size_t square) noexcept
    {
        // Convert piece index (0-11) to piece type (0-5)
        const size_t pieceType = piece % 6;
        // For black pieces, flip the square vertically
        const size_t adjustedSquare = (piece < 6) ? square : (square ^ 56);

        return PIECE_VALUES_MG[pieceType] + MG_PST[pieceType][adjustedSquare];
    }

    int16_t PieceSquareTables::getEgPST(size_t piece, size_t square) noexcept
    {
        // Convert piece index (0-11) to piece type (0-5)
        const size_t pieceType = piece % 6;
        // For black pieces, flip the square vertically
        const size_t adjustedSquare = (piece < 6) ? square : (square ^ 56);

        return PIECE_VALUES_EG[pieceType] + EG_PST[pieceType][adjustedSquare];
    }
} // namespace chess