#include "board/board.hpp"
#include "board/validation.hpp"
#include <cctype>
#include <charconv>
#include <iostream>

namespace chess
{

    bool Board::importFEN(const std::string_view fen)
    {
        //std::cout << "Debug - FEN import starting: " << fen << std::endl;
        // Reset board state
        std::fill_n(state.pieces, 12, 0ULL);
        state.hash = 0;
        state = {};
        state_manager.clear();

        size_t pos = 0;
        Square square = static_cast<Square>(56); // Start from a8

        // Parse piece placement
        while (pos < fen.length() && fen[pos] != ' ')
        {
            const char c = fen[pos++];

            if (std::isdigit(c))
            {
                square = static_cast<Square>(static_cast<int>(square) + (c - '0'));
                continue;
            }

            if (c == '/')
            {
                square = static_cast<Square>(static_cast<int>(square) - 16); // Move down one rank
                continue;
            }

            // Convert FEN char
            Piece piece = PieceOperations::getPieceLUT(c);
            PieceOperations::setPiece(state, square, piece);
            square = static_cast<Square>(static_cast<int>(square) + 1);
        }
        //std::cout << "Debug - Pieces placed" << std::endl;
        // Parse side to move
        if (pos >= fen.length() || fen[pos] != ' ')
            return false;
        pos++;
        if (pos >= fen.length())
            return false;

        state.side_to_move = (fen[pos] == 'b');
        //std::cout << "Debug - Validating side to move" << std::endl;
        // Parse castling rights
        if (pos + 2 >= fen.length() || fen[pos + 1] != ' ')
            return false;
        pos += 2;

        uint8_t rights = 0;
        while (pos < fen.length() && fen[pos] != ' ')
        {
            switch (fen[pos++])
            {
            case 'K':
                rights |= 0b0010;
                break;
            case 'Q':
                rights |= 0b0001;
                break;
            case 'k':
                rights |= 0b1000;
                break;
            case 'q':
                rights |= 0b0100;
                break;
            case '-':
                break;
            default:
                return false;
            }
        }
        state.castling_rights = rights;

        // Parse en passant square
        if (pos >= fen.length() || fen[pos] != ' ')
            return false;
        pos++;

        state.en_passant_file = 0; // Default to no en passant
        if (pos < fen.length() && fen[pos] != '-')
        {
            if (fen[pos] >= 'a' && fen[pos] <= 'h')
            {
                state.en_passant_file = (fen[pos] - 'a' + 1);

                // Validate that the en passant square is on the correct rank
                if (pos + 1 >= fen.length())
                    return false;
                if (state.side_to_move == 0 && fen[pos + 1] != '6')
                    return false;
                if (state.side_to_move == 1 && fen[pos + 1] != '3')
                    return false;

                pos++; // Skip the rank character
            }
            else
            {
                return false;
            }
        }
        //std::cout << "Debug - Validating halfmove" << std::endl;
        //std::cout << "Before parsing halfmove - pos: " << pos << " fen length: " << fen.length() << "\n";
        // Parse halfmove clock
        state.halfmove_clock = 0; // Default initialization is important!

        if (pos >= fen.length())
            return false;

        if (fen[pos] == '-')
        {
            pos++; // Skip the dash
        }
        else if (fen[pos] == ' ')
        {
            pos++; // Skip the space
            if (pos < fen.length())
            {
                const char *start = fen.data() + pos;
                const char *end = fen.data() + fen.length();
                int halfmove;
                auto result = std::from_chars(start, end, halfmove);
                if (result.ec == std::errc())
                { // Check if conversion was successful
                    state.halfmove_clock = static_cast<uint8_t>(halfmove);
                    pos = result.ptr - fen.data();
                }
            }
        }
        else
        {
            return false; // Invalid format
        }
        //std::cout << "After parsing halfmove - value: " << (int)state.halfmove_clock << "\n";
        //std::cout << "Debug - Validating fullmove" << std::endl;
        // Parse fullmove number
        if (pos >= fen.length() || (fen[pos] != ' ' && fen[pos] != '-'))
            return false;
        pos++;

        if (pos < fen.length())
        {
            const char *start = fen.data() + pos;
            const char *end = fen.data() + fen.length();
            int fullmove;
            auto result = std::from_chars(start, end, fullmove);
            state.fullmove_number = static_cast<uint16_t>(fullmove);
        }
        else
        {
            state.fullmove_number = 1; // Default to move 1 if not specified
        }
        //std::cout << "Debug - Running verifyPosition()" << std::endl;
        // Verify basic position validity
        if (!MoveValidation::verifyPosition(state))
            return false;

        state_manager.updateHash(state);
        return true;
    }

    std::string Board::exportFEN() const
    {
        std::string fen;
        fen.reserve(90); // Reserve space for typical FEN string length

        // Piece placement
        for (int rank = 7; rank >= 0; --rank)
        {
            int empty = 0;

            for (int file = 0; file < 8; ++file)
            {
                const Square square = static_cast<Square>(rank * 8 + file);
                const Piece piece = PieceOperations::getPiece(state, square);

                if (piece == Piece::EMPTY)
                {
                    empty++;
                    continue;
                }

                if (empty > 0)
                {
                    fen += static_cast<char>('0' + empty);
                    empty = 0;
                }

                // Convert piece to FEN character
                const bool is_white = !static_cast<bool>(static_cast<int>(piece) & 8);
                const int piece_type = static_cast<int>(piece) & 7;
                static constexpr char PIECE_CHARS[] = {'?', 'P', 'N', 'B', 'R', 'Q', 'K'};
                char c = PIECE_CHARS[piece_type];

                fen += is_white ? c : static_cast<char>(std::tolower(c));
            }

            if (empty > 0)
            {
                fen += static_cast<char>('0' + empty);
            }

            if (rank > 0)
                fen += '/';
        }

        // Side to move
        fen += state.side_to_move ? " b " : " w ";

        // Castling rights
        bool has_castling = false;
        if (state.castling_rights & 0b0010)
        {
            fen += 'K';
            has_castling = true;
        }
        if (state.castling_rights & 0b0001)
        {
            fen += 'Q';
            has_castling = true;
        }
        if (state.castling_rights & 0b1000)
        {
            fen += 'k';
            has_castling = true;
        }
        if (state.castling_rights & 0b0100)
        {
            fen += 'q';
            has_castling = true;
        }
        if (!has_castling)
            fen += '-';

        // En passant square
        fen += ' ';
        if (state.en_passant_file)
        {
            fen += static_cast<char>('a' + state.en_passant_file - 1);
            fen += state.side_to_move ? '3' : '6';
        }
        else
        {
            fen += '-';
        }

        // Move counts
        fen += ' ' + std::to_string(state.halfmove_clock);
        fen += ' ' + std::to_string(state.fullmove_number);

        return fen;
    }

} // namespace chess