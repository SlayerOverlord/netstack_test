#pragma once

#include <stdint.h>
#include <stdio.h>

// Mode Size reffers to the maximum possible mode lenght for
// the fopen function in <stdio.h> + 1 for null terminator
#define MODE_SIZE 4

/*	
*	file_proc_t is the main struct used by the io handler for reading/writing from/to files.
* 
*	The file can be initialized using the finit file pointer, if no initialization is used
*	then the fproc will be used imediately. On file close, the function fclose will be used
*	if available.
* 
*	File Opening/closing is handled by the thread.
* 
*	finit functions are used to initalize the contents of the file, for example, recording
*	a file header for future processing. If NULL the initialization stage will be ignored.
* 
*	fproc functions only need to read/write file from/to the char**, the thread will handle
*	scheduling and other tasks.
* 
*	fclose functions are used to close the procesing of a file, for example, writing a
*	footer to a file as reqired.
* 
*	Functions can use the void* to transmit/recieve extra data as neede, this will not be
*	handled by the thread and needs to be handled by the function/external processes.
* 
*	Functions should return success as 0, other error codes can be returned as needed while
*   not halting thread function, throw an error to terminate the thread.
* 
*   @param fptr: File*: a pointer to the file used by the thread
*	@param fmode: char[MODE_SIZE]: an array to store the fopen mode parameter of the file
*								   where MODE_SIZE = 4
*	@param flimit: size_t: a limit to the size of the i/o buffer depth for the thread
*   @param finit: int(*)(file_proc_t*, void*): a function pointer that takes in the
*											   file_proc_t type and a void ptr to initalize
*											   the file.
*	@param fproc: int(*)(file_proc_t*, char**, void*) a function pointer that takes in the
*													  file_proc_t type and a void ptr to
*													  perform i/o on the file, with i/o
*													  data present in the char**
*	@parap fclose: int(*)(file_proc_t*, void*): a function pointer that takes in the
*												file_proc_t type and a void ptr to close
*												the file.
*	
*/
typedef struct file_proc
{
	FILE*     fptr;
	char      fmode[MODE_SIZE];
	size_t    flimit;
	int     (*finit)(file_proc_t*, void*);
	int     (*fproc)(file_proc_t*, char**, void*);
	int     (*fclose)(file_proc_t*, void*);
} file_proc_t;

