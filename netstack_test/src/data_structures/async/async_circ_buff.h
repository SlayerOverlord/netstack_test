#pragma once

#include <stdlib.h>
#include <string.h>
#include <threads.h>

/*
*	ac_buff_t is a buffer structure meant to implement general purpous async
*	buffers.
* 
*	The general concept of the struct is to implement a general framework for
*	handling async data processing over a circular buffer. All data required
*   for processing should generally be included in the struct, but extra data
*   can be passed into/outof all processing stages. The bwproc (write) and
*   brproc (read) functions are protected by a mutex, so it is advised to
*   keep processing to a minimum in the functions.
* 
*   Only a read or a write function is required, but it is advisable to
*   implement both functions. The read and write pointers are automatically
*	indexed, and can be used in the function to determine positioning of the
*   current buffer segment (single increment, may add dynamic increments in
*   the future).
* 
*	@param init: int: The init value of the struct, 0 for uninitialized,
*		initialized otherwise, can be used for handling bclose.
*	@param bmtx: mtx_t: The mutex for the buffer, created at init, destroyed
*		during buffer destroy.
*	@param wmtx: mtx_t: The mutex for the write side of the buffer, used to
*		isolate write processing.
*	@param rmtx: mtx_t: The mutex for the read side of the buffer, used to
*		isolate read processing.
*	@param size: size_t: Number of buffer blocks.
*	@param wptr: size_t: Write pointer of data.
*	@param rptr: size_t: Read pointer of data.
*   @param binit: int(*)(ac_buff_t*, void*): Function used for initializing
*		the buffer. Returns thrd_succes on succesful init. Can be passed
*		external data if required using the void*
*	@param bwproc: int(*)(ac_buff_t*, size_t*, char*, void*): Function used
*		to write data to the buffer, can either use the size_t and char ptrs
*		to copy data, or use external data via the void*. Returns
*		thrd_success if write was successful.
*	@param brproc: int(*)(ac_buff_t*, size_t*, char*, void*): Function used
*		to read data from the buffer, can eitehr use the size_t and char ptrs
*		to copy data out of the critical section, or use external data via
*		the void*. Returns thrd_success if read was successful.
*	@param bclose: int(*)(ac_buff_t*, void*): Function used for closing/
*		destroying the buffer and auxilary data. Can utilize external data
*		if extra care is required during destruction. Returns thrd_success
*		on successful destruction.
*	@param bdata: void*: Buffer data, used for general processing like
*		element size or read/write counts, etc.
*	@param buff: void*: the pointer to the raw circular buffer.
*/
typedef struct ac_buff
{
	int    init;
	mtx_t  bmtx;
	mtx_t  wmtx;
	mtx_t  rmtx;
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