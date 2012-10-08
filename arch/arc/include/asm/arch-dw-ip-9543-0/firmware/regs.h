/*
 * Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __MACH_FIRMWARE_REGS_H
#define __MACH_FIRMWARE_REGS_H

#define IO_CBUS_BASE			0xc1100000
#define IO_AXI_BUS_BASE			0xc1300000
#define IO_AHB_BUS_BASE			0xc9000000
#define IO_APB_BUS_BASE			0xd0040000

#define CBUS_REG_OFFSET(reg) ((reg) << 2)
#define CBUS_REG_ADDR(reg)	 (IO_CBUS_BASE + CBUS_REG_OFFSET(reg))

#define AXI_REG_OFFSET(reg)  ((reg) << 2)
#define AXI_REG_ADDR(reg)	 (IO_AXI_BUS_BASE + AXI_REG_OFFSET(reg))

#define AHB_REG_OFFSET(reg)  ((reg) << 2)
#define AHB_REG_ADDR(reg)	 (IO_AHB_BUS_BASE + AHB_REG_OFFSET(reg))

#define APB_REG_OFFSET(reg)  (reg)
#define APB_REG_ADDR(reg)	 (IO_APB_BUS_BASE + APB_REG_OFFSET(reg))
#define APB_REG_ADDR_VALID(reg) (((unsigned long)(reg) & 3) == 0)

#define SCR_HIU                     0x100b
#define HPG_TIMER                   0x100f
#define HARM_ASB_MB0                0x1030
#define HARM_ASB_MB1                0x1031
#define HARM_ASB_MB2                0x1032
#define HARM_ASB_MB3                0x1033
#define HASB_ARM_MB0                0x1034
#define HASB_ARM_MB1                0x1035
#define HASB_ARM_MB2                0x1036
#define HASB_ARM_MB3                0x1037
#define HHI_TIMER90K                0x103b
#define HHI_AUD_DAC_CTRL            0x1044
#define HHI_SYS_PLL_CNTL2           0x104d
#define HHI_AUD_PLL_CNTL2           0x104e
#define HHI_VID_PLL_CNTL2           0x104f
#define HHI_GCLK_MPEG0              0x1050
#define HHI_GCLK_MPEG1              0x1051
#define HHI_GCLK_MPEG2              0x1052
#define HHI_GCLK_OTHER              0x1054
#define HHI_SYS_PLL_CNTL3           0x1056
#define HHI_AUD_PLL_CNTL3           0x1057
#define HHI_VID_PLL_CNTL3           0x1058
#define HHI_VID_CLK_DIV             0x1059
#define HHI_SYS_PLL_CNTL            0x105a
#define HHI_AUD_PLL_CNTL            0x105b
#define HHI_VID_PLL_CNTL            0x105c
#define HHI_MPEG_CLK_CNTL           0x105d
#define HHI_AUD_CLK_CNTL            0x105e
#define HHI_VID_CLK_CNTL            0x105f
#define HHI_WIFI_CLK_CNTL           0x1060
#define HHI_WIFI_PLL_CNTL           0x1061
#define HHI_WIFI_PLL_CNTL2          0x1062
#define HHI_WIFI_PLL_CNTL3          0x1063
#define HHI_AUD_CLK_CNTL2           0x1064
#define HHI_VID_PLL_CNTL4           0x1065
#define HHI_VID_DIVIDER_CNTL        0x1066
#define HHI_SYS_CPU_CLK_CNTL        0x1067
#define HHI_DDR_PLL_CNTL            0x1068
#define HHI_DDR_PLL_CNTL2           0x1069
#define HHI_DDR_PLL_CNTL3           0x106a
#define HHI_MALI_CLK_CNTL           0x106c
#define HHI_DEMOD_PLL_CNTL          0x106d
#define HHI_DEMOD_PLL_CNTL2         0x106e
#define HHI_DEMOD_PLL_CNTL3         0x106f
#define HHI_DEMOD_PLL_CNTL4         0x106b
#define HHI_OTHER_PLL_CNTL          0x1070
#define HHI_OTHER_PLL_CNTL2         0x1071
#define HHI_OTHER_PLL_CNTL3         0x1072
#define HHI_HDMI_CLK_CNTL           0x1073
#define HHI_DEMOD_CLK_CNTL          0x1074
#define HHI_SATA_CLK_CNTL           0x1075
#define HHI_ETH_CLK_CNTL            0x1076
#define HHI_CLK_DOUBLE_CNTL         0x1077
#define HHI_SYS_CPU_AUTO_CLK0       0x1078
#define HHI_SYS_CPU_AUTO_CLK1       0x1079
#define HHI_MEDIA_CPU_AUTO_CLK0     0x107a
#define HHI_MEDIA_CPU_AUTO_CLK1     0x107b
#define HHI_HDMI_PLL_CNTL           0x107c
#define HHI_HDMI_PLL_CNTL1          0x107d
#define HHI_HDMI_PLL_CNTL2          0x107e
#define HHI_HDMI_AFC_CNTL           0x107f
#define HHI_AUD_PLL_MOD_CNTL0       0x1080
#define HHI_AUD_PLL_MOD_LOW_TCNT    0x1081
#define HHI_AUD_PLL_MOD_HIGH_TCNT   0x1082
#define HHI_AUD_PLL_MOD_NOM_TCNT    0x1083
#define HHI_VID_PLL_MOD_CNTL0       0x1084
#define HHI_VID_PLL_MOD_LOW_TCNT    0x1085
#define HHI_VID_PLL_MOD_HIGH_TCNT   0x1086
#define HHI_VID_PLL_MOD_NOM_TCNT    0x1087
#define HHI_JTAG_CONFIG             0x108e
#define HHI_VAFE_CLKXTALIN_CNTL     0x108f
#define HHI_VAFE_CLKOSCIN_CNTL      0x1090
#define HHI_VAFE_CLKIN_CNTL         0x1091
#define HHI_TVFE_AUTOMODE_CLK_CNTL  0x1092
#define HHI_VAFE_CLKPI_CNTL         0x1093
#define HHI_VDIN_MEAS_CLK_CNTL      0x1094

#define PREG_EGPIO_EN_N        0x200c
#define PREG_EGPIO_O           0x200d
#define PREG_EGPIO_I           0x200e
#define PREG_FGPIO_EN_N        0x200f
#define PREG_FGPIO_O           0x2010
#define PREG_FGPIO_I           0x2011
#define PREG_GGPIO_EN_N        0x2012
#define PREG_GGPIO_O           0x2013
#define PREG_GGPIO_I           0x2014
#define PREG_HGPIO_EN_N        0x2015
#define PREG_HGPIO_O           0x2016
#define PREG_HGPIO_I           0x2017

#define PERIPHS_PIN_MUX_0      0x202c
#define PERIPHS_PIN_MUX_1      0x202d
#define PERIPHS_PIN_MUX_2      0x202e
#define PERIPHS_PIN_MUX_3      0x202f
#define PERIPHS_PIN_MUX_4      0x2030
#define PERIPHS_PIN_MUX_5      0x2031
#define PERIPHS_PIN_MUX_6      0x2032
#define PERIPHS_PIN_MUX_7      0x2033
#define PERIPHS_PIN_MUX_8      0x2034
#define PERIPHS_PIN_MUX_9      0x2035
#define PERIPHS_PIN_MUX_10     0x2036
#define PERIPHS_PIN_MUX_11     0x2037
#define PERIPHS_PIN_MUX_12     0x2038

#define GPIO_INTR_EDGE_POL     0x2620
#define GPIO_INTR_GPIO_SEL0    0x2621
#define GPIO_INTR_GPIO_SEL1    0x2622
#define GPIO_INTR_FILTER_SEL0  0x2623

#define WATCHDOG_TC            0x2640
#define WATCHDOG_RESET         0x2641
#define WATCHDOG_ENABLE_BIT    22

#endif
