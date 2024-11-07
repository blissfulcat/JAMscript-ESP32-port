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

//#define MEMORY_DEBUG
#ifdef MEMORY_DEBUG
#include <stdio.h>
#include <stdint.h>
#define calloc(x,y) calloc(x,y); printf("calloc: %lu  " __FILE__ ":%d\n", (uint32_t) y*x, __LINE__)
#define malloc(x) malloc(x); printf("malloc: %lu"  __FILE__ ":%d\n", (uint32_t) x, __LINE__)
#define free(x) {free(x); printf("Freed a " #x " " __FILE__ ":%d\n", __LINE__);}
#endif
#endif
