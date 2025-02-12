#include "board/piece_ops.hpp"
#include "board/board.hpp"
#include "board/validation.hpp"
#include "../include/zobrist_keys.hpp"
#include <bit>
#include <bitset>
#include <iostream>

namespace chess
{
    Piece PieceOperations::getPiece(const GameState &state, Square square) noexcept
    {
        const Bitboard square_bb = 1ULL << static_cast<int>(square);
        const Bitboard *pieces = state.pieces;

        for (int p = static_cast<int>(Piece::W_PAWN); p <= static_cast<int>(Piece::B_KING); ++p)
        {
            if (pieces[p] & square_bb)
            {
                return static_cast<Piece>(p);
            }
        }
        return Piece::EMPTY;
    }

    Bitboard PieceOperations::getPieces(const GameState &state, Piece piece) noexcept
    {
        if (piece == Piece::EMPTY)
            return 0ULL;
        const int piece_idx = static_cast<int>(piece);
        return state.pieces[piece_idx];
    }

    Bitboard PieceOperations::getAllPieces(const GameState &state) noexcept
    {
        return getPiecesOfColor(state, Color::WHITE) | getPiecesOfColor(state, Color::BLACK);
    }

    Bitboard PieceOperations::getPiecesOfColor(const GameState &state, Color color) noexcept
    {
        Bitboard pieces_bb = 0ULL;
        const Bitboard *pieces = state.pieces;
        const int start_idx = color == Color::WHITE ? static_cast<int>(Piece::W_PAWN) : static_cast<int>(Piece::B_PAWN);
        const int end_idx = color == Color::WHITE ? static_cast<int>(Piece::W_KING) : static_cast<int>(Piece::B_KING);
        //std::cout << "Color: " << (color == Color::WHITE ? "WHITE" : "BLACK") << std::endl;
        //std::cout << "Start idx: " << start_idx << ", End idx: " << end_idx << std::endl;
        for (int p = start_idx; p <= end_idx; ++p)
        {
            //std::cout << "Piece " << p << " bitboard: " << std::bitset<64>(pieces[p]) << std::endl;
            pieces_bb |= pieces[p];
        }
        //std::cout << "Final pieces_bb: " << std::bitset<64>(pieces_bb) << std::endl;
        return pieces_bb;
    }

    Square PieceOperations::getKingSquare(const GameState &state, Color color) noexcept
    {
        const Piece king = color == Color::WHITE ? Piece::W_KING : Piece::B_KING;
        const Bitboard king_bb = getPieces(state, king);
        return static_cast<Square>(__builtin_ctzll(king_bb));
    }

    bool PieceOperations::isWhitePiece(const GameState &state, Square square) noexcept
    {
        const Piece piece = getPiece(state, square);
        if (piece == Piece::EMPTY)
            return false;
        return static_cast<int>(piece) <= static_cast<int>(Piece::W_KING);
    }

    Piece PieceOperations::getMovingPiece(const GameState &state, const Move &move) noexcept
    {
        return getPiece(state, move.from());
    }

    Piece PieceOperations::getCapturedPiece(const GameState &state, const Move &move) noexcept
    {
        if (MoveValidation::isEnPassant(move))
        {
            return move.color() == Color::WHITE ? Piece::B_PAWN : Piece::W_PAWN;
        }
        return getPiece(state, move.to());
    }

    void PieceOperations::setPiece(GameState &state, Square square, Piece piece) noexcept
    {
        //std::cout << "\nSetting piece at square " << static_cast<int>(square) << std::endl;

        const Bitboard square_bb = 1ULL << static_cast<int>(square);
        //std::cout << "Square bitboard: " << std::bitset<64>(square_bb) << std::endl;

        Bitboard *pieces = const_cast<Bitboard *>(state.pieces);

        Piece old_piece = getPiece(state, square);
        if (old_piece != Piece::EMPTY)
        {
            //std::cout << "Removing old piece: " << static_cast<int>(old_piece) << std::endl;
            //std::cout << "Old piece bitboard before: " << std::bitset<64>(pieces[static_cast<int>(old_piece)]) << std::endl;
            state.hash ^= ZOBRIST_KEYS[static_cast<int>(old_piece) * 64 + static_cast<int>(square)];
            pieces[static_cast<int>(old_piece)] &= ~square_bb;
            //std::cout << "Old piece bitboard after: " << std::bitset<64>(pieces[static_cast<int>(old_piece)]) << std::endl;
        }

        if (piece != Piece::EMPTY)
        {
            const int piece_idx = static_cast<int>(piece);
            //std::cout << "Setting new piece: " << piece_idx << std::endl;
            //std::cout << "New piece bitboard before: " << std::bitset<64>(pieces[piece_idx]) << std::endl;
            pieces[piece_idx] |= square_bb;
            //std::cout << "New piece bitboard after: " << std::bitset<64>(pieces[piece_idx]) << std::endl;
            state.hash ^= ZOBRIST_KEYS[piece_idx * 64 + static_cast<int>(square)];
        }
    }

    void PieceOperations::clearSquare(GameState &state, Square square) noexcept
    {
        Piece existing = getPiece(state, square);
        if (existing != Piece::EMPTY)
        {
            const Bitboard clear_mask = ~(1ULL << static_cast<int>(square));
            Bitboard *pieces = const_cast<Bitboard *>(state.pieces);
            pieces[static_cast<int>(existing)] &= clear_mask;
            // Update hash
            state.hash ^= ZOBRIST_KEYS[static_cast<int>(existing) * 64 + static_cast<int>(square)];
        }
    }

    void PieceOperations::movePiece(GameState &state, Square from, Square to) noexcept
    {
        const Piece piece = getPiece(state, from);
        if (piece == Piece::EMPTY)
            return;

        const Bitboard from_bb = 1ULL << static_cast<int>(from);
        const Bitboard to_bb = 1ULL << static_cast<int>(to);
        const int piece_idx = static_cast<int>(piece);
        Bitboard *pieces = const_cast<Bitboard *>(state.pieces);

        pieces[piece_idx] &= ~from_bb;
        pieces[piece_idx] |= to_bb;

        const Bitboard clear_mask = ~to_bb;
        for (int i = 0; i < 12; ++i)
        {
            if (i != piece_idx)
            {
                pieces[i] &= clear_mask;
            }
        }
    }
} // namespace chess