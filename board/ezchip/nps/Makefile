#
# Copyright (C) 2014 EZchip, Inc. (www.ezchip.com)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#

ifeq ($(CONFIG_SPL_BUILD),y)
ifdef CONFIG_NPS_MINI_HE
obj-$(CONFIG_SPL_BUILD) += nps-miniHE_spl/start.o 
obj-$(CONFIG_SPL_BUILD) += nps-miniHE_spl/spl.o
else
extra-y += nps400_spl/start.o
obj-y += nps400_spl/spl.o
endif
else
obj-y += nps.o
obj-y += nps-miniHE_spl/spl_cmd.o
obj-y += ddr.o 
subdir-y += ppboot
endif
obj-y += common.o

