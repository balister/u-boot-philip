#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
# (C) Copyright 2015
# Moritz Fischer, National Instruments Corp, moritz.fischer@ettus.com
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y	:= board.o

hw-platform-$(CONFIG_TARGET_ETTUS_E3XX_1)		:= e3xx_hw_platform_1
hw-platform-$(CONFIG_TARGET_ETTUS_E3XX_3)		:= e3xx_hw_platform_3

init-objs := $(if $(wildcard $(srctree)/$(src)/$(hw-platform-y)/ps7_init_gpl.c),\
	$(hw-platform-y)/ps7_init_gpl.o)

obj-$(CONFIG_SPL_BUILD) += $(init-objs)

# Suppress "warning: function declaration isn't a prototype"
CFLAGS_REMOVE_ps7_init_gpl.o := -Wstrict-prototypes

# To include xil_io.h
CFLAGS_ps7_init_gpl.o := -I$(srctree)/$(src)
