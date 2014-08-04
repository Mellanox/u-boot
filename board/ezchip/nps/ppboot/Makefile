#
# Copyright (C) 2014 EZchip, Inc. (www.ezchip.com)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#

extra-y        := ppboot

clean-files  := $(extra-) $(addsuffix .bin,$(extra-))
ELF := $(strip $(extra-y)) 

extra-y += $(addsuffix .bin,$(extra-y))

COBJS	:= $(ELF:=.o)
LIB	= $(obj)/libspiFlash2Ram.o

LIBOBJS-y += spiFlash2Ram.o

.SECONDARY: $(call objectify,$(COBJS))
targets += $(patsubst $(obj)/%,%,$(LIB)) $(COBJS) $(LIBOBJS-y)

LIBOBJS	:= $(addprefix $(obj)/,$(LIBOBJS-y))
ELF	:= $(addprefix $(obj)/,$(ELF))

gcclibdir := $(shell dirname `$(CC) -print-libgcc-file-name`)

#########################################################################

quiet_cmd_link_lib = LD      $@
      cmd_link_lib = $(LD) $(ld_flags) -r -o $@ $(filter $(LIBOBJS), $^)

$(LIB):	$(LIBOBJS) FORCE
	$(call if_changed,link_lib)

quiet_cmd_link_elf = LD      $@
      cmd_link_elf = $(LD) $(LDFLAGS) -g -Ttext 0xFFFFE000 \
		     -o $@ $< $(LIB) -L$(gcclibdir) -lgcc -T $(obj)/ppboot.lds

$(ELF): $(obj)/%: $(obj)/%.o $(LIB) FORCE
	$(call if_changed,link_elf)
	
$(obj)/%.bin: OBJCOPYFLAGS := --gap-fill=0xff -O binary
$(obj)/%.bin: $(obj)/% FORCE
	$(call if_changed,objcopy)	
