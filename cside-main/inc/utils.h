#ifndef __UTIL_H__
#define __UTIL_H__
/**
 * THIS FILE WAS COPIED FROM THE ESP32/CSIDE 
 * WE SHOULD PUT COMMON UTILITY FUNCTIONS AND HEADERS HERE
*/
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include "esp_log.h"

#define NULL ((void *)0)
#define JAM_OK 1
#define JAM_FAIL -1
#define JAM_MEMORY_ERR -2
typedef uint32_t jam_error_t;
#define ERR_PROP(x) {jam_error_t __temperr = x; if(__temperr!=JAM_OK) return __temperr;}


void dump_bufer_hex(uint8_t* buffer, uint32_t size);
void dump_bufer_hex_raw(uint8_t* buffer, uint32_t size);
void dump_heap_left();
char* concat(const char *s1, const char *s2);
static const char* ERROR_TAG = "JAM_ERROR";
#define log_error(x) ESP_LOGE(ERROR_TAG, "Jamscript Runtime Error: %s  " __FILE__ ":%d.\n",x, __LINE__);
#define MEMORY_DEBUG
#ifdef MEMORY_DEBUG
extern int32_t total_mem_usage;
static uint32_t string_len = 1;

static const char* TAG = "MEMORY_DEBUG";
#define calloc(x,y) calloc(x,y); total_mem_usage+=(y*x); ESP_LOGI(TAG, "calloc: %lu  " __FILE__ ":%d. Total M count: %li\n", (uint32_t) y*x, __LINE__, total_mem_usage)
#define malloc(x) malloc(x); total_mem_usage+=x; ESP_LOGI(TAG, "malloc: %lu"  __FILE__ ":%d. Total M count: %li\n", (uint32_t) x, __LINE__, total_mem_usage)
#define free(x) {string_len = (sizeof(*x) == sizeof(char))?strlen(x)+1:1; free(x); total_mem_usage-=sizeof(*x)*string_len; ESP_LOGI(TAG, "Freed a " #x " " __FILE__ ":%d. Total M count: %li\n", __LINE__, total_mem_usage);}
#endif
#endif