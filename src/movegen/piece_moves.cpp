#include "../include/movegen/piece_moves.hpp"
#include "../include/bitboard/bitboard.hpp"
#include "../include/board/board.hpp"
#include "../include/movegen/move_builder.hpp"
#include <iostream>
namespace chess
{
    void generatePawnMoves(MoveList &moves, const GameState &state,
                           Bitboard pawns, Bitboard empty, Bitboard enemies) noexcept
    {
        const size_t initialMoves = moves.size();
        const Color side = state.side_to_move ? Color::BLACK : Color::WHITE;
        const Piece pawnType = Move::getPawnForColor(side);
        const Bitboard promotionRank = (side == Color::WHITE) ? patterns::RANK_8 : patterns::RANK_1;
        const Bitboard doublePushStartRank = (side == Color::WHITE) ? patterns::RANK_2 : patterns::RANK_7;

        Bitboard advances = getPawnAdvances(pawns, empty, side == Color::WHITE);
        Bitboard promotions = advances & promotionRank;
        advances &= ~promotionRank;

        size_t singlePushCount = 0;
        while (advances)
        {
            singlePushCount++;
            const Square to = static_cast<Square>(__builtin_ctzll(advances));
            const Square from = static_cast<Square>(to + (side == Color::WHITE ? -8 : 8));
            addMove(moves, from, to, side, pawnType);
            advances &= advances - 1;
        }
        //std::cout << "Debug: Single pushes generated: " << singlePushCount << std::endl;

        size_t doublePushCount = 0;
        Bitboard doublePushes = ((pawns & doublePushStartRank) << (side == Color::WHITE ? 16 : -16)) & empty & (empty << (side == Color::WHITE ? 8 : -8));
        while (doublePushes)
        {
            doublePushCount++;
            const Square to = static_cast<Square>(__builtin_ctzll(doublePushes));
            const Square from = static_cast<Square>(to - (side == Color::WHITE ? 16 : -16));
            addMove(moves, from, to, side, pawnType);
            doublePushes &= doublePushes - 1;
        }
        //std::cout << "Debug: Double pushes generated: " << doublePushCount << std::endl;

        static const Piece whitePromotions[] = {Piece::W_QUEEN, Piece::W_ROOK, Piece::W_BISHOP, Piece::W_KNIGHT};
        static const Piece blackPromotions[] = {Piece::B_QUEEN, Piece::B_ROOK, Piece::B_BISHOP, Piece::B_KNIGHT};
        const Piece *promotionPieces = (side == Color::WHITE) ? whitePromotions : blackPromotions;

        size_t promotionCount = 0;
        while (promotions)
        {
            promotionCount++;
            const Square to = static_cast<Square>(__builtin_ctzll(promotions));
            const Square from = static_cast<Square>(to - (side == Color::WHITE ? 8 : -8));
            for (int i = 0; i < 4; i++)
            {
                addMove(moves, from, to, side, pawnType, promotionPieces[i]);
            }
            promotions &= promotions - 1;
        }
        //std::cout << "Debug: Promotion moves generated: " << promotionCount * 4 << std::endl;

        const size_t preCaptureMoves = moves.size();
        generatePawnCaptures(moves, state, pawns, enemies);
        //std::cout << "Debug: Capture moves generated: " << moves.size() - preCaptureMoves << std::endl;
        //std::cout << "Debug: Total pawn moves generated: " << moves.size() - initialMoves << std::endl;
    }

