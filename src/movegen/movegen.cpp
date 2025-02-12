#include "movegen/movegen.hpp"
#include "board/board.hpp"
#include "bitboard/bitboard.hpp"
#include "movegen/move_types.hpp"
#include "movegen/piece_moves.hpp"
#include "movegen/move_builder.hpp"
#include <iostream>
#include <bitset>
namespace chess
{
    void generatePseudoLegalMoves(MoveList &moves, const GameState &state) noexcept
    {
        moves.clear();
        const uint8_t side = state.side_to_move;
        const uint8_t opp = !side;
        const Bitboard *pieces = state.pieces;
        const Bitboard occupied = PieceOperations::getAllPieces(state);
        const Bitboard ownPieces = PieceOperations::getPiecesOfColor(state, static_cast<Color>(side));
        const Bitboard targets = ~ownPieces;
        const Color sideColor = static_cast<Color>(side);
        const Piece ownPawn = sideColor == Color::WHITE ? Piece::W_PAWN : Piece::B_PAWN;
        const Piece oppPawn = sideColor == Color::WHITE ? Piece::B_PAWN : Piece::W_PAWN;
        const Piece ownKnight = sideColor == Color::WHITE ? Piece::W_KNIGHT : Piece::B_KNIGHT;
        const Piece ownBishop = sideColor == Color::WHITE ? Piece::W_BISHOP : Piece::B_BISHOP;
        const Piece ownRook = sideColor == Color::WHITE ? Piece::W_ROOK : Piece::B_ROOK;
        const Piece ownQueen = sideColor == Color::WHITE ? Piece::W_QUEEN : Piece::B_QUEEN;
        const Piece ownKing = sideColor == Color::WHITE ? Piece::W_KING : Piece::B_KING;
        //std::cout << "Debug: occupied bitboard: " << std::bitset<64>(occupied) << std::endl;
        //std::cout << "Debug: empty (~occupied) bitboard: " << std::bitset<64>(~occupied) << std::endl;
        generatePawnMoves(moves, state,
                          PieceOperations::getPieces(state, ownPawn),
                          ~occupied,
                          PieceOperations::getPieces(state, oppPawn));

        generateKnightMoves(moves, state,
                            PieceOperations::getPieces(state, ownKnight),
                            targets,
                            sideColor,
                            ownKnight);

        generateBishopMoves(moves, state,
                            PieceOperations::getPieces(state, ownBishop),
                            occupied,
                            targets,
                            sideColor,
                            ownBishop);

        generateRookMoves(moves, state,
                          PieceOperations::getPieces(state, ownRook),
                          occupied,
                          targets,
                          sideColor,
                          ownRook);

        generateQueenMoves(moves, state,
                           PieceOperations::getPieces(state, ownQueen),
                           occupied,
                           targets,
                           sideColor,
                           ownQueen);

        generateKingMoves(moves, state,
                          PieceOperations::getPieces(state, ownKing),
                          targets,
                          sideColor,
                          ownKing);

        // Generate castling moves only if not in check
        if (!MoveValidation::isInCheck(state))
        {
            generateCastlingMoves(moves, state);
        }
    }

