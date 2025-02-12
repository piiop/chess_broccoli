#ifndef TRANS_HPP
#define TRANS_HPP

#include <cstdint>
#include "types.hpp"

constexpr int MAX_MATE_SCORE = 30000;

enum class EntryType : uint8_t
{
    EXACT = 0,
    ALPHA = 1, 
    BETA = 2,
    NONE = 3
};

struct TTEntry
{
    uint64_t key;      // 8 bytes: Zobrist hash key
    uint16_t bestMove; // 2 bytes: Best move in position
    int16_t score;     // 2 bytes: Evaluation score
    uint8_t depth;     // 1 byte:  Search depth
    EntryType type;    // 1 byte:  Entry type (enum)
    uint8_t age;       // 1 byte:  For replacement strategy
    uint8_t gen;       // 1 byte:  Generation, for aging entries

    TTEntry() : key(0), bestMove(0), score(0), depth(0),
                type(EntryType::NONE), age(0), gen(0) {}
};

struct TTBucket
{
    static constexpr int BUCKET_SIZE = 4;
    TTEntry entries[BUCKET_SIZE];

    TTBucket()
    {
        for (int i = 0; i < BUCKET_SIZE; ++i)
        {
            entries[i].type = EntryType::NONE;
        }
    }
};

class TranspositionTable
{
public:
    static TranspositionTable &getInstance(size_t bytes = DEFAULT_SIZE)
    {
        static TranspositionTable instance(bytes);
        return instance;
    }

    // 
    TranspositionTable(const TranspositionTable &) = delete;
    TranspositionTable &operator=(const TranspositionTable &) = delete;
    TranspositionTable(TranspositionTable &&) = delete;
    TranspositionTable &operator=(TranspositionTable &&) = delete;

    void store(uint64_t key, int depth, int score,
               EntryType type, uint16_t bestMove, int ply);
    bool probe(uint64_t key, TTEntry &entry, int ply) const;
    void clear();
    void incrementAge();

    size_t getSize() const { return numEntries; }
    size_t getUsed() const { return used; }

private:
    static constexpr size_t MB = 1024 * 1024;
    static constexpr size_t DEFAULT_SIZE = MB;
    static constexpr int BUCKET_SIZE = 4;
    static constexpr int AGE_MAX = 255;

    TTBucket *table;
    size_t numEntries;
    uint8_t generation;
    size_t used;

    TranspositionTable(size_t bytes);
    ~TranspositionTable();

    size_t index(uint64_t key) const { return key % numEntries; }
    void aging();
    int calculateReplaceValue(const TTEntry &entry) const;
};

inline int scoreToTT(int score, int ply)
{
    return score >= MAX_MATE_SCORE ? score + ply : score <= -MAX_MATE_SCORE ? score - ply
                                                                            : score;
}

inline int scoreFromTT(int score, int ply)
{
    return score >= MAX_MATE_SCORE ? score - ply : score <= -MAX_MATE_SCORE ? score + ply
                                                                            : score;
}
#endif