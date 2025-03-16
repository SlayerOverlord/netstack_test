#pragma once

#include <stdlib.h>
#include <string.h>
#include <threads.h>

typedef struct ac_buff
{
	char   init;
	mtx_t  mtx;
	size_t size;
	size_t wptr;
	size_t rptr;
	int  (*binit)(ac_buff_t*, void*);
	int  (*bwproc)(ac_buff_t*, size_t*, char*, void*);
	int  (*brproc)(ac_buff_t*, size_t*, char*, void*);
	int  (*bclose)(ac_buff_t*, void*);
	void*  bdata;
	void*  buff;
} ac_buff_t;

int ac_buff_init(ac_buff_t* buff, void* ext_dat);
int ac_buff_rproc(ac_buff_t* buff, size_t* size, char* dst, void* ext_dat);
int ac_buff_wproc(ac_buff_t* buff, size_t* size, char* src, void* ext_dat);
int ac_buff_close(ac_buff_t* buff, void* ext_dat);

// Usage
// character buffer
#define AC_CBUFF_DEF_SIZE 4096

typedef struct ac_cbuff_data
{
	size_t page_size;
	size_t life_read;
	size_t life_write;
} ac_cbuff_data_t;

int ac_cbuff_init(ac_buff_t* buff, void* ext_dat);
int ac_cbuff_rproc(ac_buff_t* buff, size_t* size, char* dst, void* ext_dat);
int ac_cbuff_rproc(ac_buff_t* buff, size_t* size, char* src, void* ext_dat);
int ac_cbuff_close(ac_buff_t* buff, void* ext_dat);