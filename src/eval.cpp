#include "eval.hpp"
#include "board/piece_ops.hpp"
#include "bitboard/attacks.hpp"
#include "bitboard/core_ops.hpp"
#include "board/pst.hpp"

namespace chess
{

    // Then we can fix evaluatePosition:
    Score evaluatePosition(const GameState &state) noexcept
    {
        const int phase = detail::calculateGamePhase(state);
        const Score material = detail::evaluateMaterial(state);
        const Score positional = detail::evaluatePositionalScore(state);
        const Score pawnStructure = detail::evaluatePawnStructure(state);

        // Evaluate both sides' king safety using explicit colors
        const Score whiteKingSafety = detail::evaluateKingSafety(state, Color::WHITE);
        const Score blackKingSafety = detail::evaluateKingSafety(state, Color::BLACK);

        // Combine all evaluation terms
        Score totalScore = material + positional + pawnStructure +
                           whiteKingSafety - blackKingSafety;

        // Adjust score based on side to move
        if (state.side_to_move == 1)
        {
            totalScore = -totalScore;
        }

        return totalScore;
    }

    namespace detail
    {

        Score interpolateScore(Score mgScore, Score egScore, int phase) noexcept
        {
            return (mgScore * phase + egScore * (TOTAL_PHASE - phase)) / TOTAL_PHASE;
        }

        int calculateGamePhase(const GameState &state) noexcept
        {
            constexpr int PHASE_VALUES[] = {0, 0, 1, 1, 2, 4, 0}; // Empty,P,N,B,R,Q,K
            constexpr int TOTAL_PHASE = 24;                       // 2*(1*4 + 1*4 + 2*2 + 4*1)

            int phase = TOTAL_PHASE;

            // Starting from KNIGHT (index 2) to QUEEN (index 5)
            for (int piece_idx = static_cast<int>(Piece::W_KNIGHT);
                 piece_idx <= static_cast<int>(Piece::W_QUEEN);
                 ++piece_idx)
            {
                // Get both white and black pieces of this type
                Bitboard pieces = state.pieces[piece_idx] |    // White pieces
                                  state.pieces[piece_idx + 6]; // Black pieces (+6 offset for black)

                // Subtract the phase value for each piece found
                phase -= popCount(pieces) * PHASE_VALUES[piece_idx - 1]; // -1 because PHASE_VALUES starts at EMPTY
            }

            return phase;
        }

        Score evaluateMaterial(const GameState &state) noexcept
        {
            Score mgScore = 0;
            Score egScore = 0;

            // Uusing indices since GameState.pieces array
            for (int i = static_cast<int>(Piece::W_PAWN);
                 i <= static_cast<int>(Piece::W_QUEEN);
                 ++i)
            {
                const int whitePieceCount = popCount(state.pieces[i]);
                const int blackPieceCount = popCount(state.pieces[i + 6]); // +6 offset for black pieces

                const Score pieceValue = PIECE_VALUES[i];
                mgScore += pieceValue * (whitePieceCount - blackPieceCount);
                egScore += pieceValue * (whitePieceCount - blackPieceCount);
            }

            if (evaluateBishopPair(state, Color::WHITE))
                mgScore += 30;
            if (evaluateBishopPair(state, Color::BLACK))
                mgScore -= 30;

            return interpolateScore(mgScore, egScore, calculateGamePhase(state));
        }

        Score evaluatePositionalScore(const GameState &state) noexcept
        {
            Score mgScore = 0, egScore = 0;
            const int phase = calculateGamePhase(state);

            // Iterate through each piece type
            for (int piece_idx = 0; piece_idx < 12; ++piece_idx)
            {
                Bitboard pieces = state.pieces[piece_idx];
                if (!pieces)
                    continue; // Skip if no pieces of this type

                // Get color based on piece index
                Color c = piece_idx < 6 ? Color::WHITE : Color::BLACK;

                // For each piece of this type, get its squares
                while (pieces)
                {
                    Square sq = static_cast<Square>(getLSB(pieces));
                    pieces &= pieces - 1; // Clear LSB

                    // Add piece-square table scores
                    mgScore += PieceSquareTables::getMgPST(piece_idx, static_cast<size_t>(sq));
                    egScore += PieceSquareTables::getEgPST(piece_idx, static_cast<size_t>(sq));

                    // Add mobility evaluation in endgame
                    if (phase < 12)
                    {
                        Score mobility = evaluatePieceMobility(state, sq, static_cast<Piece>(piece_idx));
                        egScore += mobility;
                    }
                }
            }

            return interpolateScore(mgScore, egScore, phase);
        }

