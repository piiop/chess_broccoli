#include "../include/search.hpp"
#include "../include/movegen/movegen.hpp"
#include <algorithm>

namespace
{
    constexpr int MATE_SCORE = 30000;
    constexpr int MATE_THRESHOLD = 29000;
    constexpr int MAX_PLY = 100;
    constexpr int FUTILITY_MARGIN = 100;
    constexpr int DELTA_MARGIN = 200;
    constexpr int LMR_DEPTH = 3;
    constexpr int LMR_MOVES = 4;
    constexpr int NULL_MOVE_R = 2;
    constexpr int REVERSE_FUTILITY_MARGIN = 500;
    constexpr int LMR_BASE = 1;
    constexpr int LMR_DIVISOR = 8;
    constexpr int LMR_MAX = 3;
    constexpr int MAX_QDEPTH = 10;

    inline int mateIn(int ply) { return MATE_SCORE - ply; }
    inline int matedIn(int ply) { return -MATE_SCORE + ply; }
    inline bool isMateScore(int score) { return std::abs(score) >= MATE_THRESHOLD; }
}

namespace chess
{
    Search::Search(GameState &state, StateManager &sm, TranspositionTable &tt, History &ht)
        : tt(tt), history(ht), state(state), stateManager(sm), timeManager(), nodes(0), moves(), shouldStop(false)
    {
        moves.reserve(218);
    }

    void Search::startSearch()
    {
        nodes = 0;
        searchCompleted = false;
        tt.incrementAge();
        timeManager.setTimeControls();
        timeManager.startTimer();

        int alpha = -MATE_SCORE;
        int beta = MATE_SCORE;
        int delta = 50;

        for (int depth = 1; depth <= MAX_PLY; ++depth)
        {
            if (timeManager.shouldStop())
            {
                searchCompleted = true;
                return;
            }

            int score = aspirationWindow(depth, alpha, beta);

            if (score <= alpha || score >= beta)
            {
                if (timeManager.shouldStop())
                {
                    searchCompleted = true;
                    return;
                }
                alpha = -MATE_SCORE;
                beta = MATE_SCORE;
                score = aspirationWindow(depth, alpha, beta);
            }
            else
            {
                alpha = score - delta;
                beta = score + delta;
                delta = delta + delta / 2;
            }

            if (timeManager.shouldStop() || isMateScore(score))
            {
                searchCompleted = true;
                return;
            }
        }
        searchCompleted = true;
    }

    int Search::aspirationWindow(int depth, int alpha, int beta)
    {
        return pvs(0, depth, alpha, beta, true);
    }

