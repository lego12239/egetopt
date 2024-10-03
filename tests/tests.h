#ifndef __TESTS_H__
#define __TESTS_H__

#define TERR_OUT(fmt, ...) fprintf(stderr, "%s:%u: " fmt "\n", \
  __FILE__, __LINE__, ##__VA_ARGS__)
#define TERR(fmt, ...) do { TERR_OUT(fmt, ##__VA_ARGS__); return 0; } while (0)
#define TERR_EXIT(fmt, ...) do { TERR_OUT(fmt, ##__VA_ARGS__); fprintf(stderr, "FAIL\n"); exit(1); } while (0)
#define TEST_NULL {NULL, NULL}

struct test {
	int (*test)(void);
	char *title;
};

struct memchunk {
	int state; /* 0 - unused, 1 - allocated, 2 - freed */
	void *ptr;
};

void memalchk_init(void);
void memalchk_save(void);
void memalchk_check(void);
void memalchk_show_stat(void);

#endif /* __TESTS_H__ */
