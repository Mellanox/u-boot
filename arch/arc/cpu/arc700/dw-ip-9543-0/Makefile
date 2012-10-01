#
# Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#

include $(TOPDIR)/config.mk

LIB	=  $(obj)lib$(SOC).o

COBJS	+=  soc.o timer.o clock.o pinmux.o gpio.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS) $(SOBJS) $(COBJS-y))

all:	$(obj).depend $(LIB)

$(LIB):	$(OBJS)
		$(call cmd_link_o_target, $(OBJS))


#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
