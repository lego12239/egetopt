O := o
OBJS := $(patsubst %,%.$(O),$(SOURCES))

define projmk_prog_gen_targ
ifeq ($(TARGET_TYPE),prog)
$(1):: $(if $(wordlist 2,$(words $(TARGET)),$(TARGET)),$(1).c.$(O)) $(OBJS)
	$(CC) -o $$@ $$^ $$(LDFLAGS)
endif
endef

$(foreach targ,$(TARGET),$(eval $(call projmk_prog_gen_targ,$(targ))))

%.c.$(O): %.c
	$(CC) -c -o $@ $(CFLAGS) $<

%.$(O): %.c
	$(CC) -c -o $@ $(CFLAGS) $<
