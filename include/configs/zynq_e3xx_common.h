/*
 * (C) Copyright 2015 National Instruments Corp
 *
 * Common Configuration settings for the Ettus Research E3xx
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_ZYNQ_E3XX_COMMON_H
#define __CONFIG_ZYNQ_E3XX_COMMON_H

#define CONFIG_ZYNQ_E3XX_MEM_TEST
#define CONFIG_ZYNQ_E3XX_MEM_TEST_START CONFIG_SYS_SDRAM_BASE
#define CONFIG_ZYNQ_E3XX_MEM_TEST_SIZE 0x00001000

/* we'll need to overridedefault zynq stuff */
#ifdef CONFIG_EXTRA_ENV_SETTINGS
#undef CONFIG_EXTRA_ENV_SETTINGS
#endif

#define CONFIG_EXTRA_ENV_SETTINGS	\
	"kernel_image=uImage\0"	\
	"kernel_load_address=0x2080000\0" \
	"load_addr=0x2000000\0"		\
	"devicetree_load_address=0x2000000\0"\
	"fdt_high=0x20000000\0"		\
	"initrd_high=0x20000000\0"	\
	"loadbit_addr=0x100000\0"	\
	"loadbit_size=0x3dbafc\0"	\
	"bitstream_image=fpga.bin\0"	\
	"sdboot=echo Loading fpga image from SD to HW... && " \
		"load mmc 0 ${loadbit_addr} ${bitstream_image} && " \
		"fpga load 0 ${loadbit_addr} ${filesize} && " \
		"echo Copying kernel from SD to RAM... && " \
		"load mmc 0 ${kernel_load_address} ${kernel_image} && " \
		"load mmc 0 ${devicetree_load_address} ${devicetree_image} && " \
		"bootm ${kernel_load_address} - ${devicetree_load_address}\0" \
	"jtagboot=echo TFTPing FIT to RAM... && " \
		"tftpboot ${load_addr} ${fit_image} && " \
		"bootm ${load_addr}\0" \
	"usbboot=if usb start; then " \
			"echo Copying FIT from USB to RAM... && " \
			"load usb 0 ${load_addr} ${fit_image} && " \
			"bootm ${load_addr}\0" \
		"fi\0" \
		DFU_ALT_INFO


#endif /* __CONFIG_ZYNQ_E3XX_COMMON_H */
