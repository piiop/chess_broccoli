#include "board/validation.hpp"
#include "board/board.hpp"
#include "board/piece_ops.hpp"
#include "bitboard/attacks.hpp"
#include "movegen/movegen.hpp"
#include "board/pin_detection.hpp"
#include <bit>
#include <chrono>
#include <iostream>
#include <string>
namespace chess
{

    std::vector<chess::ScoredMove> MoveValidation::getLegalMoves(const GameState &state) noexcept
    {
        MoveList moves;
        moves.reserve(MAX_MOVES);
        generatePseudoLegalMoves(moves, state);
        std::cout << "Debug (getLegalMoves) - gen Pseudo complete. Count: " << moves.size() << std::endl;

        // Add debug for each pseudo-legal move
        for (const auto &move : moves)
        {
            //std::cout << "Debug - Pseudo move: " << getPieceName(move.move.piece())
            //          << " " << squareToString(move.move.from())
            //          << squareToString(move.move.to()) << std::endl;
        }

        MoveList legal_moves;
        legal_moves.reserve(moves.size());

        for (const auto &move : moves)
        {
            if (isLegalMove(state, move.move))
            {
                legal_moves.push_back(move);
                // Add debug for each legal move
                //std::cout << "Debug - Legal move: " << getPieceName(move.move.piece())
                //          << " " << squareToString(move.move.from())
                //          << squareToString(move.move.to()) << std::endl;
            }
        }
        std::cout << "Debug (getLegalMoves) - Move count: " << legal_moves.size() << std::endl;
        return legal_moves;
    }

    bool MoveValidation::isInCheck(const GameState &state) noexcept
    {
        // Create a temporary state to check from opponent's perspective
        GameState temp_state = state;
        temp_state.side_to_move = !temp_state.side_to_move; // Flip side to move to check from attacker's perspective

        const Piece king = state.side_to_move ? Piece::B_KING : Piece::W_KING;
        Square king_square = static_cast<Square>(getLSB(state.pieces[static_cast<int>(king)]));

        return isSquareAttacked(temp_state, king_square);
    }

    bool MoveValidation::isSquareAttacked(const GameState &state, Square square) noexcept
    {
        // Calculate attacker based on state's side to move
        Color attacker = static_cast<Color>(state.side_to_move);

        // Calculate all pieces bitboard
        Bitboard allPieces = 0;
        for (int p = 0; p < 12; p++)
        {
            allPieces |= state.pieces[p];
        }

        // Calculate offset based on attacker's color
        int offset = attacker == Color::WHITE ? 0 : 6;

        // Check pawn attacks
        if (state.pieces[offset + static_cast<int>(Piece::W_PAWN)] &
            getPawnAttacks(1ULL << static_cast<int>(square), attacker))
        {
            return true;
        }

        // Check knight attacks
        if (state.pieces[offset + static_cast<int>(Piece::W_KNIGHT)] &
            getKnightAttacks(square))
        {
            return true;
        }

        // Check bishop and queen diagonal attacks
        Bitboard bishopQueenAttacks = getBishopAttacksForSquare(square, allPieces);
        if ((state.pieces[offset + static_cast<int>(Piece::W_BISHOP)] |
             state.pieces[offset + static_cast<int>(Piece::W_QUEEN)]) &
            bishopQueenAttacks)
        {
            return true;
        }

        // Check rook and queen straight attacks
        Bitboard rookQueenAttacks = getRookAttacksForSquare(square, allPieces);
        if ((state.pieces[offset + static_cast<int>(Piece::W_ROOK)] |
             state.pieces[offset + static_cast<int>(Piece::W_QUEEN)]) &
            rookQueenAttacks)
        {
            return true;
        }

        // Check king attacks
        return state.pieces[offset + static_cast<int>(Piece::W_KING)] &
               getKingAttacks(square);
    }

    bool MoveValidation::isDraw(const GameState &state, const StateManager &manager) noexcept
    {
        //std::cout << "Debug (isDraw): Begin" << std::endl;
        //std::cout << "Board state - Side to move: " << (state.side_to_move == 0 ? "White" : "Black")
        //         << ", White king bb: " << (board.getPieces()[static_cast<int>(Piece::W_KING)])
        //          << ", Black king bb: " << (board.getPieces()[static_cast<int>(Piece::B_KING)])
        //          << std::endl;

        if (state.halfmove_clock >= 100)
        {
            //std::cout << "Draw by fifty-move rule" << std::endl;
            return true;
        }
        //std::cout << "50 move clear - checking nsf" << std::endl;
        if (isInsufficientMaterial(state))
        {
            //std::cout << "Draw by insufficient material" << std::endl;
            return true;
        }
        //std::cout << "nsf clear - checking rep" << std::endl;
        if (manager.getRepetitionCount(state) >= 3)
        {
            //std::cout << "Draw by repetition" << std::endl;
            return true;
        }
        //std::cout << "rep clear - checking stalemate" << std::endl;

        // Get legal moves and check for stalemate
        if (MoveValidation::getLegalMoves(state).empty() && !isInCheck(state))
        {
            //std::cout << "Draw by stalemate" << std::endl;
            return true;
        }
        //std::cout << "Draw condtions: no draw" << std::endl;
        return false;
    }

