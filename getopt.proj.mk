PROJECT := libegetopt
TARGET_TYPE ?= liba
SOURCES := egetopt.c
#SOURCES := src1.c src2.c src3.c

#CFLAGS := -Werror=pedantic -pedantic -std=c11
#LDFLAGS := -L.

#DEPS := libmya
#deps_get_libmya ?= ln $(PROJDIR)/libmya

include .proj.mk/c-proj.mk

install: $(TARGET)
	install -d $(D)/lib $(D)/include
	install -m 0444 $(TARGET) $(D)/lib
	install -m 0444 my.h $(D)/include

ex1: ex1.o
	$(CC) -o $@ $(CFLAGS) $< -L. -legetopt

ex2: ex2.o
	$(CC) -o $@ $(CFLAGS) $< -L. -legetopt

clean-ex:
	rm -f ex1 ex2

clean:: clean-ex
