#ifndef CHESS_BOARD_HPP
#define CHESS_BOARD_HPP

#include <string>
#include <string_view>
#include "../types.hpp"
#include "../bitboard/bitboard.hpp"
#include "../movegen/move_types.hpp"
#include "state.hpp"
#include "../transposition.hpp"
#include "../eval.hpp"
#include "piece_ops.hpp"

namespace chess
{

    class Board
    {
    public:
        Board();
        // FEN operations
        bool importFEN(std::string_view fen);
        std::string exportFEN() const;

        // UCI operations
        [[nodiscard]] static std::string toUCI(const Move &move) noexcept;
        [[nodiscard]] Move fromUCI(const std::string &uci) const noexcept;

    private:
        alignas(64) StateManager state_manager;
        GameState state;
    };

} // namespace chess

#endif // CHESS_BOARD_HPP