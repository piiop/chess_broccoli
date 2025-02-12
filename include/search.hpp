#ifndef SEARCH_HPP
#define SEARCH_HPP

#include <iostream>
#include "board/board.hpp"
#include "transposition.hpp"
#include "history.hpp"
#include "movegen/movegen.hpp"
#include "time_manager.hpp"
#include "eval.hpp"
#include "board/validation.hpp"
#include "board/state.hpp"

#include <string>

namespace chess
{
    class Search
    {
    public:
        Search(GameState &state, StateManager &sm, TranspositionTable &tt, History &ht);
        void startSearch();
        void resetSearch()
        {
            nodes = 0;
            shouldStop = false;
            searchCompleted = false;
        }
        [[nodiscard]] uint64_t getNodes() const { return nodes; }
        [[nodiscard]] Move getBestMove() const
        {
            if (!searchCompleted)
            {
                return Move();
            }

            TTEntry entry;
            std::cout << "Debug (getBestMove) - Hash: " << state.hash << std::endl;
            bool found = tt.probe(state.hash, entry, 0);
            std::cout << "Debug (getBestMove) - TT probe result: " << (found ? "found" : "not found") << std::endl;

            if (found && entry.bestMove != 0)
            {
                std::cout << "BESTMOVEFOUND - Entry type: " << static_cast<int>(entry.type) << std::endl;
                std::cout << "BESTMOVEFOUND - Best move from TT: " << entry.bestMove << std::endl;
                return Move(entry.bestMove);
            }
            return Move();
        }

    private:
        TranspositionTable &tt;
        History &history;
        GameState &state;
        StateManager &stateManager;
        TimeManager timeManager;
        uint64_t nodes;
        MoveList moves;
        
        bool shouldStop;
        bool searchCompleted{false};

        int aspirationWindow(int depth, int alpha, int beta);
        int pvs(int ply, int depth, int alpha, int beta, bool doNull);
        auto quiesce(int ply, int alpha, int beta, int qDepth = 0) -> int;
        void scoreMoves(MoveList &moves, Move ttMove, int ply);
        int mvvLva(Move move) const;
        int pieceValue(int piece) const;
        bool checkStopCondition() { return timeManager.shouldStop(); }
    };
}
#endif