O := o-$(TARGET_TYPE)
OBJS := $(patsubst %,%.$(O),$(SOURCES))
ifneq ($(filter -fpic,$(CFLAGS)),-fpic)
CFLAGS += -fPIC
endif

$(TARGET): $(OBJS)
	$(CC) -shared -o $@ $^

%.c.$(O): %.c
	$(CC) -c -o $@ $(CFLAGS) $<

%.$(O): %.c
	$(CC) -c -o $@ $(CFLAGS) $<
