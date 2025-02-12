#include <iostream>
#include <string>
#include <chrono>
#include <memory>
#include "include/types.hpp"
#include "include/board/board.hpp"
#include "include/board/state.hpp"
#include "include/search.hpp"
#include "include/eval.hpp"
#include "include/history.hpp"
#include "include/transposition.hpp"

namespace
{
    constexpr int MAX_DEPTH = 64;

    std::unique_ptr<chess::Board> board = nullptr;
    std::unique_ptr<chess::Search> search = nullptr;
    std::unique_ptr<chess::StateManager> stateManager = nullptr;
    auto &tt = TranspositionTable::getInstance(0x100000);
}

class Engine
{
public:
    Engine()
    {
        chess::initializeMagicBitboards();
        board = std::make_unique<chess::Board>();
        history = std::make_unique<chess::History>();
        stateManager = std::make_unique<chess::StateManager>();

        // Pre-warm the transposition table
        tt.clear();

        // Initialize search with a dummy depth-1 search
        search = std::make_unique<chess::Search>(gameState, *stateManager, tt, *history);
        search->startSearch(); // Depth 1 search to warm up caches
        search->resetSearch();
    }

    void set_position(const std::string &fen)
    {
        
        try
        {
            board->importFEN(fen);
            
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: Failed to set position: " << e.what() << std::endl;
        }
    }

    chess::Move get_best_move()
    {
        std::cout << "Debug (main): Starting search for best move" << std::endl;
        auto start_time = std::chrono::high_resolution_clock::now();
        search->startSearch();
        auto best_move = search->getBestMove();
        search->resetSearch();
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        std::cout << "Debug (main): Search completed in " << duration.count() << "ms" << std::endl;
        return best_move;
    }

    std::string move_to_uci(chess::Move move)
    {
        if (move == chess::Move())
        {
            std::cout << "Debug (main): Null move detected" << std::endl;
            return "0000";
        }
        auto uci = board->toUCI(move);
        std::cout << "Debug (main): Converting move to UCI: " << uci << std::endl;
        return uci;
    }

private:
    std::unique_ptr<chess::History> history;
    chess::GameState gameState;
    chess::Move parse_uci_move(const std::string &move_str)
    {
        std::cout << "Debug: Parsing UCI move: " << move_str << std::endl;
        if (move_str.length() < 4)
        {
            std::cout << "Debug: Invalid move string length" << std::endl;
            return chess::Move();
        }
        auto move = board->fromUCI(move_str);
        return move;
    }
};

static Engine *engine = nullptr;

extern "C"
{
    void init_engine()
    {
        if (!engine)
            engine = new Engine();
    }

    void cleanup_engine()
    {
        delete engine;
        engine = nullptr;
    }

    const char *get_best_move(const char *fen)
    {
        
        if (!engine)
        {
            std::cout << "Debug (main): Engine not initialized, creating new instance" << std::endl;
            init_engine();
        }

        engine->set_position(fen);
        chess::Move best_move = engine->get_best_move();

        static std::string move_str;
        move_str = engine->move_to_uci(best_move);
        std::cout << "Debug (main): Returning best move: " << move_str << std::endl;
        return move_str.c_str();
    }
}

int main()
{
    std::string command;
    init_engine();

    while (std::getline(std::cin, command))
    {
    
        if (command == "quit")
            break;
        else if (command == "uci")
        {
            std::cout << "id name ChessBroccoli" << std::endl;
            std::cout << "uciok" << std::endl;
            std::cout.flush();
        }
        else if (command.substr(0, 8) == "position")
        {
            if (command.substr(9, 3) == "fen")
            {
                std::string fen = command.substr(13);
                engine->set_position(fen);
            }
        }
        else if (command.substr(0, 2) == "go")
        {
            chess::Move best_move = engine->get_best_move();
            std::string move_str = engine->move_to_uci(best_move);
            std::cout << "bestmove (main): " << move_str << std::endl;
            std::cout.flush();
        }
    }

    cleanup_engine();
    return 0;
}