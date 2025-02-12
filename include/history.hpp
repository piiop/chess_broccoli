#ifndef HISTORY_HPP
#define HISTORY_HPP

#include "types.hpp"
#include "board/board.hpp"
#include <array>
#include <cstdint>

namespace chess
{
    class History
    {
        using HistoryScore = int16_t;

    public:
        History() { clear(); }

        void clear();
        HistoryScore &getValue(Color c, Piece p, Square sq);
        void update(Move move, int16_t depth);
        [[nodiscard]] bool isKiller(Move move, int ply) const;
        void updateKiller(Move move, int ply);

    private:
        static constexpr size_t NUM_COLORS = 2;
        static constexpr size_t NUM_PIECE_TYPES = 7;
        static constexpr size_t NUM_SQUARES = 64;
        static constexpr size_t MAX_PLY = 100;
        static constexpr size_t KILLERS_PER_PLY = 2;

        std::array<HistoryScore, NUM_COLORS * NUM_PIECE_TYPES * NUM_SQUARES> table{};
        std::array<Move, MAX_PLY * KILLERS_PER_PLY> killer_moves{};

        [[nodiscard]] size_t getIndex(Color c, Piece p, Square sq) const noexcept
        {
            return (static_cast<size_t>(c) * NUM_PIECE_TYPES * NUM_SQUARES) +
                   (static_cast<size_t>(p) * NUM_SQUARES) +
                   static_cast<size_t>(sq);
        }

        [[nodiscard]] size_t getKillerIndex(int ply, int killer_num) const noexcept
        {
            return ply * KILLERS_PER_PLY + killer_num;
        }
    };
} // namespace chess
#endif