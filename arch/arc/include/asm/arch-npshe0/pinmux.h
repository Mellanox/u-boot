/*
 * Copyright (C) 2012 EZchip, Inc. (www.ezchip.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef MESON_PINMUX_H
#define MESON_PINMUX_H

/*
 * Linux Pinmux.h
 */

int clear_mio_mux(unsigned mux_index, unsigned mux_mask);
int set_mio_mux(unsigned mux_index, unsigned mux_mask);
void  clearall_pinmux(void);

#endif
