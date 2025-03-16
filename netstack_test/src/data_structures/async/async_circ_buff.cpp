#include "async_circ_buff.h"

int _ac_buff_valid(ac_buff_t* buff);
int _ac_buff_rready(ac_buff_t* buff);
int _ac_buff_wready(ac_buff_t* buff);

int ac_buff_init(ac_buff_t* buff, void* ext_dat)
{
	int ret = thrd_error;
	if (!_ac_buff_valid(buff)) return ret;
	
	mtx_destroy(&buff->mtx);

	buff->init = 0;
	buff->rptr = 0;
	buff->wptr = 0;
	mtx_init(&buff->mtx, mtx_plain);
	
	ret = buff->binit(buff, ext_dat);
	if (!buff->init) buff->init = 1;

	return ret;
}

int ac_buff_rproc(ac_buff_t* buff, size_t* size, char* dst, void* ext_dat)
{
	int success = thrd_busy;
	if (!_ac_buff_valid(buff)) return thrd_error;

	while (success == thrd_busy)
	{
		mtx_lock(&buff->mtx);
		
		// --<Shared Region>--
		if (_ac_buff_rready(buff))
		{
			success = buff->brproc(buff, size, dst, ext_dat);
			buff->rptr = (buff->rptr >= buff->size) ? 0 : buff->rptr + 1;
		}
		// --<Shared Region>--

		mtx_unlock(&buff->mtx);
	}

	return success;
}

int ac_buff_wproc(ac_buff_t* buff, size_t* size, char* src, void* ext_dat)
{
	int success = thrd_busy;
	if (!_ac_buff_valid(buff)) return thrd_error;

	while (success == thrd_busy)
	{
		mtx_lock(&buff->mtx);

		// --<Shared Region>--
		if (_ac_buff_wready(buff))
		{
			success = buff->bwproc(buff, size, src, ext_dat);
			buff->wptr = (buff->wptr >= buff->size) ? 0 : buff->wptr + 1;
		}
		// --<Shared Region>--

		mtx_unlock(&buff->mtx);
	}

	return success;
}

int ac_buff_close(ac_buff_t* buff, void* ext_dat)
{
	int ret = thrd_error;
	if (!_ac_buff_valid(buff)) return ret;

	ret = buff->bclose(buff, ext_dat);

	buff->init = 0;
	buff->rptr = 0;
	buff->wptr = 0;
	mtx_destroy(&buff->mtx);

	return ret;
}

int _ac_buff_valid(ac_buff_t* buff)
{
	if (!buff) return thrd_error;
	if (!buff->binit) return thrd_error;
	if (!buff->bclose) return thrd_error;
	if (!buff->brproc && !buff->bwproc) return thrd_error;

	return buff->init;
}

int _ac_buff_rready(ac_buff_t* buff)
{
	return buff->rptr == buff->wptr;
}

int _ac_buff_wready(ac_buff_t* buff)
{
	if (buff->rptr == 0 && buff->wptr == buff->size - 1) return 0;
	return (buff->wptr != buff->rptr - 1);
}

// --------<Character Buffer Implementation>--------
/*typedef struct ac_cbuff_data
{
	size_t page_size;
	size_t life_read;
	size_t life_write;
} ac_cbuff_data;*/

int ac_cbuff_init(ac_buff_t* buff, void* ext_dat)
{
	ac_cbuff_data_t* bdata = NULL;
	buff->init = 0;

	if (!buff->bdata) // If no data exists, create default and mark for auto-delete
	{
		buff->bdata = calloc(1, sizeof(ac_cbuff_data_t));
		if (!buff->bdata) return thrd_error;
		buff->init = 2;
	} else // Mark non-auto delete
	{
		buff->init = 1;
	}

	bdata = (ac_cbuff_data_t*) buff->bdata;
	if (!bdata->page_size) bdata->page_size = AC_CBUFF_DEF_SIZE;
	
	bdata->life_read  = 0;
	bdata->life_write = 0;

	if (buff->buff) free(buff->buff);
	buff->buff = calloc(buff->size, bdata->page_size);
	if (!buff->buff)
	{
		bdata = NULL;
		return thrd_error;
	}

	return thrd_success;
}

int ac_cbuff_rproc(ac_buff_t* buff, size_t* size, char* dst, void* ext_dat)
{
	char* block = NULL;
	ac_cbuff_data_t* bdata = NULL;
	if (!buff->buff) return thrd_error;
	if (!buff->bdata) return thrd_error;

	bdata = (ac_cbuff_data_t*)buff->bdata;
	if (bdata->page_size == 0)
	{
		bdata = NULL;
		return thrd_error;
	}

	if (buff->rptr >= buff->size) buff->size = 0;
	block = (char*)buff->buff + (bdata->page_size * buff->rptr);

	strcpy(dst, block);

	block = NULL;
	bdata = NULL;
	return thrd_success;
}

int ac_cbuff_rproc(ac_buff_t* buff, size_t* size, char* src, void* ext_dat)
{
	char* block = NULL;
	ac_cbuff_data_t* bdata = NULL;
	if (!buff->buff) return thrd_error;
	if (!buff->bdata) return thrd_error;

	bdata = (ac_cbuff_data_t*)buff->bdata;
	if (bdata->page_size == 0)
	{
		bdata = NULL;
		return thrd_error;
	}

	if (buff->rptr >= buff->size) buff->size = 0;
	if (buff->rptr == buff->wptr)
	{
		block = NULL;
		bdata = NULL;
		return thrd_error;
	}

	block = (char*)buff->buff + (bdata->page_size * buff->rptr);
	strcpy(block, src);

	block = NULL;
	bdata = NULL;
	return thrd_success;
}

int ac_cbuff_close(ac_buff_t* buff, void* ext_dat)
{
	ac_cbuff_data_t* bdata = NULL;
	if (buff->bdata && buff->size)
	{
		bdata = (ac_cbuff_data_t*)buff->bdata;
		memset(buff->buff, 0, bdata->page_size * buff->size);
	}

	if (buff->init == 2 && buff->bdata) free(buff->bdata);
	if (buff->buff) free(buff->buff);

	buff->init = 0;
	buff->rptr = 0;
	buff->wptr = 0;

	bdata = NULL;

	return thrd_success;
}
