#include <stdlib.h>
#include <string.h>
#define __USE_GNU
#include <dlfcn.h>
#include <unistd.h>
#include "svsnprintf.h"
#include "tests.h"

#define CHUNKS_BASKET_BITS 4
#define CHUNKS_CNT_IN_BASKET 1024
#define CHUNKS_BASKET_MASK (unsigned int)(0xffffffff >> (32 - CHUNKS_BASKET_BITS))
#define CHUNKS_BASKETS_CNT (CHUNKS_BASKET_MASK + 1)

#define ERR_OUT(buf, size, fmt, ...) \
  write(2, buf, ssnprintf(buf, size, fmt, ##__VA_ARGS__))
#define ERR_EXIT(buf, size, fmt, ...) do {\
  ERR_OUT(buf, size, fmt, ##__VA_ARGS__);\
  exit(1);\
  } while (0)

struct memchunk memchunks[CHUNKS_BASKETS_CNT][CHUNKS_CNT_IN_BASKET], memchunks_saved[CHUNKS_BASKETS_CNT][CHUNKS_CNT_IN_BASKET];
unsigned int alloc_cnt;

void *(*libc_malloc)(size_t);
void *(*libc_realloc)(void*,size_t);
void (*libc_free)(void*);

void
memalchk_init(void)
{
	libc_malloc = dlsym(RTLD_NEXT, "malloc");
	libc_realloc = dlsym(RTLD_NEXT, "realloc");
	libc_free = dlsym(RTLD_NEXT, "free");
}

void
memalchk_show_stat(void)
{
	char buf[1024];

	ERR_OUT(buf, 1024, "MEMALCHK: mem allocations: %u\n", alloc_cnt);
}

void
memalchk_save(void)
{
	memcpy(memchunks_saved, memchunks, sizeof(memchunks));
}

void
memalchk_check(void)
{
	int i, j, is_ok = 1;
	char buf[1024];

	for(i = 0; i < CHUNKS_BASKETS_CNT; i++)
		for(j = 0; j < CHUNKS_CNT_IN_BASKET; j++) {
			if (memchunks[i][j].state == 0) {
				break;
			} else if (memchunks_saved[i][j].state == 0) {
				if (memchunks[i][j].state != 2) {
					is_ok = 0;
					ERR_OUT(buf, 1024, "MEMALCHK: found not freed memory: "
					  "%p\n", memchunks[i][j].ptr);
				}
			} else {
				if (memchunks[i][j].state != memchunks_saved[i][j].state) {
					if (memchunks[i][j].state == 1) {
						is_ok = 0;
						ERR_OUT(buf, 1024, "MEMALCHK: found not freed "
						  "memory: %p\n", memchunks[i][j].ptr);
					} else /* if (memchunks[i][j].state == 2) */ {
						is_ok = 0;
						ERR_OUT(buf, 1024, "MEMALCHK: found freed memory: "
						"%p\n", memchunks[i][j].ptr);
					}
				}
				if (memchunks_saved[i][j].ptr != memchunks[i][j].ptr) {
					is_ok = 0;
					ERR_OUT(buf, 1024, "MEMALCHK: memory chunks ptr are "
					  "not matched: %p and %p\n", memchunks_saved[i][j].ptr,
					  memchunks[i][j].ptr);
				}
			}
		}
	if (!is_ok)
		exit(1);
}

void
memalchk_log_alloc(void *ptr)
{
	int i, bidx;
	char buf[1024];

	bidx = (unsigned int)ptr & CHUNKS_BASKET_MASK;
	for(i = 0; i < CHUNKS_CNT_IN_BASKET; i++)
		if (memchunks[bidx][i].state == 0) {
			break;
		} else if (memchunks[bidx][i].ptr == ptr) {
			if (memchunks[bidx][i].state == 1)
				ERR_EXIT(buf, 1024, "MEMALCHK: allocated memory has the "
				  "same address as already allocated one: %p\n", ptr);
			break;
		}
	if (i == CHUNKS_CNT_IN_BASKET)
		ERR_EXIT(buf, 1024, "MEMALCHK: not enough space in memchunks "
		  "array: use bigger value for CHUNKS_CNT_IN_BASKET\n");
	memchunks[bidx][i].state = 1;
	memchunks[bidx][i].ptr = ptr;

	alloc_cnt++;
}

void
memalchk_log_free(void *ptr)
{
	int i, bidx;
	char buf[1024];

	bidx = (unsigned int)ptr & CHUNKS_BASKET_MASK;
	for(i = 0; i < CHUNKS_CNT_IN_BASKET; i++)
		if (memchunks[bidx][i].state == 0) {
			ERR_EXIT(buf, 1024, "MEMALCHK: try to free not allocated mem: "
			  "%p\n", ptr);
		} else if (memchunks[bidx][i].ptr == ptr) {
			if (memchunks[bidx][i].state == 2)
				ERR_EXIT(buf, 1024, "MEMALCHK: try to free already "
				  "freed mem: %p\n", ptr);
			memchunks[bidx][i].state = 2;
			break;
		}
	if (i == CHUNKS_CNT_IN_BASKET)
		ERR_EXIT(buf, 1024, "MEMALCHK: try to free not allocated mem: "
		  "%p\n", ptr);
}

void*
malloc(size_t size)
{
	void *ptr;

	ptr = libc_malloc(size);
	memalchk_log_alloc(ptr);
	return ptr;
}

void*
realloc(void *ptr, size_t size)
{
	void *prev_ptr = ptr;

	ptr = libc_realloc(ptr, size);
	if (prev_ptr != ptr) {
		memalchk_log_free(prev_ptr);
		memalchk_log_alloc(ptr);
	}
	return ptr;
}

void
free(void *ptr)
{
	memalchk_log_free(ptr);
	libc_free(ptr);
}

