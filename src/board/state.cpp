#include "board/state.hpp"
#include "zobrist_keys.hpp"
#include "board/validation.hpp"
#include "board/piece_ops.hpp" 
#include <iostream>

namespace chess
{
    namespace
    {
        constexpr uint8_t DEFAULT_CASTLING_RIGHTS = 0b1111;
        
    }

    bool StateManager::makeMove(GameState &state, const Move &move) noexcept
    {
        Piece captured_piece = Piece::EMPTY;
        if (MoveValidation::isCapture(move))
        {
            Square to = move.to();
            captured_piece = PieceOperations::getPiece(state, to);
            PieceOperations::clearSquare(state, to);
            updateCaptureHash(state, to, captured_piece);
        }

        pushHistory(move, state, captured_piece);

        const Piece moving_piece = PieceOperations::getMovingPiece(state, move);
        PieceOperations::movePiece(state, move.from(), move.to());
        updatePieceHash(state, move.from(), move.to(), moving_piece);

        const uint8_t old_castling_rights = state.castling_rights;
        const uint8_t old_en_passant_file = state.en_passant_file;

        state.side_to_move = !state.side_to_move;
        state.hash ^= ZOBRIST_KEYS[793];

        state.halfmove_clock++;
        if (moving_piece == Piece::W_PAWN || moving_piece == Piece::B_PAWN || MoveValidation::isCapture(move))
        {
            state.halfmove_clock = 0;
        }

        if (state.side_to_move == 0)
        {
            state.fullmove_number++;
        }

        state.en_passant_file = 0;
        updateEnPassantHash(state, old_en_passant_file, 0);

        if (MoveValidation::isCastle(move))
        {
            handleCastling(state, move.from(), move.to());
        }
        else if (MoveValidation::isEnPassant(move))
        {
            handleEnPassant(state, move.from(), move.to());
        }
        else if (MoveValidation::isPromotion(move))
        {
            handlePromotion(state, move.to(), move.promotion());
        }
        else if ((moving_piece == Piece::W_PAWN || moving_piece == Piece::B_PAWN) &&
                 std::abs(static_cast<int>(move.to()) - static_cast<int>(move.from())) == 16)
        {
            state.en_passant_file = static_cast<uint8_t>(move.from()) % 8 + 1;
            updateEnPassantHash(state, 0, state.en_passant_file);
        }

        updateCastlingRights(state, move);
        updateCastlingHash(state, old_castling_rights, state.castling_rights);

        return true;
    }

    bool StateManager::undoMove(GameState &state) noexcept
    {
        if (!hasHistory())
            return false;

        const HistoryEntry &entry = history.back();
        const Move &move = entry.move;
        const GameState old_state = entry.prev_state;
        Piece captured_piece = entry.captured_piece;

        // First restore non-piece state
        state.en_passant_file = old_state.en_passant_file;
        state.castling_rights = old_state.castling_rights;
        state.halfmove_clock = old_state.halfmove_clock;
        state.fullmove_number = old_state.fullmove_number;

        // Then handle piece movements
        if (MoveValidation::isEnPassant(move))
        {
            undoEnPassant(state, move);
        }
        else if (MoveValidation::isPromotion(move))
        {
            undoPromotion(state, move);
        }
        else if (MoveValidation::isCastle(move))
        {
            undoCastling(state, move);
        }
        else
        {
            PieceOperations::movePiece(state, move.to(), move.from());
            updatePieceHash(state, move.to(), move.from(), PieceOperations::getMovingPiece(state, move));

            if (captured_piece != Piece::EMPTY)
            {
                PieceOperations::setPiece(state, move.to(), captured_piece);
                updateCaptureHash(state, move.to(), captured_piece);
            }
        }

        history.pop_back();
        return true;
    }
    bool StateManager::makeNullMove(GameState &state) noexcept
    {
        const uint8_t old_en_passant_file = state.en_passant_file;

        pushHistory(Move(), state, Piece::EMPTY);

        state.side_to_move = !state.side_to_move;
        state.hash ^= ZOBRIST_KEYS[793];

        state.en_passant_file = 0;
        updateEnPassantHash(state, old_en_passant_file, 0);

        state.halfmove_clock++;
        if (state.side_to_move == 0)
        {
            state.fullmove_number++;
        }

        return true;
    }

