#include "../include/transposition.hpp"
#include <cstring>
#include <algorithm>
#include <climits>
#include <iostream>

TranspositionTable::TranspositionTable(size_t bytes) : generation(0), used(0)
{
    // Calculate number of buckets
    numEntries = (bytes / sizeof(TTBucket));
    numEntries = std::max(size_t(256), numEntries); // Adjusted minimum size

    // Allocate and clear the table
    table = new TTBucket[numEntries];
    clear();
}

TranspositionTable::~TranspositionTable()
{
    delete[] table;
}

void TranspositionTable::clear()
{
    constexpr size_t CHUNK_SIZE = 1024; // Adjust based on cache line size
    TTEntry empty;

    for (size_t i = 0; i < numEntries; i += CHUNK_SIZE)
    {
        size_t chunk = std::min(CHUNK_SIZE, numEntries - i);
        std::fill_n(&table[i].entries[0], chunk * BUCKET_SIZE, empty);
    }
    used = generation = 0;
}

void TranspositionTable::incrementAge()
{
    generation++;
    if (generation >= AGE_MAX)
    {
        aging();
    }
}

void TranspositionTable::aging()
{
    for (size_t i = 0; i < numEntries; ++i)
    {
        for (int j = 0; j < TTBucket::BUCKET_SIZE; ++j)
        {
            if (table[i].entries[j].gen != generation)
            {
                table[i].entries[j].type = EntryType::NONE;
            }
        }
    }
    generation = 1;
}

bool TranspositionTable::probe(uint64_t key, TTEntry &entry, int ply) const
{
    size_t idx = index(key);
    const TTBucket &bucket = table[idx];

    //std::cout << "Debug - TT probe: key=" << key << " index=" << idx << std::endl;

    // Search all entries in the bucket
    for (int i = 0; i < BUCKET_SIZE; ++i)
    {
        const TTEntry &slot = bucket.entries[i];
        //std::cout << "Debug - Checking slot " << i
        //          << ": key=" << slot.key
        //          << " move=" << slot.bestMove
        //         << " type=" << static_cast<int>(slot.type) << std::endl;

        if (slot.key == key && slot.type != EntryType::NONE)
        {
            entry = slot;
            entry.score = scoreFromTT(entry.score, ply); // Add this line
            return true;
        }
    }
    //std::cout << "Debug - No valid entry found" << std::endl;
    return false;
}

void TranspositionTable::store(uint64_t key, int depth, int score,
                               EntryType type, uint16_t bestMove, int ply)
{
    
    size_t idx = index(key);
    TTBucket &bucket = table[idx];

    // Prepare new entry
    TTEntry newEntry;
    newEntry.key = key;
    newEntry.score = static_cast<int16_t>(scoreToTT(score, ply));
    newEntry.bestMove = bestMove;
    newEntry.depth = static_cast<uint8_t>(depth);
    newEntry.type = type;
    newEntry.age = 0;
    newEntry.gen = generation;

    // Find the best slot to replace
    int replace_idx = 0;
    int lowest_value = INT_MAX;

    for (int i = 0; i < BUCKET_SIZE; ++i)
    {
        const TTEntry &slot = bucket.entries[i];

        // Found an empty slot
        if (slot.type == EntryType::NONE)
        {
            replace_idx = i;
            break;
        }

        // Found existing entry
        if (slot.key == key)
        {
            replace_idx = i;
            break;
        }

        // Calculate replacement value
        int value = calculateReplaceValue(slot);
        if (value < lowest_value)
        {
            lowest_value = value;
            replace_idx = i;
        }
    }

    // Update used count if replacing empty entry
    if (bucket.entries[replace_idx].type == EntryType::NONE)
    {
        used++;
    }
    bucket.entries[replace_idx] = newEntry;
    std::cout << "Debug - Bucket values after store:"
              << " key=" << bucket.entries[replace_idx].key
              << " depth=" << (int)bucket.entries[replace_idx].depth
              << " type=" << static_cast<int>(bucket.entries[replace_idx].type) << std::endl;
}

// Helper method for replacement strategy
int TranspositionTable::calculateReplaceValue(const TTEntry &entry) const
{
        int value = entry.depth * 2;
        if (entry.gen == generation)
            value += 2;
        if (entry.type == EntryType::EXACT)
            value += 1;
        return value;
}