    void generatePawnCaptures(MoveList &moves, const GameState &state,
                              Bitboard pawns, Bitboard enemies) noexcept
    {
        const Color side = state.side_to_move ? Color::BLACK : Color::WHITE;
        const Piece pawnType = Move::getPawnForColor(side);
        const Bitboard promotionRank = (side == Color::WHITE) ? patterns::RANK_8 : patterns::RANK_1;

        Bitboard captures = getPawnAttacks(pawns, side) & enemies;

        static const Piece whitePromotions[] = {Piece::W_QUEEN, Piece::W_ROOK, Piece::W_BISHOP, Piece::W_KNIGHT};
        static const Piece blackPromotions[] = {Piece::B_QUEEN, Piece::B_ROOK, Piece::B_BISHOP, Piece::B_KNIGHT};
        const Piece *promotionPieces = (side == Color::WHITE) ? whitePromotions : blackPromotions;

        while (captures)
        {
            const Square to = static_cast<Square>(__builtin_ctzll(captures));
            const Bitboard fromBoard = pawns & getPawnAttacksToSquare(to, side);
            const Square from = static_cast<Square>(__builtin_ctzll(fromBoard));
            if (toBitboard(to) & promotionRank)
            {
                for (int i = 0; i < 4; i++)
                {
                    addMove(moves, from, to, side, pawnType, promotionPieces[i], Move::CAPTURE_FLAG);
                }
            }
            else
            {
                addMove(moves, from, to, side, pawnType, Piece::EMPTY, Move::CAPTURE_FLAG);
            }
            captures &= captures - 1;
        }

        if (state.en_passant_file != 0) // 0 = None
        {
            Square epSquare = Square(((side == Color::WHITE ? 5 : 2) * 8) + state.en_passant_file - 1);
            Bitboard epCapturers = pawns & getPawnAttacksToSquare(epSquare, ~side);

            while (epCapturers)
            {
                Square from = static_cast<Square>(__builtin_ctzll(epCapturers));
                addMove(moves, from, epSquare, side, pawnType, Piece::EMPTY,
                        Move::CAPTURE_FLAG | Move::SPECIAL_FLAG);
                epCapturers &= epCapturers - 1;
            }
        }
    }

    void generateKnightMoves(MoveList &moves, const GameState &state,
                             Bitboard knights, Bitboard targets, Color sideColor, Piece knightPiece) noexcept
    {
        while (knights)
        {
            const Square from = static_cast<Square>(getLSB(knights));
            Bitboard attacks = getKnightAttacks(from) & targets;

            while (attacks)
            {
                const Square to = static_cast<Square>(getLSB(attacks));
                addMove(moves, from, to, sideColor, knightPiece, Piece::EMPTY,
                        (PieceOperations::getPiece(state, to) != Piece::EMPTY) ? Move::CAPTURE_FLAG : Move::QUIET_MOVE);
                attacks &= attacks - 1;
            }
            knights &= knights - 1;
        }
    }

    void generateBishopMoves(MoveList &moves, const GameState &state,
                             Bitboard bishops, Bitboard occupied, Bitboard targets,
                             Color sideColor, Piece bishopPiece) noexcept
    {
        while (bishops)
        {
            const Square from = static_cast<Square>(getLSB(bishops));
            Bitboard attacks = getBishopAttacksForSquare(from, occupied) & targets;

            while (attacks)
            {
                const Square to = static_cast<Square>(getLSB(attacks));
                addMove(moves, from, to, sideColor, bishopPiece, Piece::EMPTY,
                        (PieceOperations::getPiece(state, to) != Piece::EMPTY) ? Move::CAPTURE_FLAG : Move::QUIET_MOVE);
                attacks &= attacks - 1;
            }
            bishops &= bishops - 1;
        }
    }

    void generateRookMoves(MoveList &moves, const GameState &state,
                           Bitboard rooks, Bitboard occupied, Bitboard targets,
                           Color sideColor, Piece rookPiece) noexcept
    {
        while (rooks)
        {
            const Square from = static_cast<Square>(getLSB(rooks));
            Bitboard attacks = getRookAttacksForSquare(from, occupied) & targets;

            while (attacks)
            {
                const Square to = static_cast<Square>(getLSB(attacks));
                addMove(moves, from, to, sideColor, rookPiece, Piece::EMPTY,
                        (PieceOperations::getPiece(state, to) != Piece::EMPTY) ? Move::CAPTURE_FLAG : Move::QUIET_MOVE);
                attacks &= attacks - 1;
            }
            rooks &= rooks - 1;
        }
    }

    void generateQueenMoves(MoveList &moves, const GameState &state,
                            Bitboard queens, Bitboard occupied, Bitboard targets,
                            Color sideColor, Piece queenPiece) noexcept
    {
        while (queens)
        {
            const Square from = static_cast<Square>(getLSB(queens));
            // Combine rook and bishop attacks for queen moves
            Bitboard attacks = (getRookAttacksForSquare(from, occupied) |
                                getBishopAttacksForSquare(from, occupied)) &
                               targets;

            while (attacks)
            {
                const Square to = static_cast<Square>(getLSB(attacks));
                addMove(moves, from, to, sideColor, queenPiece, Piece::EMPTY,
                        (PieceOperations::getPiece(state, to) != Piece::EMPTY) ? Move::CAPTURE_FLAG : Move::QUIET_MOVE);
                attacks &= attacks - 1;
            }
            queens &= queens - 1;
        }
    }