    bool StateManager::undoNullMove(GameState &state) noexcept
    {
        if (!hasHistory())
            return false;

        const HistoryEntry &entry = history.back();
        state = entry.prev_state;
        history.pop_back();

        return true;
    }
    void StateManager::pushHistory(const Move &move, const GameState &current_state, Piece captured) noexcept
    {
        if (history.size() < MAX_HISTORY_SIZE)
        {
            history.push_back({move, current_state, captured});
        }
    }

    void StateManager::handleCastling(GameState &state, Square from, Square to) noexcept
    {
        const bool is_kingside = to > from;
        const bool is_white = from == Square::E1;

        const Square rook_from = is_kingside ? (is_white ? Square::H1 : Square::H8)
                                             : (is_white ? Square::A1 : Square::A8);
        const Square rook_to = is_kingside ? (is_white ? Square::F1 : Square::F8)
                                           : (is_white ? Square::D1 : Square::D8);

        const Piece king = is_white ? Piece::W_KING : Piece::B_KING;
        const Piece rook = is_white ? Piece::W_ROOK : Piece::B_ROOK;

        state.pieces[static_cast<int>(king)] ^= (1ULL << static_cast<int>(from)) | (1ULL << static_cast<int>(to));
        state.pieces[static_cast<int>(rook)] ^= (1ULL << static_cast<int>(rook_from)) | (1ULL << static_cast<int>(rook_to));

        updatePieceHash(state, rook_from, rook_to, rook);
    }

    void StateManager::undoCastling(GameState &state, const Move &move) noexcept
    {
        handleCastling(state, move.to(), move.from());
    }

    void StateManager::handleEnPassant(GameState &state, Square from, Square to) noexcept
    {
        const int captured_rank = state.side_to_move ? 3 : 4;
        const Square captured_square = static_cast<Square>(captured_rank * 8 + static_cast<uint8_t>(to) % 8);

        Piece captured_pawn = state.side_to_move ? Piece::W_PAWN : Piece::B_PAWN;
        PieceOperations::clearSquare(state, captured_square);
        updateCaptureHash(state, captured_square, captured_pawn);
    }

    void StateManager::handlePromotion(GameState &state, Square to, Piece promotion_piece) noexcept
    {
        Piece pawn = PieceOperations::getPiece(state, to);
        PieceOperations::clearSquare(state, to);
        updatePieceHash(state, to, Square::NO_SQUARE, pawn);

        PieceOperations::setPiece(state, to, promotion_piece);
        updatePieceHash(state, Square::NO_SQUARE, to, promotion_piece);
    }

    void StateManager::undoEnPassant(GameState &state, const Move &move) noexcept
    {
        const Square capture_square = static_cast<Square>(
            (move.color() == Color::WHITE ? 4 : 3) * 8 + static_cast<uint8_t>(move.to()) % 8);
        const Piece captured_pawn = move.color() == Color::WHITE ? Piece::B_PAWN : Piece::W_PAWN;

        PieceOperations::setPiece(state, capture_square, captured_pawn);
        updateCaptureHash(state, capture_square, captured_pawn);

        PieceOperations::movePiece(state, move.to(), move.from());
        updatePieceHash(state, move.to(), move.from(),
                        move.color() == Color::WHITE ? Piece::W_PAWN : Piece::B_PAWN);
    }

    void StateManager::undoPromotion(GameState &state, const Move &move) noexcept
    {
        Piece promoted_piece = PieceOperations::getPiece(state, move.to());
        PieceOperations::clearSquare(state, move.to());
        updatePieceHash(state, move.to(), Square::NO_SQUARE, promoted_piece);

        Piece pawn = move.color() == Color::WHITE ? Piece::W_PAWN : Piece::B_PAWN;
        PieceOperations::setPiece(state, move.from(), pawn);
        updatePieceHash(state, Square::NO_SQUARE, move.from(), pawn);
    }