        Score evaluatePawnStructure(const GameState &state) noexcept
        {
            Score mgScore = 0;
            Score egScore = 0;

            // Evaluate white pawns
            Bitboard whitePawns = state.pieces[static_cast<int>(Piece::W_PAWN)];
            while (whitePawns)
            {
                Square sq = static_cast<Square>(getLSB(whitePawns));
                whitePawns = clearLSB(whitePawns);

                if (isDoubledPawn(state, sq, Color::WHITE))
                {
                    mgScore += terms::DOUBLED_PAWN_PENALTY_MG;
                    egScore += terms::DOUBLED_PAWN_PENALTY_EG;
                }

                if (isIsolatedPawn(state, sq))
                {
                    mgScore += terms::ISOLATED_PAWN_PENALTY_MG;
                    egScore += terms::ISOLATED_PAWN_PENALTY_EG;
                }

                if (isBackwardPawn(state, sq, Color::WHITE))
                {
                    mgScore += terms::BACKWARD_PAWN_PENALTY;
                    egScore += terms::BACKWARD_PAWN_PENALTY;
                }

                if (isPassedPawn(state, sq, Color::WHITE))
                {
                    int rank = static_cast<int>(sq) / 8;
                    mgScore += terms::PASSED_PAWN_BONUS[rank];
                    egScore += terms::PASSED_PAWN_BONUS[rank] * 2;
                }
            }

            // Evaluate black pawns
            Bitboard blackPawns = state.pieces[static_cast<int>(Piece::B_PAWN)];
            while (blackPawns)
            {
                Square sq = static_cast<Square>(getLSB(blackPawns));
                blackPawns = clearLSB(blackPawns);

                if (isDoubledPawn(state, sq, Color::BLACK))
                {
                    mgScore -= terms::DOUBLED_PAWN_PENALTY_MG;
                    egScore -= terms::DOUBLED_PAWN_PENALTY_EG;
                }

                if (isIsolatedPawn(state, sq))
                {
                    mgScore -= terms::ISOLATED_PAWN_PENALTY_MG;
                    egScore -= terms::ISOLATED_PAWN_PENALTY_EG;
                }

                if (isBackwardPawn(state, sq, Color::BLACK))
                {
                    mgScore -= terms::BACKWARD_PAWN_PENALTY;
                    egScore -= terms::BACKWARD_PAWN_PENALTY;
                }

                if (isPassedPawn(state, sq, Color::BLACK))
                {
                    int rank = 7 - static_cast<int>(sq) / 8;
                    mgScore -= terms::PASSED_PAWN_BONUS[rank];
                    egScore -= terms::PASSED_PAWN_BONUS[rank] * 2;
                }
            }

            return interpolateScore(mgScore, egScore, calculateGamePhase(state));
        }

        Score evaluateKingSafety(const GameState &state, Color side_to_evaluate) noexcept
        {
            Color c = side_to_evaluate;

            const Square kingSquare = PieceOperations::getKingSquare(state, c);
            const Color opponent = ~c;
            const Bitboard allPieces = PieceOperations::getAllPieces(state);

            Score score = evaluateKingPawnShield(state, kingSquare, c);

            // Get opponent piece indices for GameState.pieces array
            const int knightIdx = (opponent == Color::WHITE) ? static_cast<int>(Piece::W_KNIGHT) : static_cast<int>(Piece::B_KNIGHT);
            const int bishopIdx = (opponent == Color::WHITE) ? static_cast<int>(Piece::W_BISHOP) : static_cast<int>(Piece::B_BISHOP);
            const int rookIdx = (opponent == Color::WHITE) ? static_cast<int>(Piece::W_ROOK) : static_cast<int>(Piece::B_ROOK);
            const int queenIdx = (opponent == Color::WHITE) ? static_cast<int>(Piece::W_QUEEN) : static_cast<int>(Piece::B_QUEEN);

            // Calculate attacks
            const Bitboard rookAttacks = getRookAttacksForSquare(kingSquare, allPieces);
            const Bitboard bishopAttacks = getBishopAttacksForSquare(kingSquare, allPieces);

            // Count attackers using direct piece bitboard access
            Bitboard attackers = getKnightAttacks(kingSquare) & state.pieces[knightIdx];
            attackers |= bishopAttacks & state.pieces[bishopIdx];
            attackers |= rookAttacks & state.pieces[rookIdx];
            attackers |= (rookAttacks | bishopAttacks) & state.pieces[queenIdx];

            return score - popCount(attackers) * 10;
        }