    void generateCaptures(MoveList &moves, const GameState &state, bool includeChecks) noexcept
    {
        moves.clear();
        const uint8_t side = state.side_to_move;
        const uint8_t opp = !side;
        const Bitboard *pieces = state.pieces;
        const Bitboard occupied = PieceOperations::getAllPieces(state);
        const Bitboard enemies = PieceOperations::getPiecesOfColor(state, static_cast<Color>(opp));
        const Bitboard empty = ~occupied;
        const Color sideColor = static_cast<Color>(side);

        // Define piece types based on side
        const Piece ownPawn = sideColor == Color::WHITE ? Piece::W_PAWN : Piece::B_PAWN;
        const Piece ownKnight = sideColor == Color::WHITE ? Piece::W_KNIGHT : Piece::B_KNIGHT;
        const Piece ownBishop = sideColor == Color::WHITE ? Piece::W_BISHOP : Piece::B_BISHOP;
        const Piece ownRook = sideColor == Color::WHITE ? Piece::W_ROOK : Piece::B_ROOK;
        const Piece ownQueen = sideColor == Color::WHITE ? Piece::W_QUEEN : Piece::B_QUEEN;
        const Piece ownKing = sideColor == Color::WHITE ? Piece::W_KING : Piece::B_KING;

        // Generate pawn captures and promotions
        Bitboard pawns = PieceOperations::getPieces(state, ownPawn);
        generatePawnCaptures(moves, state, pawns, enemies);

        // Add pawn promotions on non-capture moves
        if (sideColor == Color::WHITE)
        {
            Bitboard promotionRank = pawns & patterns::RANK_7;
            Bitboard pushes = (promotionRank << 8) & empty;
            while (pushes)
            {
                Square to = static_cast<Square>(getLSB(pushes));
                Square from = to - 8;
                addPromotionMove(moves, from, to, sideColor, Piece::W_QUEEN);
                pushes = clearLSB(pushes);
            }
        }
        else
        {
            Bitboard promotionRank = pawns & patterns::RANK_2;
            Bitboard pushes = (promotionRank >> 8) & empty;
            while (pushes)
            {
                Square to = static_cast<Square>(getLSB(pushes));
                Square from = to + 8;
                addPromotionMove(moves, from, to, sideColor, Piece::B_QUEEN);
                pushes = clearLSB(pushes);
            }
        }

        // Generate piece captures using piece-specific functions
        generateKnightMoves(moves, state,
                            PieceOperations::getPieces(state, ownKnight),
                            enemies,
                            sideColor,
                            ownKnight);

        generateBishopMoves(moves, state,
                            PieceOperations::getPieces(state, ownBishop),
                            occupied,
                            enemies,
                            sideColor,
                            ownBishop);

        generateRookMoves(moves, state,
                          PieceOperations::getPieces(state, ownRook),
                          occupied,
                          enemies,
                          sideColor,
                          ownRook);

        generateQueenMoves(moves, state,
                           PieceOperations::getPieces(state, ownQueen),
                           occupied,
                           enemies,
                           sideColor,
                           ownQueen);

        generateKingMoves(moves, state,
                          PieceOperations::getPieces(state, ownKing),
                          enemies,
                          sideColor,
                          ownKing);

        // Check moves (if requested)
        if (includeChecks)
        {
            const Square enemyKingSq = PieceOperations::getKingSquare(state, static_cast<Color>(opp));
            const Bitboard ownPieces = PieceOperations::getPiecesOfColor(state, sideColor);

            // Add discovered check candidates
            Bitboard discoveredCheckers = ownPieces &
                                          ~(getKnightAttacks(enemyKingSq) |
                                            getBishopAttacksForSquare(enemyKingSq, occupied) |
                                            getRookAttacksForSquare(enemyKingSq, occupied));

            while (discoveredCheckers)
            {
                Square from = static_cast<Square>(getLSB(discoveredCheckers));
                Piece piece = PieceOperations::getPiece(state, from);
                MoveList tempMoves;
                Bitboard singlePiece = (1ULL << from);

                switch (piece)
                {
                case Piece::W_PAWN:
                case Piece::B_PAWN:
                    generatePawnMoves(tempMoves, state, singlePiece, empty, enemies);
                    break;
                case Piece::W_KNIGHT:
                case Piece::B_KNIGHT:
                    generateKnightMoves(tempMoves, state, singlePiece, empty, sideColor, piece);
                    break;
                case Piece::W_BISHOP:
                case Piece::B_BISHOP:
                    generateBishopMoves(tempMoves, state, singlePiece, occupied, empty, sideColor, piece);
                    break;
                case Piece::W_ROOK:
                case Piece::B_ROOK:
                    generateRookMoves(tempMoves, state, singlePiece, occupied, empty, sideColor, piece);
                    break;
                case Piece::W_QUEEN:
                case Piece::B_QUEEN:
                    generateQueenMoves(tempMoves, state, singlePiece, occupied, empty, sideColor, piece);
                    break;
                case Piece::W_KING:
                case Piece::B_KING:
                    generateKingMoves(tempMoves, state, singlePiece, empty, sideColor, piece);
                    break;
                }

                for (const ScoredMove &scoredMove : tempMoves)
                {
                    if (MoveValidation::isDiscoveredCheck(state, from, scoredMove.move.to(), enemyKingSq))
                    {
                        moves.push_back({scoredMove.move, 0});
                    }
                }
                discoveredCheckers = clearLSB(discoveredCheckers);
            }
        }
    }

    void generateQuiescenceMoves(MoveList &moves, const GameState &state) noexcept
    {
        moves.clear();
        generateCaptures(moves, state);

        if (!MoveValidation::isInCheck(state))
        {
            MoveList checkMoves;
            generatePseudoLegalMoves(checkMoves, state);

            for (const auto &scored_move : checkMoves)
            {
                if (!MoveValidation::isCapture(scored_move.move))
                {
                    GameState testState = state;
                    StateManager stateManager;

                    if (stateManager.makeMove(testState, scored_move.move) &&
                        MoveValidation::isInCheck(testState))
                    {
                        moves.push_back(scored_move);
                    }
                }
            }
        }
    }

} // namespace chess