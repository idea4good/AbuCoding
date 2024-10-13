//Build with Visual Studio command prompt: cl.exe prediction.c
//Build with GCC: gcc prediction.c

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif
#include <stdint.h>
#include <stdio.h>

#define PAGE_NUM            256
#define CACHE_HIT_THRESHOLD 100

int get_access_time(uint8_t* page)
{
    unsigned long long tick1, tick2;
    unsigned int aux;

    tick1 = __rdtscp(&aux);
    uint8_t tmp = *page;
    tick2 = __rdtscp(&aux);
    
    return (tick2 - tick1);
}

static uint8_t mem_pages[PAGE_NUM][1024 * 4];

int detect_cached_page()
{
    //find the pages which in cache. Order is lightly mixed up to prevent stride prediction
    for (int i = 0; i < PAGE_NUM; i++)
    {
        uint8_t mix_i = ((i * 167) + 13) & 255;
        if (get_access_time(mem_pages[mix_i]) < CACHE_HIT_THRESHOLD)
        {
            return mix_i;
        }
    }

    return -1;
}

int data = 100;

int main()
{
    for(int i = 0; i < 30; i++)
    {
        _mm_clflush(&data);
        for (int i = 0; i < PAGE_NUM; i++)
        {
            _mm_clflush(mem_pages[i]);
        }
        
        if(data == 0)
        {
            data = mem_pages[37][0];
        }

        int cached_page = detect_cached_page();

        printf("cached page:%d, data: %d\n", cached_page, data);
    }
}