    void generateKingMoves(MoveList &moves, const GameState &state,
                           Bitboard king, Bitboard targets,
                           Color sideColor, Piece kingPiece) noexcept
    {
        const Square from = static_cast<Square>(getLSB(king));
        Bitboard attacks = getKingAttacks(from) & targets;

        while (attacks)
        {
            const Square to = static_cast<Square>(getLSB(attacks));

            // Create temporary state with opposite side to check attacks
            GameState temp_state = state;
            temp_state.side_to_move ^= 1; // Switch sides using bit flip

            // Only generate move if destination square isn't attacked
            if (!MoveValidation::isSquareAttacked(temp_state, to))
            {
                addMove(moves, from, to, sideColor, kingPiece, Piece::EMPTY,
                        (PieceOperations::getPiece(state, to) != Piece::EMPTY) ? Move::CAPTURE_FLAG : Move::QUIET_MOVE);
            }
            attacks &= attacks - 1;
        }
    }

    void generateCastlingMoves(MoveList &moves, const GameState &state) noexcept
    {
        const Color side = state.side_to_move ? Color::BLACK : Color::WHITE;
        const bool isWhite = (side == Color::WHITE);
        const Bitboard occupied = PieceOperations::getAllPieces(state);
        const int rankNum = isWhite ? 0 : 7;
        const Square kingSquare = static_cast<Square>(rankNum * 8 + 4);

        const uint8_t rights = state.castling_rights;
        const bool canKingside = isWhite ? (rights & 1) : (rights & 4);
        const bool canQueenside = isWhite ? (rights & 2) : (rights & 8);

        // Create temporary state for attack checking
        GameState temp_state = state;
        temp_state.side_to_move ^= 1; // Switch sides using bit flip

        if (canKingside)
        {
            const Bitboard kingPath = (1ULL << (rankNum * 8 + 5)) | (1ULL << (rankNum * 8 + 6));
            const Bitboard rookPath = (1ULL << (rankNum * 8 + 5));

            if (!(occupied & (kingPath | rookPath)))
            {
                bool pathSafe = true;
                for (int file = 4; file <= 6 && pathSafe; ++file)
                {
                    if (MoveValidation::isSquareAttacked(temp_state, static_cast<Square>(rankNum * 8 + file)))
                    {
                        pathSafe = false;
                    }
                }

                if (pathSafe)
                {
                    const Square to = static_cast<Square>(rankNum * 8 + 6);
                    const Piece kingPiece = (side == Color::WHITE) ? Piece::W_KING : Piece::B_KING;
                    moves.push_back(ScoredMove{
                        Move(kingSquare, to, side, kingPiece, Piece::EMPTY, Move::CASTLING_FLAG),
                        0});
                }
            }
        }

        if (canQueenside)
        {
            const Bitboard kingPath = (1ULL << (rankNum * 8 + 3)) | (1ULL << (rankNum * 8 + 2));
            const Bitboard rookPath = (1ULL << (rankNum * 8 + 3)) | (1ULL << (rankNum * 8 + 2)) | (1ULL << (rankNum * 8 + 1));

            if (!(occupied & (kingPath | rookPath)))
            {
                bool pathSafe = true;
                for (int file = 4; file >= 2 && pathSafe; --file)
                {
                    if (MoveValidation::isSquareAttacked(temp_state, static_cast<Square>(rankNum * 8 + file)))
                    {
                        pathSafe = false;
                    }
                }

                if (pathSafe)
                {
                    const Square to = static_cast<Square>(rankNum * 8 + 2);
                    const Piece kingPiece = (side == Color::WHITE) ? Piece::W_KING : Piece::B_KING;
                    moves.push_back(ScoredMove{
                        Move(kingSquare, to, side, kingPiece, Piece::EMPTY, Move::CASTLING_FLAG),
                        0});
                }
            }
        }
    }

} // namespace chess