        Score evaluatePieceMobility(const GameState &state, Square sq, Piece pt) noexcept
        {
            Bitboard attacks = 0;
            const Bitboard allPieces = PieceOperations::getAllPieces(state);
            const int pieceIndex = static_cast<int>(pt) % 6; // Modulo 6 to get the piece type
            const Color c = static_cast<int>(pt) < 6 ? Color::WHITE : Color::BLACK;

            // Calculate attacks based on piece type
            switch (pieceIndex)
            {
            case static_cast<int>(Piece::W_KNIGHT) % 6: // K
                attacks = getKnightAttacks(sq);
                break;
            case static_cast<int>(Piece::W_BISHOP) % 6: // B
                attacks = getBishopAttacksForSquare(sq, allPieces);
                break;
            case static_cast<int>(Piece::W_ROOK) % 6: // R
                attacks = getRookAttacksForSquare(sq, allPieces);
                break;
            case static_cast<int>(Piece::W_QUEEN) % 6: // Q
                // Combine rook and bishop attacks for queen
                attacks = getRookAttacksForSquare(sq, allPieces) |
                          getBishopAttacksForSquare(sq, allPieces);
                break;
            default:
                return 0;
            }

            // Get own pieces to exclude from mobility
            Bitboard ownPieces = 0;
            if (c == Color::WHITE)
            {
                // Combine all white piece bitboards (indices 0-5)
                for (int i = 0; i < 6; ++i)
                {
                    ownPieces |= state.pieces[i];
                }
            }
            else
            {
                // Combine all black piece bitboards (indices 6-11)
                for (int i = 6; i < 12; ++i)
                {
                    ownPieces |= state.pieces[i];
                }
            }
            // Don't count attacks on own pieces
            attacks &= ~ownPieces;
            // pieceIndex - 1 instead of - 2 since we're using modulo 6 indexing
            return popCount(attacks) * terms::MOBILITY_BONUS[pieceIndex - 1];
        }

        bool isPassedPawn(const GameState &state, Square sq, Color c) noexcept
        {
            const int file = static_cast<int>(sq) & 7;
            const int rank = static_cast<int>(sq) / 8;
            Bitboard passedMask = 0;

            if (c == Color::WHITE)
            {
                passedMask = patterns::RANK_1 | patterns::RANK_2 | patterns::RANK_3 |
                             patterns::RANK_4 | patterns::RANK_5 | patterns::RANK_6 |
                             patterns::RANK_7 | patterns::RANK_8;
            }
            else
            {
                passedMask = patterns::RANK_8 | patterns::RANK_7 | patterns::RANK_6 |
                             patterns::RANK_5 | patterns::RANK_4 | patterns::RANK_3 |
                             patterns::RANK_2 | patterns::RANK_1;
            }

            if (file > 0)
                passedMask |= patterns::FILE_A << (file - 1);
            passedMask |= patterns::FILE_A << file;
            if (file < 7)
                passedMask |= patterns::FILE_A << (file + 1);

            return !(passedMask & PieceOperations::getPieces(state, c == Color::WHITE ? Piece::B_PAWN : Piece::W_PAWN));
        }

        bool isIsolatedPawn(const GameState &state, Square sq) noexcept
        {
            const int file = static_cast<int>(sq) & 7;
            const bool isWhite = (PieceOperations::getPieces(state, Piece::W_PAWN) & (1ULL << sq)) != 0;

            return !(patterns::ADJACENT_FILES[file] & PieceOperations::getPieces(state,
                                                                                 isWhite ? Piece::W_PAWN : Piece::B_PAWN));
        }

        bool isBackwardPawn(const GameState &state, Square sq, Color c) noexcept
        {
            const int file = static_cast<int>(sq) & 7;
            const int rank = static_cast<int>(sq) / 8;

            // Check adjacent files for supporting pawns
            Bitboard supportMask = 0;
            if (file > 0)
                supportMask |= patterns::FILE_A << (file - 1);
            if (file < 7)
                supportMask |= patterns::FILE_A << (file + 1);

            // Adjust mask based on color
            if (c == Color::WHITE)
            {
                supportMask &= (patterns::RANK_1 | patterns::RANK_2 | patterns::RANK_3 |
                                patterns::RANK_4 | patterns::RANK_5) >>
                               (8 * (rank - 1));
            }
            else
            {
                supportMask &= (patterns::RANK_4 | patterns::RANK_5 | patterns::RANK_6 |
                                patterns::RANK_7 | patterns::RANK_8)
                               << (8 * (6 - rank));
            }

            return !(supportMask & PieceOperations::getPieces(state,
                                                              c == Color::WHITE ? Piece::W_PAWN : Piece::B_PAWN));
        }