    bool MoveValidation::isInsufficientMaterial(const GameState &state) noexcept
    {
        // Check pawns
        if (PieceOperations::getPieces(state, Piece::W_PAWN) ||
            PieceOperations::getPieces(state, Piece::B_PAWN))
            return false;

        // Count pieces
        int whiteKnights = popCount(PieceOperations::getPieces(state, Piece::W_KNIGHT));
        int whiteBishops = popCount(PieceOperations::getPieces(state, Piece::W_BISHOP));
        int whiteRooks = popCount(PieceOperations::getPieces(state, Piece::W_ROOK));
        int whiteQueens = popCount(PieceOperations::getPieces(state, Piece::W_QUEEN));

        int blackKnights = popCount(PieceOperations::getPieces(state, Piece::B_KNIGHT));
        int blackBishops = popCount(PieceOperations::getPieces(state, Piece::B_BISHOP));
        int blackRooks = popCount(PieceOperations::getPieces(state, Piece::B_ROOK));
        int blackQueens = popCount(PieceOperations::getPieces(state, Piece::B_QUEEN));

        // Check for queens or rooks
        if (whiteQueens || blackQueens || whiteRooks || blackRooks)
            return false;

        // Check for multiple minor pieces
        if (whiteKnights + whiteBishops > 1 || blackKnights + blackBishops > 1)
            return false;

        // Kings only
        if (whiteKnights + whiteBishops + blackKnights + blackBishops == 0)
            return true;

        // Single minor piece per side or less
        return (whiteKnights + whiteBishops <= 1) && (blackKnights + blackBishops <= 1);
    }

    bool MoveValidation::canCastleKingside(const GameState &state, Color side) noexcept
    {
        // First do the quick bitfield check
        if (!(state.castling_rights & (side == Color::WHITE ? 0x1 : 0x4)))
        {
            return false;
        }

        // Only if rights exist, do the expensive validation
        return validateKingsideCastleSquares(state, side);
    }

    bool MoveValidation::canCastleQueenside(const GameState &state, Color side) noexcept
    {
        // Quick bitfield check - white queenside (0x2) or black queenside (0x8)
        if (!(state.castling_rights & (side == Color::WHITE ? 0x2 : 0x8)))
        {
            return false;
        }

        // Only if rights exist, do the expensive validation
        return validateQueensideCastleSquares(state, side);
    }

    bool MoveValidation::validateCastle(const GameState &state, const Move &move) noexcept
    {
        const Color side = move.color();
        const Square to = move.to();

        if (to == Square::G1 || to == Square::G8)
            return canCastleKingside(state, side);
        if (to == Square::C1 || to == Square::C8)
            return canCastleQueenside(state, side);

        return false;
    }

    bool MoveValidation::validateKingsideCastleSquares(const GameState &state, Color side) noexcept
    {
        const Square king_square = side == Color::WHITE ? Square::E1 : Square::E8;
        const Square rook_square = side == Color::WHITE ? Square::H1 : Square::H8;
        const Square f_square = side == Color::WHITE ? Square::F1 : Square::F8;
        const Square g_square = side == Color::WHITE ? Square::G1 : Square::G8;

        if (PieceOperations::getPiece(state, king_square) != (side == Color::WHITE ? Piece::W_KING : Piece::B_KING) ||
            PieceOperations::getPiece(state, rook_square) != (side == Color::WHITE ? Piece::W_ROOK : Piece::B_ROOK))
        {
            return false;
        }

        if (PieceOperations::getPiece(state, f_square) != Piece::EMPTY ||
            PieceOperations::getPiece(state, g_square) != Piece::EMPTY)
        {
            return false;
        }

        // Create a temporary state with opposite side to move for attack checking
        GameState temp_state = state;
        temp_state.side_to_move ^= 1; // Use opposite side as the attacker

        return !isSquareAttacked(temp_state, king_square) &&
               !isSquareAttacked(temp_state, f_square) &&
               !isSquareAttacked(temp_state, g_square);
    }

