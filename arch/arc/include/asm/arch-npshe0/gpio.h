/*
 * Copyright (C) 2012 EZchip, Inc. (www.ezchip.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __MESON_GPIO_H__
#define __MESON_GPIO_H__

typedef enum gpio_bank {
	PREG_EGPIO = 0,
	PREG_FGPIO,
	PREG_GGPIO
} gpio_bank_t;

typedef enum gpio_mode {
	GPIO_OUTPUT_MODE,
	GPIO_INPUT_MODE,
} gpio_mode_t;

int set_gpio_mode(gpio_bank_t bank, int bit, gpio_mode_t mode);
gpio_mode_t get_gpio_mode(gpio_bank_t bank, int bit);

int set_gpio_val(gpio_bank_t bank, int bit, unsigned long val);
unsigned long  get_gpio_val(gpio_bank_t bank, int bit);


#define GPIOA_bank_bit(bit)     (PREG_EGPIO)
#define GPIOA_bit_bit0_17(bit)  (bit)

#define GPIOB_bank_bit0_7(bit)  (PREG_FGPIO)
#define GPIOB_bit_bit0_7(bit)   (bit)

#define GPIOC_bank_bit0_23(bit) (PREG_FGPIO)
#define GPIOC_bit_bit0_23(bit)  (bit + 8)

#define GPIOD_bank_bit0_11(bit) (PREG_GGPIO)
#define GPIOD_bit_bit0_11(bit)  (bit + 16)

#define GPIOE_bank_bit0_15(bit) (PREG_GGPIO)
#define GPIOE_bit_bit0_15(bit)  (bit)

#define GPIOF_bank_bit0_9(bit)  (PREG_EGPIO)
#define GPIOF_bit_bit0_9(bit)   (bit + 18)

/*
 * enable gpio edge interrupt
 *
 * pin  index number of the chip, start with 0 up to 255
 * flag rising(0) or falling(1) edge
 * group  this interrupt belong to which interrupt group  from 0 to 7
 */
extern void gpio_enable_edge_int(int pin, int flag, int group);

/*
 * enable gpio level interrupt
 *
 * pin  index number of the chip, start with 0 up to 255
 * flag high(0) or low(1) level
 * group  this interrupt belong to which interrupt group  from 0 to 7
 */
extern void gpio_enable_level_int(int pin, int flag, int group);

/*
 * enable gpio interrupt filter
 *
 * filter from 0~7(*222ns)
 * group  this interrupt belong to which interrupt group  from 0 to 7
 */
extern void gpio_enable_int_filter(int filter, int group);

extern int gpio_is_valid(int number);
extern int gpio_request(unsigned gpio, const char *label);
extern void gpio_free(unsigned gpio);
extern int gpio_direction_input(unsigned gpio);
extern int gpio_direction_output(unsigned gpio, int value);
extern void gpio_set_value(unsigned gpio, int value);
extern int gpio_get_value(unsigned gpio);

#endif