    int Search::pvs(int ply, int depth, int alpha, int beta, bool doNull)
    {
        nodes++; // Increment node counter
        if ((nodes & 1023) == 0)
        {
            std::cout << "Node check at " << nodes << " nodes. Time exceeded: " << timeManager.shouldStop() << std::endl;
            if (timeManager.shouldStop())
            {
                return 0;
            }
        }

        // Base cases
        if (depth <= 0)
        {
            return quiesce(ply, alpha, beta);
        }
        std::cout << "Debug (pvs) - depth good: continue" << std::endl;
        if (ply > MAX_PLY)
        {
            return evaluatePosition(state);
        }
        std::cout << "Debug (pvs)- MAX_PLY good: continue" << std::endl;
        // Early exit conditions
        std::cout << "Debug (pvs)- time check:" << std::endl;
        if (timeManager.shouldStop())
        {
            std::cout << "Debug (pvs) - Time stop - return 0" << std::endl;
            return 0;
        }
        std::cout << "Debug - time good" << std::endl;
        if (MoveValidation::isDraw(state, stateManager))
        {
            std::cout << "Debug (pvs) - Draw found - return 0" << std::endl;
            return 0;
        }
        std::cout << "Debug - Base cases clear" << std::endl;
        const bool inCheck = MoveValidation::isInCheck(state);
        std::cout << "Debug (pvs) - incheck: " << inCheck << std::endl;
        const bool isPv = (beta - alpha) > 1;
        std::cout << "Debug (pvs) - isPV: " << isPv << std::endl;
        // Transposition table lookup
        TTEntry ttEntry;
        const bool ttHit = tt.probe(state.hash, ttEntry, ply);

        std::cout << "Debug (pvs) - TT probe result: " << ttHit << std::endl;

        if (ttHit && ttEntry.depth >= depth && !isPv)
        {
            if (ttEntry.type == EntryType::EXACT)
            {
                return scoreFromTT(ttEntry.score, ply);
            }
            if (ttEntry.type == EntryType::ALPHA && ttEntry.score <= alpha)
            {
                return alpha;
            }
            if (ttEntry.type == EntryType::BETA && ttEntry.score >= beta)
            {
                return beta;
            }
        }

        // Static null move pruning (reverse futility pruning)
        if (!isPv && !inCheck && depth <= 3)
        {
            int eval = evaluatePosition(state);
            if (eval - REVERSE_FUTILITY_MARGIN * depth >= beta)
            {
                return eval;
            }
        }
        // Null Move Pruning
        if (doNull && !isPv && !inCheck && depth >= 3 && !MoveValidation::isInsufficientMaterial(state))
        {
            bool success = stateManager.makeNullMove(state);
            if (success)
            {
                int score = -pvs(ply + 1, depth - NULL_MOVE_R - 1, -beta, -beta + 1, false);
                stateManager.undoNullMove(state);

                if (score >= beta)
                {
                    return beta;
                }
            }
        }

        // Internal iterative deepening
        if (!ttHit && depth >= 4)
        {
            pvs(ply, depth - 2, alpha, beta, false);
            // Probe again after IID
            tt.probe(state.hash, ttEntry, ply);
        }

        // Initialize search variables
        int bestScore = -MATE_SCORE;
        Move bestMove;
        if (ttHit)
        {
            bestMove.data = ttEntry.bestMove;
        }
        int movesSearched = 0;
        bool foundPv = false;

        // Generate and score moves
        MoveList moves;
        moves = MoveValidation::getLegalMoves(state);
        scoreMoves(moves, bestMove, ply);
        std::cout << "Debug pvs - Scored " << moves.size() << " moves at depth " << depth << ", ply " << ply << std::endl;
        
        // Search parameters
        const int origDepth = depth;
        bool futilityPruning = !isPv && !inCheck && depth <= 3;
        int futilityBase = evaluatePosition(state) + FUTILITY_MARGIN * depth;

        // Main move loop
        for (const auto &scoredMove : moves)
        {
            std::cout << "Debug (pvs): main move loop entered" << std::endl;
            const auto &move = scoredMove.move;

            if (!stateManager.makeMove(state, move))
            {
                std::cout << "Move failed validation - from: " << static_cast<int>(move.from())
                          << " to: " << static_cast<int>(move.to()) << std::endl;
                continue;
            }

            const bool givesCheck = MoveValidation::isInCheck(state);
            depth = givesCheck ? origDepth : origDepth - 1;

            int score;

            if (movesSearched == 0)
            {
                std::cout << "First move PVS - from: " << static_cast<int>(move.from())
                          << " to: " << static_cast<int>(move.to()) << std::endl;
                score = -pvs(ply + 1, depth - 1, -beta, -alpha, true);
            }
            else
            {
                int reduction = 0;
                if (depth >= LMR_DEPTH && movesSearched >= LMR_MOVES &&
                    !MoveValidation::isCapture(move) && !givesCheck && !inCheck)
                {
                    reduction = LMR_BASE + std::min(movesSearched / LMR_DIVISOR, LMR_MAX);
                    reduction += !isPv;
                    reduction = std::min(depth - 1, reduction);
                    std::cout << "LMR candidate - from: " << static_cast<int>(move.from())
                              << " to: " << static_cast<int>(move.to())
                              << " reduction: " << reduction << std::endl;

                    if (history.getValue(move.color(), move.piece(), move.to()) < 0)
                    {
                        reduction++;
                        std::cout << "Extra history reduction - from: " << static_cast<int>(move.from())
                                  << " to: " << static_cast<int>(move.to()) << std::endl;
                    }
                }

                if (futilityPruning && !MoveValidation::isCapture(move) &&
                    !(move.getFlags() & Move::PROMOTION_FLAG) && futilityBase <= alpha)
                {
                    std::cout << "Futility pruning - from: " << static_cast<int>(move.from())
                              << " to: " << static_cast<int>(move.to())
                              << " score: " << futilityBase << " alpha: " << alpha << std::endl;
                    stateManager.undoMove(state);
                    continue;
                }

                if (movesSearched >= depth * 4 && depth <= 3)
                {
                    stateManager.undoMove(state);
                    continue;
                }

                int newDepth = depth - 1 - reduction;
                std::cout << "Zero window search - from: " << static_cast<int>(move.from())
                          << " to: " << static_cast<int>(move.to())
                          << " depth: " << newDepth << std::endl;
                score = -pvs(ply + 1, newDepth, -alpha - 1, -alpha, true);

                if (score > alpha && (reduction > 0 || score < beta))
                {
                    score = -pvs(ply + 1, depth - 1, -beta, -alpha, true);
                }
                if (timeManager.shouldStop())
                {
                    stateManager.undoMove(state); // Add undo before return
                    return 0;
                }
            }

            stateManager.undoMove(state);
            movesSearched++;

            if (timeManager.shouldStop())
            {
                std::cout << "Time manager stop signal received" << std::endl;
                return 0;
            }

            std::cout << "Move from: " << static_cast<int>(move.from())
                      << " to: " << static_cast<int>(move.to()) << " score: " << score
                      << " best: " << bestScore << " a/b: " << alpha << "/" << beta << std::endl;

            if (score > bestScore)
            {
                bestScore = score;
                bestMove = move;

                if (score > alpha)
                {
                    alpha = score;
                    if (score >= beta)
                    {
                        std::cout << "Beta cutoff - from: " << static_cast<int>(move.from())
                                  << " to: " << static_cast<int>(move.to()) << std::endl;
                        if (!MoveValidation::isCapture(move))
                        {
                            history.update(move, depth);
                            history.updateKiller(move, ply);
                        }
                        tt.store(state.hash, origDepth, beta,
                                 EntryType::BETA, move.data, ply);
                        return beta;
                    }
                }
            }
        }
        // Handle no legal moves
        if (movesSearched == 0)
        {
            return inCheck ? matedIn(ply) : 0;
        }

        // Store position in transposition table
        EntryType bound = (bestScore <= alpha) ? EntryType::ALPHA : (bestScore >= beta) ? EntryType::BETA
                                                                                        : EntryType::EXACT;

        int adjustedScore = isMateScore(bestScore) ? (bestScore > 0 ? bestScore + ply : bestScore - ply) : bestScore;

        std::cout << "TT Store - Hash: " << state.hash
                  << " Depth: " << origDepth
                  << " Score: " << adjustedScore
                  << " Bound: " << static_cast<int>(bound)
                  << " Move: " << bestMove.data
                  << " Ply: " << ply << std::endl;

        tt.store(state.hash, origDepth, adjustedScore, bound, bestMove.data, ply);

        return bestScore;
    }

