﻿//Build from Visual Studio command prompt: cl.exe cache.c
//Build with GCC: gcc cache.c

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define PAGE_SIZE			(1024 * 4)
#define MAX_VALUE			256
#define CACHE_HIT_THRESHOLD 100

static uint8_t mem_pages[MAX_VALUE][PAGE_SIZE];

int get_access_time(uint8_t* page)
{
	unsigned long long tick1, tick2;
	unsigned int aux;

	tick1 = __rdtscp(&aux);
	uint8_t tmp = *page;
	tick2 = __rdtscp(&aux);
	
	return (tick2 - tick1);
}

uint8_t detect_memory_quick(uint8_t* address)
{
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

	return '?';
}

uint8_t detect_memory_slow(uint8_t* address)
{
	//move all pages out of cache
	for (int i = 0; i < MAX_VALUE; i++)
	{
		_mm_clflush(mem_pages[i]);
	}

	//move 1 page into cache
	uint8_t index = *(uint8_t*)address;
	uint8_t tmp = mem_pages[index][0];

	//find the pages which in cache. Order is lightly mixed up to prevent stride prediction
	int ret = '?';
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

char data[] = "password";

int main()
{
	memset(mem_pages, 1, sizeof(mem_pages));

	printf("The data: ");
	for (int i = 0; i < strlen(data); i++)
	{
		printf("%c", detect_memory_quick((uint8_t*)&data[i]));
	}
	printf("\n");

	printf("The data: ");
	for (int i = 0; i < strlen(data); i++)
	{
		printf("%c", detect_memory_slow((uint8_t*)&data[i]));
	}
	printf("\n");
}
