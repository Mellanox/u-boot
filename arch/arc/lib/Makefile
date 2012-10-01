#
# Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(ARCH).o
AOBJS	=

COBJS	= board.o arclinux.o cache.o
COBJS	+= interrupts.o
COBJS-$(CONFIG_CMD_KGDB)+= kgdb.o
OBJS	= $(AOBJS) $(COBJS) $(sort $(COBJS-y))
OBJS	:= $(addprefix $(obj),$(OBJS))

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
