#
# Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#

CROSS_COMPILE ?= arc-elf32-

PLATFORM_CPPFLAGS+= -D__ARC__ -DCONFIG_ARC

LDSCRIPT := $(SRCTREE)/$(CPUDIR)/u-boot.lds

# Load address for standalone apps
CONFIG_STANDALONE_LOAD_ADDR = 0xc100000
