# Licensed under 2-clause BSD license. See LICENSE file.
# Copyright 2021-2024 Oleg Nemanov <lego12239@yandex.ru>
# https://github.com/lego12239/proj.mk
# Version 1.1.2

ifeq ($(PROJECT),)
$(error PROJECT variable is't defined)
endif

# Set target and target type
# TARGET_TYPE or TARGET must be set.
# We can detect TARGET from TARGET_TYPE and PROJECT;
# and we can detect TARGET_TYPE from TARGET.
ifeq ($(TARGET_TYPE),)
TARGET ?= $(PROJECT)
ifeq ($(suffix $(TARGET)),.a)
TARGET_TYPE := liba
else ifeq ($(suffix $(TARGET)),.so)
TARGET_TYPE := libso
else
TARGET_TYPE := prog
endif
endif

# Check target type value
ifeq ($(TARGET_TYPE),prog)
else ifeq ($(TARGET_TYPE),liba)
else ifeq ($(TARGET_TYPE),libso)
else
$(error unknown TARGET_TYPE: $(TARGET_TYPE))
endif
$(info $(PROJECT): TARGET_TYPE is $(TARGET_TYPE))

ifeq ($(TARGET),)
ifeq ($(TARGET_TYPE),liba)
TARGET := $(PROJECT).a
else ifeq ($(TARGET_TYPE),libso)
TARGET := $(PROJECT).so
else ifeq ($(TARGET_TYPE),prog)
TARGET := $(PROJECT)
endif
endif

CCYAN := $(shell /bin/echo -e '\033[36;01m')
CRST := $(shell /bin/echo -e '\033[00m')
projmk_infomsg = echo "$(CCYAN)[$(1)]$(CRST)";

DESTDIR ?= /
PREFIX ?= /usr/local
D := $(DESTDIR)/$(PREFIX)
O ?= o-$(TARGET_TYPE)

ifdef DEPSDIR
DEPS_ME_IS_DEP := 1
endif

PROJDIR ?= $(shell pwd)
# Change also clean-deps-all rule
DEPSDIR ?= $(PROJDIR)/deps/
PROJMKDIR ?= $(shell A=`pwd`; while [ ! -d $$A/.proj.mk ] ; do A=$${A%/*}; done; echo $$A/.proj.mk)
PROJMKDIR_REL := $(shell $(PROJMKDIR)/rmpref.sh '$(PROJMKDIR)' '$(PROJDIR)')
DEPS_VARS_RMDUPS += CFLAGS LDFLAGS
DEPS_VARS_EXPORT += CFLAGS LDFLAGS

CFLAGS += -Wall -Werror=implicit -I$(shell pwd) -I$(DEPSDIR)/include
LDFLAGS += -L$(DEPSDIR)/lib

ifdef DEBUG
	CFLAGS += -g3 -ggdb -DDEBUG
endif

export PROJDIR
export DEPSDIR
export DEBUG
ifdef TCHECKS
	export TCHECKS
endif


.PHONY: deps deps_ deps_show projmk_gendepsinfo
.PHONY: build build-pre build-post
.PHONY: clean clean-all clean-deps clean-deps-all clean-tests tests

#ifneq ($(MAKECMDGOALS),deps_show)
#DEPS := $(shell $(MAKE) deps_show)
#endif
#DEPS_REV := $(shell echo $(DEPS) | $(PROJMKDIR)/revdeps.sh)
#DEPS := $(shell echo $(DEPS_REV) | $(PROJMKDIR)/revdeps.sh)

DEPS := $(shell $(PROJMKDIR)/stripslash.sh $(DEPS))
ifneq ($(.SHELLSTATUS),0)
$(error DEPS set failure)
endif


######################################################################
# TEMPLATES FOR include statements
######################################################################
define _deps_gen_incl
include $(DEPSDIR)/src/$(1).proj.mk.info

endef

######################################################################
# TEMPLATES FOR deps_show TARGET
######################################################################
define _deps_gen_show
	$(if $(shell test -e $(DEPSDIR)/src/$(1)/.proj.mk && echo t),\
	  @$(MAKE) --no-print-directory -C $(DEPSDIR)/src/$(1) deps_show)

endef

######################################################################
# TEMPLATES FOR GET STAGE
######################################################################
define _deps_gen_get_ln
	if [ -e $(DEPSDIR)/src/$(1) ]; then \
		rm -rf $(DEPSDIR)/src/$(1); \
	fi
	ln -Ts $(word 2,$(2)) $(DEPSDIR)/src/$(1)
endef

define _deps_gen_get_cp
	if [ -e $(DEPSDIR)/src/$(1) ]; then \
		rm -rf $(DEPSDIR)/src/$(1); \
	fi
	cp -rf $(word 2,$(2)) $(DEPSDIR)/src/$(1)
endef

define _deps_gen_get_git
	if [ -d $(DEPSDIR)/src/$(1)/.git ]; then \
		cd $(DEPSDIR)/src/$(1) && git pull; \
	elif [ -e $(DEPSDIR)/src/$(1) ]; then \
		rm -rf $(DEPSDIR)/src/$(1); \
		git clone $(word 2,$(2)) $(DEPSDIR)/src/$(1) || exit 1; \
	else \
		git clone $(word 2,$(2)) $(DEPSDIR)/src/$(1) || exit 1; \
	fi
	$(if $(word 3,$(2)),cd $(DEPSDIR)/src/$(1) && git checkout $(word 3,$(2)))
endef

define _deps_gen_get_wget
	if [ -e $(DEPSDIR)/src/$(1) ]; then \
		rm -rf $(DEPSDIR)/src/$(1); \
	fi
	wget -O $(DEPSDIR)/src/$(1).WK "$(word 2,$(2))"
	$(PROJMKDIR)/dep_unpack.sh $(1)
endef

define _deps_gen_get_custom
	$(wordlist 2,$(words $(2)),$(2))
endef

define _deps_gen_get
	@$(call projmk_infomsg,$(PROJECT): OBTAIN $(1) using $(word 1,$(2)))
	$(call _deps_gen_get_$(word 1,$(2)),$(1),$(2))
endef

######################################################################
# TEMPLATES FOR PATCH STAGE
######################################################################
define _projmk_gen_deppatch
	@$(call projmk_infomsg,$(PROJECT): PATCH $(1))
	if [ -e $(DEPSDIR)/src/$(1) ]; then \
		for P in $(2); do \
			patch -d $(DEPSDIR)/src/$(1) -p1 <$$$$P || exit 1; \
		done \
	fi
endef

######################################################################
# TEMPLATES FOR BUILD STAGE
######################################################################
define _deps_gen_build
	$(call projmk_infomsg,$(PROJECT): BUILD $(1) using deps_build_$(1))\
	cd $(DEPSDIR)/src/$(1) && { $(2); }
endef

define _deps_gen_build_configure
	cd $(DEPSDIR)/src/$(1) && ./configure --prefix=/ || exit 1; \
	$$(MAKE) -C $(DEPSDIR)/src/$(1) || exit 1; \
	$$(MAKE) -C $(DEPSDIR)/src/$(1) DESTDIR=$(DEPSDIR) install
endef

define _deps_gen_build_projmk
	$(if $(deps_ttype_$(1)),TARGET_TYPE=$(deps_ttype_$(1))) $$(MAKE) -C $(DEPSDIR)/src/$(1) -f $(1).proj.mk || exit 1; \
	$(if $(deps_ttype_$(1)),TARGET_TYPE=$(deps_ttype_$(1))) $$(MAKE) -C $(DEPSDIR)/src/$(1) -f $(1).proj.mk DESTDIR=$(DEPSDIR) PREFIX=/ install
endef

define _deps_gen_build_default
	$$(MAKE) -C $(DEPSDIR)/src/$(1) || exit 1; \
	$$(MAKE) -C $(DEPSDIR)/src/$(1) DESTDIR=$(DEPSDIR) PREFIX=/ install
endef

define _deps_gen_build_detect
	if [ -e $(PROJDIR)/.proj.mk/db.priv/$(1).proj.mk ]; then\
		$(call projmk_infomsg,$(PROJECT): BUILD $(1) using $(PROJDIR)/.proj.mk/db.priv/$(1).proj.mk)\
		cd $(DEPSDIR)/src/$(1) || exit 1;\
		$$(MAKE) -f $(PROJDIR)/.proj.mk/db.priv/$(1).proj.mk || exit 1;\
		$$(MAKE) -f $(PROJDIR)/.proj.mk/db.priv/$(1).proj.mk DESTDIR=$(DEPSDIR) PREFIX=/ install || exit 1;\
	elif [ -e $(PROJMKDIR)/db.priv/$(1).proj.mk ]; then\
		$(call projmk_infomsg,$(PROJECT): BUILD $(1) using $(PROJMKDIR)/db.priv/$(1).proj.mk)\
		cd $(DEPSDIR)/src/$(1) || exit 1;\
		$$(MAKE) -f $(PROJMKDIR)/db.priv/$(1).proj.mk || exit 1;\
		$$(MAKE) -f $(PROJMKDIR)/db.priv/$(1).proj.mk DESTDIR=$(DEPSDIR) PREFIX=/ install || exit 1;\
	elif [ -e $(DEPSDIR)/src/$(1)/$(1).proj.mk ]; then\
		$(call projmk_infomsg,$(PROJECT): BUILD $(1) using $(DEPSDIR)/src/$(1)/$(1).proj.mk)\
		$(call _deps_gen_build_projmk,$(1)) || exit 1; \
	elif [ -e $(PROJMKDIR)/db/$(1).proj.mk ]; then\
		$(call projmk_infomsg,$(PROJECT): BUILD $(1) using $(PROJMKDIR)/db/$(1).proj.mk)\
		cd $(DEPSDIR)/src/$(1) || exit 1;\
		$$(MAKE) -f $(PROJMKDIR)/db/$(1).proj.mk || exit 1;\
		$$(MAKE) -f $(PROJMKDIR)/db/$(1).proj.mk DESTDIR=$(DEPSDIR) PREFIX=/ install || exit 1;\
	elif [ -e $(DEPSDIR)/src/$(1)/configure ]; then\
		$(call projmk_infomsg,$(PROJECT): BUILD $(1) using configure&make)\
		$(call _deps_gen_build_configure,$(1)) || exit 1;\
	else\
		$(call projmk_infomsg,$(PROJECT): BUILD $(1) using make)\
		$(call _deps_gen_build_default,$(1)) || exit 1; \
	fi
endef

######################################################################
# TEMPLATES FOR DEPS TARGETS
######################################################################
define _deps_gen_targets
$(DEPSDIR)/src/$(1).proj.mk.info: $(DEPSDIR)/src/.$(1).get
	@if [ -e $(DEPSDIR)/src/$(1) ]; then \
		$(if $(deps_build_$(1)),$(call _deps_gen_build,$(1),\
		  $(deps_build_$(1))),$(call _deps_gen_build_detect,$(1))) || exit 1; \
	fi
	@$(call projmk_infomsg,$(PROJECT): GENERATE info for $(1))
	@echo USE_$(1) := 1 > $(DEPSDIR)/src/$(1).proj.mk.info
	@if [ -e $(PROJDIR)/.proj.mk/db.priv/$(1).proj.mk.info ]; then \
		$(call projmk_infomsg,$(PROJECT): GENERATE info for $(1) using $(PROJDIR)/.proj.mk/db.priv/$(1).proj.mk.info)\
		echo 'include $$$$(PROJDIR)/.proj.mk/db.priv/$(1).proj.mk.info' >> $(DEPSDIR)/src/$(1).proj.mk.info; \
	elif [ -e $(PROJMKDIR)/db.priv/$(1).proj.mk.info ]; then \
		$(call projmk_infomsg,$(PROJECT): GENERATE info for $(1) using $(PROJMKDIR)/db.priv/$(1).proj.mk.info)\
		echo 'include $$$$(PROJDIR)/$(PROJMKDIR_REL)/db.priv/$(1).proj.mk.info' >> $(DEPSDIR)/src/$(1).proj.mk.info; \
	elif [ -e $(DEPSDIR)/src/$(1)/$(1).proj.mk.info ]; then \
		$(call projmk_infomsg,$(PROJECT): GENERATE info for $(1) using $(DEPSDIR)/src/$(1)/$(1).proj.mk.info)\
		echo 'include $$$$(DEPSDIR)/src/$(1)/$(1).proj.mk.info' >> $(DEPSDIR)/src/$(1).proj.mk.info; \
	elif [ -e $(PROJMKDIR)/db/$(1).proj.mk.info ]; then \
		$(call projmk_infomsg,$(PROJECT): GENERATE info for $(1) using $(PROJMKDIR)/db/$(1).proj.mk.info)\
		echo 'include $$$$(PROJDIR)/$(PROJMKDIR_REL)/db/$(1).proj.mk.info' >> $(DEPSDIR)/src/$(1).proj.mk.info; \
	elif pkg-config --exists $(1); then \
		$(call projmk_infomsg,$(PROJECT): GENERATE info for $(1) using pkg-config)\
		echo CFLAGS += `pkg-config --cflags $(1)` >> $(DEPSDIR)/src/$(1).proj.mk.info; \
		echo LDFLAGS += `pkg-config --libs $(1)` >> $(DEPSDIR)/src/$(1).proj.mk.info; \
	fi
	@if [ -e $(PROJDIR)/.proj.mk/db.priv/$(1).proj.mk ]; then\
		$(call projmk_infomsg,$(PROJECT): GENERATE info for $(1) using $(PROJDIR)/.proj.mk/db.priv/$(1).proj.mk)\
		cd $(DEPSDIR)/src/$(1) || exit 1;\
		$$(MAKE) -f $(PROJDIR)/.proj.mk/db.priv/$(1).proj.mk projmk_gendepsinfo || exit 1; \
	elif [ -e $(PROJMKDIR)/db.priv/$(1).proj.mk ]; then\
		$(call projmk_infomsg,$(PROJECT): GENERATE info for $(1) using $(PROJMKDIR)/db.priv/$(1).proj.mk)\
		cd $(DEPSDIR)/src/$(1) || exit 1;\
		$$(MAKE) -f $(PROJMKDIR)/db.priv/$(1).proj.mk projmk_gendepsinfo || exit 1; \
	elif [ -e $(DEPSDIR)/src/$(1)/$(1).proj.mk ]; then \
		$(call projmk_infomsg,$(PROJECT): GENERATE info for $(1) using $(1).proj.mk)\
		$$(MAKE) -C $(DEPSDIR)/src/$(1) -f $(1).proj.mk projmk_gendepsinfo || exit 1; \
	elif [ -e $(PROJMKDIR)/db/$(1).proj.mk ]; then\
		$(call projmk_infomsg,$(PROJECT): GENERATE info for $(1) using $(PROJMKDIR)/db/$(1).proj.mk)\
		cd $(DEPSDIR)/src/$(1) || exit 1;\
		$$(MAKE) -f $(PROJMKDIR)/db/$(1).proj.mk projmk_gendepsinfo || exit 1; \
	fi
	@$(call projmk_infomsg,$(PROJECT): DONE with $(1))

$(DEPSDIR)/src/.$(1).get: $(DEPSDIR)/.deps_dirs
	@$(if $(deps_get_$(1)),$(call _deps_gen_get,$(1),$(deps_get_$(1))))
	@$(if $(deps_patch_$(1)),$(call _projmk_gen_deppatch,$(1),$(deps_patch_$(1))))
	touch $$@

endef

######################################################################
# TEMPLATES FOR CLEANDEPS TARGETS
######################################################################
define _deps_gen_cleandep
	$(if $(deps_ttype_$(1)),TARGET_TYPE=$(deps_ttype_$(1))) $(MAKE) -C $(DEPSDIR)/src/$(1) clean-all
	$(if $(deps_ttype_$(1)),TARGET_TYPE=$(deps_ttype_$(1))) $(MAKE) -C $(DEPSDIR)/src/$(1) clean-deps
endef


build: build-pre $(TARGET) build-post

build-pre::
	@$(call projmk_infomsg,$(PROJECT): BUILD me)

build-post::

# Generate rules for dependencies
$(foreach dep,$(DEPS),$(eval $(call _deps_gen_targets,$(dep))))

# Generate include statements for dependencies
ifeq ($(or $(if $(filter deps_ tests,$(MAKECMDGOALS)),yes),$(if $(MAKECMDGOALS),,yes)),yes)
DEPS_BACKUP := $(DEPS)
#$(shell echo $(DEPS) >&2)
$(foreach dep,$(DEPS_BACKUP),$(eval $(call _deps_gen_incl,$(dep))))
DEPS := $(DEPS_BACKUP)
endif

# Generate export statements for all USE_* variables, deps_ttype_* variables
# and variables specified in DEPS_VARS_EXPORT variable.
$(foreach var,$(filter USE_%,$(.VARIABLES)),$(eval export $(var)))
$(foreach var,$(filter deps_get_%,$(.VARIABLES)),$(eval export $(var)))
$(foreach var,$(filter deps_patch_%,$(.VARIABLES)),$(eval export $(var)))
$(foreach var,$(filter deps_ttype_%,$(.VARIABLES)),$(eval export $(var)))
$(foreach var,$(DEPS_VARS_EXPORT),$(eval export $(var)))

# Add USE_* variables to CFLAGS
$(foreach var,$(filter USE_%,$(.VARIABLES)),$(eval CFLAGS += -D$(var)=$($(var))))

# Remove duplicates words from variables values which names specified
# in DEPS_VARS_RMDUPS variable.
$(foreach var,$(DEPS_VARS_RMDUPS),$(eval $(var) := $(shell echo $($(var)) | $(PROJMKDIR)/rmdups.sh)))

qqq:
	echo $(DEPS)
	echo $(LDFLAGS)

projmk_gendepsinfo:
	@$(foreach dep,$(DEPS),echo include $(DEPSDIR)/src/$(dep).proj.mk.info >> $(DEPSDIR)/src/$(PROJECT).proj.mk.info)

deps:
	rm -f $(DEPSDIR)/src/*.proj.mk.info
	$(MAKE) deps_

deps_:

$(DEPSDIR)/.deps_dirs:
	install -d $(DEPSDIR)
	install -d $(DEPSDIR)/lib
	install -d $(DEPSDIR)/include
	install -d $(DEPSDIR)/src
	touch $@

deps_show:
	@echo $(DEPS)
	$(foreach dep,$(DEPS),$(call _deps_gen_show,$(dep)))

# Use $(shell pwd)/deps instead of $(DEPSDIR) because we
# don't want to delete $(PROJDIR)/deps on 'make clean-deps'
# called on a parent project when current project is a dependency.
clean-all:: clean clean-tests
	rm -rf $(shell pwd)/deps
	rm -f $(TARGET)

clean-deps::
	$(foreach dep,$(DEPS),$(call _deps_gen_cleandep,$(dep)))

clean-tests::
	$(MAKE) -C tests clean || true

clean::
	rm -f *~ *.$(O)

tests:
	@$(call projmk_infomsg,$(PROJECT): RUN TESTS)
	@$(MAKE) --no-print-directory -C tests
