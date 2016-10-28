/*
 * (C) Copyright 2013 Xilinx, Inc.
 * (C) Copyright 2015 National Instruments Corp
 *
 * Configuration settings for the Ettus Research E3xx
 * See zynq-common.h for Zynq common configs
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_ZYNQ_E3XX_3_H
#define __CONFIG_ZYNQ_E3XX_3_H

#define CONFIG_SYS_SDRAM_SIZE		(1024 * 1024 * 1024)

#define CONFIG_ZYNQ_SERIAL_UART0
#define CONFIG_ZYNQ_GEM0
#define CONFIG_ZYNQ_GEM_PHY_ADDR0	7

#define CONFIG_SYS_NO_FLASH

#define CONFIG_ZYNQ_USB
#define CONFIG_ZYNQ_I2C0
#define CONFIG_ZYNQ_EEPROM
#define CONFIG_ZYNQ_BOOT_FREEBSD

#include <configs/zynq-common.h>
#include <configs/zynq_e3xx_common.h>


#endif /* __CONFIG_ZYNQ_E3XX_3_H */
