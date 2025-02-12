#include "board/board.hpp"
#include "zobrist_keys.hpp"
#include <algorithm>
#include <iostream>

namespace chess
{
    Board::Board() : state_manager(), state{}
    {

        state_manager.updateHash(state);
    }

    Move Board::fromUCI(const std::string &uci) const noexcept
    {
        if (uci.length() < 4 || uci.length() > 5)
            return Move();

        const Square from = static_cast<Square>((uci[0] - 'a') + (8 - (uci[1] - '0')) * 8);
        const Square to = static_cast<Square>((uci[2] - 'a') + (8 - (uci[3] - '0')) * 8);

        Piece promotion = Piece::EMPTY;
        if (uci.length() == 5)
        {
            const bool is_white = PieceOperations::getPiece(state, from) == Piece::W_PAWN;
            switch (uci[4])
            {
            case 'q':
                promotion = is_white ? Piece::W_QUEEN : Piece::B_QUEEN;
                break;
            case 'r':
                promotion = is_white ? Piece::W_ROOK : Piece::B_ROOK;
                break;
            case 'b':
                promotion = is_white ? Piece::W_BISHOP : Piece::B_BISHOP;
                break;
            case 'n':
                promotion = is_white ? Piece::W_KNIGHT : Piece::B_KNIGHT;
                break;
            }
        }
        const Piece piece = PieceOperations::getPiece(state, from);
        const Color color = PieceOperations::isWhitePiece(state, from) ? Color::WHITE : Color::BLACK;
        return Move(from, to, color, piece, promotion);
    }

    std::string Board::toUCI(const Move &move) noexcept
    {
        if (move.data == 0)
            return "0000";

        std::string uci;
        uci.reserve(5);

        const int from_file = static_cast<int>(move.from()) % 8;
        const int from_rank = static_cast<int>(move.from()) / 8;
        const int to_file = static_cast<int>(move.to()) % 8;
        const int to_rank = static_cast<int>(move.to()) / 8;

        uci += 'a' + from_file;
        uci += '1' + from_rank;
        uci += 'a' + to_file;
        uci += '1' + to_rank;

        if (move.promotion() != Piece::EMPTY)
        {
            switch (move.promotion())
            {
            case Piece::W_QUEEN:
            case Piece::B_QUEEN:
                uci += 'q';
                break;
            case Piece::W_ROOK:
            case Piece::B_ROOK:
                uci += 'r';
                break;
            case Piece::W_BISHOP:
            case Piece::B_BISHOP:
                uci += 'b';
                break;
            case Piece::W_KNIGHT:
            case Piece::B_KNIGHT:
                uci += 'n';
                break;
            default:
                break;
            }
        }

        return uci;
    }  
      

} // namespace chess