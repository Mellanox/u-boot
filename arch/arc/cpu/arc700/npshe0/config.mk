#
# Copyright (C) 2012 EZchip, Inc. (www.ezchip.com)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#

PLATFORM_CPPFLAGS += -mA7 -mno-sdata -Os -g -fomit-frame-pointer
PLATFORM_LDFLAGS += -EB
CONFIG_SYS_TEXT_BASE = 0x9FF00000
