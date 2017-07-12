#
# Copyright (c) 2016, Mellanox Technologies. All rights reserved.
#
# This software is available to you under a choice of one of two
# licenses.  You may choose to be licensed under the terms of the GNU
# General Public License (GPL) Version 2, available from the file
# COPYING in the main directory of this source tree, or the
# OpenIB.org BSD license below:
#
#     Redistribution and use in source and binary forms, with or
#     without modification, are permitted provided that the following
#     conditions are met:
#
#      - Redistributions of source code must retain the above
#        copyright notice, this list of conditions and the following
#        disclaimer.
#
#      - Redistributions in binary form must reproduce the above
#        copyright notice, this list of conditions and the following
#        disclaimer in the documentation and/or other materials
#        provided with the distribution.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
# BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
# ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

ifeq ($(CONFIG_SPL_BUILD),y)
extra-$(CONFIG_TARGET_NPS_MINIHE) += nps-miniHE_spl/start.o
extra-$(CONFIG_TARGET_NPS_SIM) += nps-sim_spl/start.o 
extra-$(CONFIG_TARGET_NPS_HE) += nps-HE_spl/start.o
extra-$(CONFIG_TARGET_NPS_SOC) += nps-soc_spl/start.o
extra-$(CONFIG_TARGET_NPS_IDG4400) += nps-idg4400_spl/start.o

obj-$(CONFIG_TARGET_NPS_MINIHE) += nps-miniHE_spl/spl.o
obj-$(CONFIG_TARGET_NPS_SIM) += nps-sim_spl/spl.o
obj-$(CONFIG_TARGET_NPS_HE) += nps-HE_spl/spl.o
obj-$(CONFIG_TARGET_NPS_SOC) += nps-soc_spl/spl.o
obj-$(CONFIG_TARGET_NPS_IDG4400) += nps-idg4400_spl/spl.o

ifneq ($(CONFIG_TARGET_NPS_MINIHE),y)
ifeq ($(CONFIG_TARGET_NPS_IDG4400),y)
obj-y += nps-idg4400_spl/spl_common.o
obj-y += nps-idg4400_spl/gen3_ucode.o
else
obj-y += nps-soc_spl/spl_common.o
obj-y += nps-soc_spl/gen3_ucode.o
endif
endif
subdir-$(CONFIG_TARGET_NPS_MINIHE) += ppboot
endif

ifneq ($(CONFIG_SPL_BUILD),y)
obj-y += nps.o
obj-$(CONFIG_NPS_ETH) += serdes.o
obj-$(CONFIG_TARGET_NPS_HE) += ddr_he.o
ifneq ($(CONFIG_TARGET_NPS_MINIHE),y)
obj-y += chip.o
ifneq ($(CONFIG_TARGET_NPS_HE),y)
obj-y += ddr.o
obj-y += bist.o
obj-$(CONFIG_NPS_DDR_DEBUG) += ddr_debug.o
endif
endif
endif

obj-y += common.o
