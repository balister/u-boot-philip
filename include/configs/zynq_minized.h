/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2019 Philip Balister <philip@balister.org>.
 *
 * Configuration settings for the MiniZed board
 * See zynq-common.h for Zynq common configs
 */

#ifndef __CONFIG_ZYNQ_MINIZED_H
#define __CONFIG_ZYNQ_MINIZED_H

#define CONFIG_EXTRA_ENV_SETTINGS       \
	"fit_image=fitImage\0"           \
	"bit_file=fpga.bit\0"           \
	"load_addr=0x2000000\0"         \
	"fit_size=0x800000\0"           \
	"flash_off=0x100000\0"          \
	"nor_flash_off=0xE2100000\0"    \
	"fdt_high=0x20000000\0"         \
	"initrd_high=0x20000000\0"      \
	"loadbootenv_addr=0x2000000\0" \
	"fdt_addr_r=0x1f00000\0"        \
	"kernel_addr_r=0x2000000\0"     \
	"ramdisk_addr_r=0x3100000\0"    \
	"bootenv=uEnv.txt\0" \
	"bootenv_dev=mmc\0" \
	"qspiboot=echo Copying FIT from QSPI to RAM... (Not Yet) \0" \
	"mmcboot= " \
		"echo Copying fpga.bit from MMC and loading FPGA... && " \
		"load mmc 0:1 ${load_addr} /boot/${bit_file} && " \
		"fpga loadb 0 ${load_addr} ${filesize} && " \
		"echo Copying FIT from MMC to RAM... && " \
		"load mmc 0:1 ${load_addr} /boot/${fit_image} && " \
		"setenv bootargs root=/dev/mmcblk1p1 rootwait && " \
		"bootm ${load_addr}\0" \
	"usbboot=if usb start; then " \
		"echo Copying fpga.bit from USB and loading FPGA... && " \
		"load usb 0 ${load_addr} /boot/${bit_file} && " \
		"fpga loadb 0 ${load_addr} ${filesize} && " \
		"echo Copying FIT from USB to RAM... && " \
		"load usb 0 ${load_addr} /boot/${fit_image} && " \
		"setenv bootargs root=/dev/sda1 rootwait && " \
		"bootm ${load_addr}; fi\0" \
	"usbrecover=if usb start; then " \
		"echo Copying fpga.bit from USB and loading FPGA... && " \
		"load usb 0 ${load_addr} ${bit_file} && " \
		"fpga loadb 0 ${load_addr} ${filesize} && " \
		"echo Copying FIT from USB to RAM... && " \
		"load usb 0 ${load_addr} ${fit_image} && " \
		"bootm ${load_addr}; fi\0"

#include <configs/zynq-common.h>

#endif /* __CONFIG_ZYNQ_MINIZED_H */
