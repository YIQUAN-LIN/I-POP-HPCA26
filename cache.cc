void CACHE::handle_fill()
{
    /*
        ......
        if (do_fill) {
            ......
    */
    
    /*
        The following code is the modified code for the function `CACHE::handle_fill()`, added within the `"if (do_fill) {"` block.  

        It is primarily used to collect statistics after data is returned to the MSHR:  

        1. **Latency, used to calculate average latency.  
        2. **Counting of `delay_count`, `delay_same_channel`, and `delay_same_bank`** (corresponding to the counters N in Equation 4 of the paper), used for PE computation.
    */
    
    if (warmup_complete[fill_cpu])
    {
        MSHR.entry[mshr_index].cycle_fill_cache = current_core_cycle[cpu];
        uint64_t current_miss_latency = MSHR.entry[mshr_index].cycle_fill_cache - MSHR.entry[mshr_index].cycle_add_mshr;
        total_miss_latency += current_miss_latency;

        if (MSHR.entry[mshr_index].hit_dram)
        {
            hit_dram_latency += current_miss_latency;
            hit_dram_count++;
        }
        else
        {
            hit_l3_latency += current_miss_latency;
            hit_l3_count++;
        }
    }

    if (MSHR.entry[mshr_index].hit_dram && MSHR.entry[mshr_index].type == PREFETCH)
    {
        uint64_t channel_mask = DRAM_CHANNELS - 1;
        uint64_t channel_tag = MSHR.entry[mshr_index].address & channel_mask;
        uint64_t bank_mask = ((DRAM_RANKS - 1) << (LOG2_DRAM_COLUMNS + LOG2_DRAM_BANKS + LOG2_DRAM_CHANNELS)) | (DRAM_CHANNELS * DRAM_BANKS - 1);
        uint64_t bank_tag = MSHR.entry[mshr_index].address & bank_mask;
        int delay_type = -1;
        for (uint32_t index = 0; index < MSHR_SIZE; index++)
        {
            if (index == mshr_index)
                continue;
            // have not determine whether the demand hit dram
            if (MSHR.entry[index].address && MSHR.entry[index].type == LOAD && MSHR.entry[index].returned == INFLIGHT)
            {
                if ((MSHR.entry[index].address & bank_mask) == bank_tag)
                {
                    delay_type = 3;
                    break;
                }
                else if (delay_type < 1 && (MSHR.entry[index].address & channel_mask) == channel_tag)
                {
                    delay_type = 2;
                }
                else
                {
                    delay_type = 1;
                }
            }
        }
        if (delay_type == 3)
        {
            for (size_t i = 0; i < NUM_PREFETCHERS; i++)
            {
                if (MSHR.entry[mshr_index].issue_prefetcher.data[i])
                    delay_same_bank[i]++;
            }
        }
        else if (delay_type == 2)
        {
            for (size_t i = 0; i < NUM_PREFETCHERS; i++)
            {
                if (MSHR.entry[mshr_index].issue_prefetcher.data[i])
                    delay_same_channel[i]++;
            }
        }
        else if (delay_type == 1)
        {
            for (size_t i = 0; i < NUM_PREFETCHERS; i++)
            {
                if (MSHR.entry[mshr_index].issue_prefetcher.data[i])
                    delay_count[i]++;
            }
        }
    }

    /*
        ......
    */
}


float CACHE::get_l3_latency()
{
    if (hit_l3_count)
        return (float)hit_l3_latency / hit_l3_count;
    return 0;
}

float CACHE::get_dram_latency()
{
    if (hit_dram_count)
        return (float)hit_dram_latency / hit_dram_count;
    return 0;
}

uint64_t CACHE::get_dram_count()
{
    return hit_dram_count;
}

void CACHE::reset_udc_info()
{
    for (size_t i = 0; i < NUM_PREFETCHERS; i++)
    {
        delay_same_bank[i] = 0;
        delay_same_channel[i] = 0;
        delay_count[i] = 0;
    }
    hit_l3_latency = 0;
    hit_dram_latency = 0;
    hit_l3_count = 0;
    hit_dram_count = 0;
}