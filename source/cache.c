//Build from Visual Studio command prompt: cl cache.c
//Build with GCC: gcc cache.c

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif
#include <stdint.h>
#include <stdio.h>

#define PAGE_SIZE			(1024 * 4)
#define MAX_VALUE			256
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

uint8_t detect_memory1(uint8_t* address)
{
	static uint8_t mem_pages[MAX_VALUE][PAGE_SIZE];

	//move all pages out of cache
	for (int i = 0; i < MAX_VALUE; i++)
	{
		_mm_clflush(mem_pages[i]);
	}
	
	//move 1 page into cache
	uint8_t index = *address;
	uint8_t temp = mem_pages[index][0];

	//find the pages which in cache. Order is lightly mixed up to prevent stride prediction
	for (int i = 0; i < MAX_VALUE; i++)
	{
		int mix_i = ((i * 167) + 13) & 255;
		if (get_access_time(mem_pages[mix_i]) < CACHE_HIT_THRESHOLD)
		{
			return mix_i;
		}
	}

	return 0;
}

uint8_t detect_memory(uint8_t* address)
{
	static uint8_t mem_pages[MAX_VALUE][PAGE_SIZE];

	//move all pages out of cache
	for (int i = 0; i < MAX_VALUE; i++)
	{
		_mm_clflush(mem_pages[i]);
	}

	//move 1 page into cache
	uint8_t index = *(uint8_t*)address;
	uint8_t tmp = mem_pages[index][0];

	//find the pages which in cache. Order is lightly mixed up to prevent stride prediction
	int ret = 0;
	int min_time = 10000000;
	for (int i = 0; i < MAX_VALUE; i++)
	{
		int mixed_i = ((i * 167) + 13) & 255;
		int access_time = get_access_time(mem_pages[mixed_i]);
		if (min_time > access_time)
		{
			min_time = access_time;
			ret = mixed_i;
		}
	}

	return ret;
}

uint8_t data[] = "password";

int main()
{
	printf("The data: ");
	for (int i = 0; i < sizeof(data); i++)
	{
		printf("%c", detect_memory(&data[i]));
	}

	printf("\nThe data: ");
	for (int i = 0; i < sizeof(data); i++)
	{
		printf("%c", detect_memory1(&data[i]));
	}
}
