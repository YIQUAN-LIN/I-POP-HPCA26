#ifndef ALECTO_H
#define ALECTO_H

#include <vector>
#include <deque>
#include "champsim.h"

#define ALECTO_ALLCATION_TABLE_SIZE 64
#define ALECTO_SAMPLE_TABLE_SIZE 64
#define ALECTO_SANDBOX_TABLE_SIZE 512
#define ALECTO_PERIOD 100
#define ALECTO_DEAD_THRESH 150
#define ALECTO_DEBUG_LEVEL 0

using namespace std;

struct AllocationTableData
{
    bool valid;
    uint16_t tag; // 9 bits
    int8_t states[NUM_PREFETCHERS]; // 4 bits * P 
};

struct SampleTableData
{
    bool valid;
    uint16_t tag; // 9 bits
    uint8_t demands; // 8 bits
    uint8_t deads; // 7 bits
    uint16_t issued[NUM_PREFETCHERS]; // 8 bits * P 
    uint16_t confirmed[NUM_PREFETCHERS]; // 8 bits * P 
    void reset() {
        memset(this, 0, sizeof(SampleTableData));
    }
};

struct SandboxTableData
{
public:
    uint8_t tag; // 6 bits
    bool issued[NUM_PREFETCHERS]; // 1 bit * P
    uint64_t pc[NUM_PREFETCHERS]; // 9 bits * P
    void reset() {
        memset(this, 0, sizeof(SandboxTableData));
    }
};

class Alecto
{
public:
    AllocationTableData allocation_table[ALECTO_ALLCATION_TABLE_SIZE];
    SampleTableData sample_table[ALECTO_SAMPLE_TABLE_SIZE];
    SandboxTableData sandbox_table[ALECTO_SANDBOX_TABLE_SIZE];

    Alecto(/* args */);
    ~Alecto();
    vector<int8_t> Access(uint64_t pc, uint64_t address);
    vector<uint64_t> IssuePrefetch(uint64_t pc, bool dead, vector<vector<uint64_t>> &pref_addr);
    bool CheckSandboxTable(uint64_t address);
    bool SampleTableIncrease(uint64_t pc, uint8_t prefetcher_index);
    void EndOfPhase(uint16_t index, SampleTableData const &data);
    void Dead(uint16_t index);
    uint64_t xor_fold(uint64_t key, int index_len);
};

#endif /* ALECTO_H */
