#include "../include/history.hpp"
#include "../include/board/validation.hpp"

namespace chess
{
    void History::clear()
    {
        table.fill(0);
        killer_moves.fill(Move());
    }

    History::HistoryScore &History::getValue(Color c, Piece p, Square sq)
    {
        return table[getIndex(c, p, sq)];
    }

    void History::update(Move move, int16_t depth)
    {
        auto &historyValue = getValue(move.color(), move.piece(), move.to());
        int16_t bonus = depth * depth;
        historyValue += bonus - bonus * historyValue / 512;
    }

    void History::updateKiller(Move move, int ply)
    {
        if (MoveValidation::isCapture(move))
            return;

        size_t idx1 = getKillerIndex(ply, 0);
        size_t idx2 = getKillerIndex(ply, 1);

        if (killer_moves[idx1] == move)
            return;

        killer_moves[idx2] = killer_moves[idx1];
        killer_moves[idx1] = move;
    }

    bool History::isKiller(Move move, int ply) const
    {
        if (MoveValidation::isCapture(move))
            return false;

        size_t idx1 = getKillerIndex(ply, 0);
        size_t idx2 = getKillerIndex(ply, 1);

        return move == killer_moves[idx1] || move == killer_moves[idx2];
    }
}