        bool isDoubledPawn(const GameState &state, Square sq, Color c) noexcept
        {
            const int file = static_cast<int>(sq) & 7;
            const Bitboard fileMask = patterns::FILE_A << file;
            const Bitboard pawns = PieceOperations::getPieces(state,
                                                              c == Color::WHITE ? Piece::W_PAWN : Piece::B_PAWN);

            return popCount(fileMask & pawns) > 1;
        }

        Score evaluateKingPawnShield(const GameState &state, Square kingSquare, Color c) noexcept
        {
            // Early exit for kings not on back ranks
            const int kRank = static_cast<int>(kingSquare) >> 3; 
            if ((c == Color::WHITE && kRank > 1) || (c == Color::BLACK && kRank < 6))
                return 0;

            const int kFile = static_cast<int>(kingSquare) & 7;
            const Piece pawnType = (c == Color::WHITE) ? Piece::W_PAWN : Piece::B_PAWN;
            const Bitboard shieldMask = (c == Color::WHITE) ? (patterns::RANK_2 | patterns::RANK_3) : (patterns::RANK_7 | patterns::RANK_6);
            const Bitboard pawns = PieceOperations::getPieces(state, pawnType);

            Score score = 0;
            // Check three files at once using bitwise operations
            Bitboard relevantFiles = ((patterns::FILE_A << std::max(0, kFile - 1)) |
                                      (patterns::FILE_A << kFile) |
                                      (patterns::FILE_A << std::min(7, kFile + 1)));

            Bitboard shieldPawns = pawns & shieldMask & relevantFiles;
            score += popCount(shieldPawns) * terms::KING_SHIELD_BONUS;

            // Extra bonus for center file
            if (shieldPawns & (patterns::FILE_A << kFile))
                score += terms::KING_SHIELD_BONUS / 2;

            return score;
        }

        Score evaluatePieceActivity(const GameState &state, Color c) noexcept
        {
            Score score = 0;

            // Evaluate mobility for all pieces
            for (int pt = 2; pt < 6; ++pt)
            { // Knight to Queen
                Piece piece = static_cast<Piece>(pt + (c == Color::BLACK ? 8 : 0));
                Bitboard pieces = PieceOperations::getPieces(state, piece);

                while (pieces)
                {
                    Square sq = static_cast<Square>(getLSB(pieces));
                    pieces = clearLSB(pieces);
                    score += evaluatePieceMobility(state, sq, piece);
                }
            }

            // Evaluate control of center squares
            Bitboard centerControl = patterns::CENTER_SQUARES;
            Bitboard extendedCenter = patterns::EXTENDED_CENTER;

            // Add bonuses for center control
            while (centerControl)
            {
                Square sq = static_cast<Square>(getLSB(centerControl));
                centerControl = clearLSB(centerControl);
                if (PieceOperations::isWhitePiece(state, sq) == (c == Color::WHITE))
                {
                    score += 10;
                }
            }

            while (extendedCenter)
            {
                Square sq = static_cast<Square>(getLSB(extendedCenter));
                extendedCenter = clearLSB(extendedCenter);
                if (PieceOperations::isWhitePiece(state, sq) == (c == Color::WHITE))
                {
                    score += 5;
                }
            }

            return score;
        }

        Score evaluateRookPlacement(const GameState &state, Square sq, Color c) noexcept
        {
            Score score = 0;
            const int file = static_cast<int>(sq) & 7;
            const Bitboard fileMask = patterns::FILE_A << file;

            // Bonus for rook on open file
            if (!(fileMask & PieceOperations::getPieces(state, c == Color::WHITE ? Piece::W_PAWN : Piece::B_PAWN)))
            {
                score += 20; // Open file

                // Extra bonus if the file is also open for the opponent
                if (!(fileMask & PieceOperations::getPieces(state, c == Color::WHITE ? Piece::B_PAWN : Piece::W_PAWN)))
                {
                    score += 10; // Fully open file
                }
            }

            // Bonus for rook on seventh rank
            const int rank = static_cast<int>(sq) / 8;
            if ((c == Color::WHITE && rank == 6) || (c == Color::BLACK && rank == 1))
            {
                score += 20;
            }

            return score;
        }

        Score evaluateBishopPair(const GameState &state, Color c) noexcept
        {
            Piece bishop = c == Color::WHITE ? Piece::W_BISHOP : Piece::B_BISHOP;
            return popCount(PieceOperations::getPieces(state, bishop)) >= 2;
        }

    } // namespace detail
} // namespace chess