    bool MoveValidation::validateQueensideCastleSquares(const GameState &state, Color side) noexcept
    {
        const Square king_square = side == Color::WHITE ? Square::E1 : Square::E8;
        const Square rook_square = side == Color::WHITE ? Square::A1 : Square::A8;
        const Square d_square = side == Color::WHITE ? Square::D1 : Square::D8;
        const Square c_square = side == Color::WHITE ? Square::C1 : Square::C8;
        const Square b_square = side == Color::WHITE ? Square::B1 : Square::B8;

        if (PieceOperations::getPiece(state, king_square) != (side == Color::WHITE ? Piece::W_KING : Piece::B_KING) ||
            PieceOperations::getPiece(state, rook_square) != (side == Color::WHITE ? Piece::W_ROOK : Piece::B_ROOK))
        {
            return false;
        }

        if (PieceOperations::getPiece(state, b_square) != Piece::EMPTY ||
            PieceOperations::getPiece(state, c_square) != Piece::EMPTY ||
            PieceOperations::getPiece(state, d_square) != Piece::EMPTY)
        {
            return false;
        }

        // Create a temporary state with opposite side to move for attack checking
        GameState temp_state = state;
        temp_state.side_to_move ^= 1; // Use opposite side as the attacker

        return !isSquareAttacked(temp_state, king_square) &&
               !isSquareAttacked(temp_state, d_square) &&
               !isSquareAttacked(temp_state, c_square);
    }

    bool MoveValidation::verifyPosition(const GameState &state) noexcept
    {
        const Bitboard *pieces = state.pieces;

        // Verify exactly one king of each color
        if (popCount(pieces[static_cast<int>(Piece::W_KING)]) != 1 ||
            popCount(pieces[static_cast<int>(Piece::B_KING)]) != 1)
        {
            return false;
        }

        // Verify no pawns on first/last rank
        Bitboard pawns = pieces[static_cast<int>(Piece::W_PAWN)] | pieces[static_cast<int>(Piece::B_PAWN)];
        if (pawns & (patterns::RANK_1 | patterns::RANK_8))
        {
            return false;
        }

        // Verify kings are not adjacent
        const Square w_king = PieceOperations::getKingSquare(state, Color::WHITE);
        const Square b_king = PieceOperations::getKingSquare(state, Color::BLACK);
        const int file_diff = std::abs((static_cast<int>(w_king) % 8) - (static_cast<int>(b_king) % 8));
        const int rank_diff = std::abs((static_cast<int>(w_king) / 8) - (static_cast<int>(b_king) / 8));
        if (file_diff <= 1 && rank_diff <= 1)
        {
            return false;
        }

        // Verify no overlapping pieces
        Bitboard all_pieces = 0ULL;
        for (int i = 0; i < 12; ++i) // 12 piece types total
        {
            if (all_pieces & pieces[i]) // If any overlap exists
            {
                return false;
            }
            all_pieces |= pieces[i];
        }

        // Verify non-moving side is not in check
        GameState temp_state = state;
        temp_state.side_to_move = state.side_to_move ^ 1;
        if (isInCheck(temp_state))
        {
            return false;
        }

        return true; // Position is valid if all checks pass
    }

    bool MoveValidation::isDiscoveredCheck(const GameState &state, Square from, Square to, Square enemyKing) noexcept
    {
        Bitboard occupied = PieceOperations::getAllPieces(state);
        occupied &= ~(1ULL << from);
        occupied |= (1ULL << to);

        const Color side = static_cast<Color>(state.side_to_move); // Convert 0/1 to Color enum
        Bitboard attackers = PieceOperations::getPiecesOfColor(state, side);
        attackers &= ~(1ULL << from);
        attackers |= (1ULL << to);

        Bitboard bishops = PieceOperations::getPieces(state, side == Color::WHITE ? Piece::W_BISHOP : Piece::B_BISHOP);
        Bitboard queens = PieceOperations::getPieces(state, side == Color::WHITE ? Piece::W_QUEEN : Piece::B_QUEEN);
        Bitboard rooks = PieceOperations::getPieces(state, side == Color::WHITE ? Piece::W_ROOK : Piece::B_ROOK);

        return (getBishopAttacksForSquare(enemyKing, occupied) & (bishops | queens) & attackers) |
               (getRookAttacksForSquare(enemyKing, occupied) & (rooks | queens) & attackers);
    }
    bool MoveValidation::isLegalMove(const GameState &state, const Move &move) noexcept
    {
        // Handle castling moves separately
        if (isCastle(move))
        {
            return validateCastle(state, move);
        }

        // Create temporary state and manager for move validation
        GameState temp_state = state;
        StateManager temp_manager;

        // Try to make the move
        if (!temp_manager.makeMove(temp_state, move))
        {
            return false; // Move couldn't be made (likely invalid)
        }

        // Check if the king is in check after the move
        bool in_check = isInCheck(temp_state);

        // No need to undo the move since we used a temporary state
        return !in_check;
    }

    } // namespace chess