.PHONY: tests
.PRECIOUS: %.o

TESTS := $(patsubst %.c,%,$(shell ls t[0-9]*.c))

# none or rtld_next
MEMALCHK ?= none
ifeq ($(MEMALCHK),rtld_next)
LDFLAGS += -ldl
endif
MEMALCHK := memalchk.$(MEMALCHK)

tests: $(TESTS)
	@for i in $^; do \
		./$$i || exit 1; \
	done

clean:
	rm -f $(TESTS) *.o

t%: t%.o $(MEMALCHK).o svsnprintf.o
	gcc $(CFLAGS) -o $@ $< $(MEMALCHK).o svsnprintf.o $(LDFLAGS)

t%.o: t%.c ../MY.c
	gcc $(CFLAGS) -c -o $@ $<

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $<
