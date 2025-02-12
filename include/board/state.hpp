#ifndef CHESS_BOARD_STATE_HPP
#define CHESS_BOARD_STATE_HPP

#include <cstdint>
#include <vector>
#include "../types.hpp"
#include "../movegen/move_types.hpp"
#include "../bitboard/bitboard.hpp"

namespace chess
{

    struct GameState
    {
        alignas(64) Bitboard pieces[12];
        uint8_t side_to_move : 1;
        uint8_t castling_rights : 4;
        uint8_t en_passant_file : 3;
        uint8_t halfmove_clock;
        uint16_t fullmove_number;
        uint64_t hash;

        GameState() : pieces{}, side_to_move(0), castling_rights(0b1111), en_passant_file(0),
                      halfmove_clock(0), fullmove_number(1), hash(0)
        {
            pieces[static_cast<int>(Piece::W_PAWN)] = 0x000000000000FF00ULL;
            pieces[static_cast<int>(Piece::W_ROOK)] = 0x0000000000000081ULL;
            pieces[static_cast<int>(Piece::W_KNIGHT)] = 0x0000000000000042ULL;
            pieces[static_cast<int>(Piece::W_BISHOP)] = 0x0000000000000024ULL;
            pieces[static_cast<int>(Piece::W_QUEEN)] = 0x0000000000000008ULL;
            pieces[static_cast<int>(Piece::W_KING)] = 0x0000000000000010ULL;

            pieces[static_cast<int>(Piece::B_PAWN)] = 0x00FF000000000000ULL;
            pieces[static_cast<int>(Piece::B_ROOK)] = 0x8100000000000000ULL;
            pieces[static_cast<int>(Piece::B_KNIGHT)] = 0x4200000000000000ULL;
            pieces[static_cast<int>(Piece::B_BISHOP)] = 0x2400000000000000ULL;
            pieces[static_cast<int>(Piece::B_QUEEN)] = 0x0800000000000000ULL;
            pieces[static_cast<int>(Piece::B_KING)] = 0x1000000000000000ULL;
        }
    };

    struct HistoryEntry
    {
        Move move;
        GameState prev_state;
        Piece captured_piece;
    };

    class StateManager
    {
    public:
        StateManager() { history.reserve(MAX_HISTORY_SIZE); }
        StateManager(const StateManager &other) = default;
        StateManager &operator=(const StateManager &other) = default;
        StateManager(StateManager &&other) noexcept = default;
        StateManager &operator=(StateManager &&other) noexcept = default;

        // Core state operations
        [[nodiscard]] bool makeMove(GameState &state, const Move &move) noexcept;
        bool undoMove(GameState &state) noexcept;
        [[nodiscard]] bool makeNullMove(GameState &state) noexcept;
        bool undoNullMove(GameState &state) noexcept;

        void updateCastlingRights(GameState &state, const Move &move) noexcept;

        // Hash operations
        void updateHash(GameState &state) noexcept;
        void updatePieceHash(GameState &state, Square from, Square to, Piece piece) noexcept;
        void updateCaptureHash(GameState &state, Square square, Piece captured_piece) noexcept;
        void updateCastlingHash(GameState &state, uint8_t old_rights, uint8_t new_rights) noexcept;
        void updateEnPassantHash(GameState &state, uint8_t old_file, uint8_t new_file) noexcept;

        // Special move handling
        void handleEnPassant(GameState &state, Square from, Square to) noexcept;
        void handlePromotion(GameState &state, Square to, Piece promotion_piece) noexcept;
        void handleCastling(GameState &state, Square from, Square to) noexcept;

        // History operations
        [[nodiscard]] bool hasHistory() const noexcept { return !history.empty(); }
        void pushHistory(const Move &move, const GameState &current_state, Piece captured) noexcept;
        [[nodiscard]] const std::vector<HistoryEntry> &getHistory() const noexcept { return history; }
        void clear() noexcept { history.clear(); }

        // State utilities
        [[nodiscard]] Square getEnPassantSquare(const GameState &state) const noexcept;
        [[nodiscard]] int getRepetitionCount(const GameState &state) const noexcept;

    private:
        static constexpr size_t MAX_HISTORY_SIZE = 512;
        std::vector<HistoryEntry> history;

        // Internal move validation
        void undoCastling(GameState &state, const Move &move) noexcept;
        void undoEnPassant(GameState &state, const Move &move) noexcept;
        void undoPromotion(GameState &state, const Move &move) noexcept;
    };

} // namespace chess

#endif // CHESS_BOARD_STATE_HPP