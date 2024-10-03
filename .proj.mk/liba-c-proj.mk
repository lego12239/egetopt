O := o
OBJS := $(patsubst %,%.$(O),$(SOURCES))

$(TARGET): $(OBJS)
	$(AR) rvs $@ $^

%.c.$(O): %.c
	$(CC) -c -o $@ $(CFLAGS) $<

%.$(O): %.c
	$(CC) -c -o $@ $(CFLAGS) $<
