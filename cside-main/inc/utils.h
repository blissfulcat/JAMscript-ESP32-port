#ifndef __UTIL_H__
#define __UTIL_H__
/**
 * THIS FILE WAS COPIED FROM THE ESP32/CSIDE 
 * WE SHOULD PUT COMMON UTILITY FUNCTIONS AND HEADERS HERE
*/
#include <stdint.h>
#include <stdbool.h>

#define NULL ((void *)0)
#define JAM_OK 1
#define JAM_FAIL -1
#define JAM_MEMORY_ERR -2
typedef uint32_t jam_error_t;
#define ERR_PROP(x) {jam_error_t __temperr = x; if(__temperr!=JAM_OK) return __temperr;}


void dump_bufer_hex(uint8_t* buffer, uint32_t size);
void dump_bufer_hex_raw(uint8_t* buffer, uint32_t size);
void dump_heap_left();

#define MEMORY_DEBUG
#ifdef MEMORY_DEBUG
static int32_t total_mem_usage = 0;
#include <stdio.h>
#include <stdint.h>
#define calloc(x,y) calloc(x,y); total_mem_usage+=(y*x); printf("calloc: %lu  " __FILE__ ":%d. Total M count: %li\n", (uint32_t) y*x, __LINE__, total_mem_usage)
#define malloc(x) malloc(x); total_mem_usage+=x; printf("malloc: %lu"  __FILE__ ":%d. Total M count: %li\n", (uint32_t) x, __LINE__, total_mem_usage)
#define free(x) {free(x); total_mem_usage-=sizeof(*x); printf("Freed a " #x " " __FILE__ ":%d. Total M count: %li\n", __LINE__, total_mem_usage);}
#endif
#endif