    void StateManager::updateCastlingRights(GameState &state, const Move &move) noexcept
    {
        const Square from = move.from();
        const Square to = move.to();

        if (from == Square::E1)
            state.castling_rights &= ~0b0011;
        else if (from == Square::E8)
            state.castling_rights &= ~0b1100;
        else if (from == Square::H1 || to == Square::H1)
            state.castling_rights &= ~0b0010;
        else if (from == Square::H8 || to == Square::H8)
            state.castling_rights &= ~0b1000;
        else if (from == Square::A1 || to == Square::A1)
            state.castling_rights &= ~0b0001;
        else if (from == Square::A8 || to == Square::A8)
            state.castling_rights &= ~0b0100;
    }

    void StateManager::updateHash(GameState & state) noexcept
    {
        state.hash = 0;
        for (Square sq = Square::A1; sq <= Square::H8;
             sq = static_cast<Square>(static_cast<int>(sq) + 1))
        {
            const Piece piece = PieceOperations::getPiece(state, sq);
            if (piece != Piece::EMPTY)
            {
                const int piece_idx = static_cast<int>(piece) - 1;
                state.hash ^= ZOBRIST_KEYS[piece_idx * 64 + static_cast<int>(sq)];
            }
        }

        if (state.side_to_move)
            state.hash ^= ZOBRIST_KEYS[793];
        state.hash ^= ZOBRIST_KEYS[777 + state.castling_rights];
        if (state.en_passant_file)
            state.hash ^= ZOBRIST_KEYS[769 + state.en_passant_file - 1];
    }

    void StateManager::updatePieceHash(GameState &state, Square from, Square to, Piece piece) noexcept
    {
        if (piece != Piece::EMPTY)
        {
            const size_t piece_idx = static_cast<size_t>(piece) - 1;
            if (from != Square::NO_SQUARE)
            {
                state.hash ^= ZOBRIST_KEYS[piece_idx * 64 + static_cast<size_t>(from)];
            }
            if (to != Square::NO_SQUARE)
            {
                state.hash ^= ZOBRIST_KEYS[piece_idx * 64 + static_cast<size_t>(to)];
            }
        }
    }

    void StateManager::updateCaptureHash(GameState &state, Square square, Piece captured_piece) noexcept
    {
        if (captured_piece != Piece::EMPTY)
        {
            const size_t piece_idx = static_cast<size_t>(captured_piece) - 1;
            state.hash ^= ZOBRIST_KEYS[piece_idx * 64 + static_cast<size_t>(square)];
        }
    }

    void StateManager::updateCastlingHash(GameState &state, uint8_t old_rights, uint8_t new_rights) noexcept
    {
        if (old_rights < 16)
            state.hash ^= ZOBRIST_KEYS[777 + old_rights];
        if (new_rights < 16)
            state.hash ^= ZOBRIST_KEYS[777 + new_rights];
    }

    void StateManager::updateEnPassantHash(GameState &state, uint8_t old_file, uint8_t new_file) noexcept
    {
        if (old_file > 0 && old_file <= 8)
            state.hash ^= ZOBRIST_KEYS[769 + old_file - 1];
        if (new_file > 0 && new_file <= 8)
            state.hash ^= ZOBRIST_KEYS[769 + new_file - 1];
    }

    Square StateManager::getEnPassantSquare(const GameState &state) const noexcept
    {
        if (state.en_passant_file == 0)
            return Square::NO_SQUARE;
        const int rank = state.side_to_move ? 2 : 5;
        return static_cast<Square>((rank * 8) + (state.en_passant_file - 1));
    }

    int StateManager::getRepetitionCount(const GameState &state) const noexcept
    {
        int count = 1;
        for (auto it = history.rbegin(); it != history.rend(); ++it)
        {
            if (it->prev_state.hash == state.hash)
            {
                count++;
            }
        }
        return count;
    }

} // namespace chess