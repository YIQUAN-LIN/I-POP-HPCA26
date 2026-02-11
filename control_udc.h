#ifndef CONTROL_UDC_H
#define CONTROL_UDC_H

#include <vector>
#include <deque>
#include <unordered_map>
#include "champsim.h"
#include "util.h"
#include "uncore.h"
#include "fstream"

#define UDC_PF_ADDR_TABLE_SIZE 512
#define UDC_PF_ADDR_TABLE_TAG 6
#define UDC_POLL_QUEUE_SIZE 2
#define UDC_PHASE_LENTH 4096
#define UDC_BLOCK_PHASE 3
#define UDC_DEBUG_LEVEL 0

// #define FINAL_PE

#define T_NOC 2 // NoC latency, shall we need it in 1-core system?
#define T_CMD_BUS 1.66 // 1 DDR Cycle, 1.66 for (4 GHz, 4800 MT/s)
#define T_DATA_BUS 6.66 // 4 DDR Cycle
#define T_IDEAL_DRAM_ACCESS 180 // Set as a threshold (compared to 'latency'), used to tune prefetch degree 

// to track prefetch pollution
#define UDC_PO_ADDR_TABLE_SIZE 512
#define UDC_PO_ADDR_TABLE_TAG 6

#define UDC_DYNAMIC_DEGREE
#define UDC_LOW_DEGREE 2
#define UDC_MID_DEGREE 6
#define UDC_HIGH_DEGREE 10
#define UDC_DEGREE_STEP 2

using namespace std;

// DRAM_DBUS_RETURN_TIME = 10 (3200) / 13.3 (2400) / 53.3 (600) cycles 
// tRP + tRCD + tCAS = 150 cycles
extern uint32_t DRAM_DBUS_RETURN_TIME,
                tRP,  // Row Precharge (RP) latency
                tRCD, // Row address to Column address (RCD) latency
                tCAS; // Column Address Strobe (CAS) latency

struct PfAddrTableData
{
public:
    uint16_t tag; // 6 bits
    bool from_dram;
    uint32_t latency;
    BoolPrefetchers issue_prefetcher; // 1 bit * P
    void reset() {
        tag = 0;
        from_dram = false;
        latency = 0;
        for (size_t i = 0; i < NUM_PREFETCHERS; i++)
        {
            issue_prefetcher.data[i] = false;
        }
    }
};

struct PollBfData
{
public:
    uint16_t tag; // 6 bits
    BoolPrefetchers issue_prefetcher; // 1 bit * P
    void reset() {
        tag = 0;
        for (size_t i = 0; i < NUM_PREFETCHERS; i++)
        {
            issue_prefetcher.data[i] = false;
        }
    }
};

class UDC
{
public:
    PfAddrTableData pf_addr_table[UDC_PF_ADDR_TABLE_SIZE];
	PollBfData poll_bf[UDC_PO_ADDR_TABLE_SIZE];
    
    vector<int16_t> current_state;

    // count register
    uint64_t demand_count;
    uint64_t demand_miss_count;
    uint64_t pollution_count_all, pollution_count_l3[NUM_PREFETCHERS], pollution_count_dram[NUM_PREFETCHERS];
    uint64_t useful_count_l3[NUM_PREFETCHERS], useful_count_dram[NUM_PREFETCHERS];
    uint64_t fill_count[NUM_PREFETCHERS];

    // only for stat the contribution of cache pollution 
    double pollution_contribution_total, pollution_contribution_count;

    // row buffer
    uint64_t last_row_buffer_hit[DRAM_CHANNELS], last_row_buffer_miss[DRAM_CHANNELS];

	UDC(/* args */);
	~UDC();
    void prefetch_fill(uint64_t address, bool from_dram, uint32_t latency, BoolPrefetchers issue_prefetcher);
    void cache_hit(uint64_t address);
    void cache_miss_fill(uint64_t address, int hit_level);
    void prefetch_caused_eviction(uint32_t set, uint64_t evict_address, BoolPrefetchers issue_prefetcher);
    void get_control_result(float lat_l3, float lat_dram, uint64_t dram_count, uint64_t* delay_count, uint64_t* same_channel_count, uint64_t* same_bank_count);
    void reset_counters();

};
#endif /* CONTROL_UDC_H */
