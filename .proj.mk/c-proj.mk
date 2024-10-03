PROJMKDIR ?= $(shell A=`pwd`; while [ ! -d $$A/.proj.mk ] ; do A=$${A%/*}; done; echo $$A/.proj.mk)

include $(PROJMKDIR)/proj.mk
include $(PROJMKDIR)/$(TARGET_TYPE)-c-proj.mk
