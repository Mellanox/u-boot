/*
*	Copyright (C) 2015 EZchip, Inc. (www.ezchip.com)
*
* 	This program is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License version 2 as
*	published by the Free Software Foundation.
*/

#ifndef _DDR_H_
#define _DDR_H_

/* Memory interface block */
#define EMEM_MI_START_BLOCK1_ID		0x610
#define EMEM_MI_END_BLOCK1_ID		0x615
#define EMEM_MI_START_BLOCK2_ID		0x710
#define EMEM_MI_END_BLOCK2_ID		0x715

/* L2C block */
#define L2C_START_BLOCK1_ID		0x600
#define L2C_END_BLOCK1_ID		0x607
#define L2C_START_BLOCK2_ID		0x700
#define L2C_END_BLOCK2_ID		0x707

/* Memory controller block */
#define MEMORY_CONTROLLER_START_BLOCK1_ID		0x618
#define MEMORY_CONTROLLER_END_BLOCK1_ID			0x61D
#define MEMORY_CONTROLLER_START_BLOCK2_ID		0x718
#define MEMORY_CONTROLLER_END_BLOCK2_ID			0x71D

/* CRG block */
#define CRG_START_BLOCK_ID		0x12F
#define CRG_END_BLOCK_ID		0x13A

/* MSU block */
#define MSU_BLOCK_ID		0x18

/* CIU block */
#define CIU_BLOCK_ID		0x8

#define CLUSTER_X_VALUE		0
#define CLUSTER_Y_VALUE		0

void configure_emem(void);

#endif /* _DDR_H_ */