    void Search::scoreMoves(MoveList &moves, Move ttMove, int ply)
    {
        for (auto &move : moves)
        {
            int score = 0;
            if (move.move == ttMove)
                score = 30000;
            else if (MoveValidation::isCapture(move.move))
                score = 20000 + mvvLva(move.move);
            else if (MoveValidation::isPromotion(move.move))
                score = 19000 + pieceValue(static_cast<int>(move.move.promotion()));
            else if (history.isKiller(move.move, ply))
                score = 15000;
            else
                score = history.getValue(move.move.color(), move.move.piece(), move.move.to());
            move.score = score;
        }
        std::sort(moves.begin(), moves.end(), std::greater<ScoredMove>());
    }

    int Search::mvvLva(Move move) const
    {
        static const int MVV_LVA[15][15] = {
            // Create a 15x15 matrix to accommodate piece values 0-14
            // Index represents piece type (0-14)
            // Value represents capture priority
            // Higher value = better capture
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},             // EMPTY captures (impossible)
            {0, 15, 14, 14, 13, 12, 11, 0, 15, 14, 14, 13, 12, 11, 0}, // W_PAWN captures
            {0, 25, 24, 24, 23, 22, 21, 0, 25, 24, 24, 23, 22, 21, 0}, // W_KNIGHT captures
            {0, 35, 34, 34, 33, 32, 31, 0, 35, 34, 34, 33, 32, 31, 0}, // W_BISHOP captures
            {0, 45, 44, 44, 43, 42, 41, 0, 45, 44, 44, 43, 42, 41, 0}, // W_ROOK captures
            {0, 55, 54, 54, 53, 52, 51, 0, 55, 54, 54, 53, 52, 51, 0}, // W_QUEEN captures
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},             // W_KING captures
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},             // Gap
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},             // Gap
            {0, 15, 14, 14, 13, 12, 11, 0, 15, 14, 14, 13, 12, 11, 0}, // B_PAWN captures
            {0, 25, 24, 24, 23, 22, 21, 0, 25, 24, 24, 23, 22, 21, 0}, // B_KNIGHT captures
            {0, 35, 34, 34, 33, 32, 31, 0, 35, 34, 34, 33, 32, 31, 0}, // B_BISHOP captures
            {0, 45, 44, 44, 43, 42, 41, 0, 45, 44, 44, 43, 42, 41, 0}, // B_ROOK captures
            {0, 55, 54, 54, 53, 52, 51, 0, 55, 54, 54, 53, 52, 51, 0}, // B_QUEEN captures
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}              // B_KING captures
        };

        int victim = static_cast<int>(PieceOperations::getCapturedPiece(state, move));
        int attacker = static_cast<int>(move.piece());
        return MVV_LVA[attacker][victim];
    }

    int Search::pieceValue(int piece) const
    {
        static const int values[6] = {100, 300, 300, 500, 900, 0};
        return values[piece & 7];
    }

    auto Search::quiesce(int ply, int alpha, int beta, int qDepth) -> int
    {
        moves.clear();
        nodes++;

        // Add depth limit
        if (qDepth >= MAX_QDEPTH)
            return evaluatePosition(state);static int maxDepth = 0;
    maxDepth = std::max(maxDepth, qDepth);
    if (qDepth % 100 == 0) {
        //std::cout << "Current depth: " << qDepth << " Max: " << maxDepth << std::endl;
    }

        int standPat = evaluatePosition(state);
        if ((nodes & 1023) == 0 && timeManager.shouldStop())
        {
            return standPat;
        }
        if (standPat >= beta)
            return beta;

        // Delta pruning
        if (standPat + DELTA_MARGIN < alpha)
            return alpha;

        if (standPat > alpha)
            alpha = standPat;

        MoveList moves;
        generateQuiescenceMoves(moves, state);
        for (const auto &move : moves)
        {
            if (!stateManager.makeMove(state, move.move))
                continue;
            int score = -quiesce(ply + 1, -beta, -alpha, qDepth + 1);
            stateManager.undoMove(state);

            if (score >= beta)
                return beta;
            if (score > alpha)
                alpha = score;
        }

        return alpha;
    }
    
}