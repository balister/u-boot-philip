/*
 * (C) Copyright 2012 Michal Simek <monstr@monstr.eu>
 * (C) Copyright 2015 Moritz Fischer <moritz.fischer@ettus.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fdtdec.h>
#include <fpga.h>
#include <mmc.h>
#include <netdev.h>
#include <i2c.h>
#include <zynqpl.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

#if (defined(CONFIG_FPGA) && !defined(CONFIG_SPL_BUILD)) || \
    (defined(CONFIG_SPL_FPGA_SUPPORT) && defined(CONFIG_SPL_BUILD))
static xilinx_desc fpga;

/* It can be done differently */
static xilinx_desc fpga010 = XILINX_XC7Z010_DESC(0x10);
static xilinx_desc fpga015 = XILINX_XC7Z015_DESC(0x15);
static xilinx_desc fpga020 = XILINX_XC7Z020_DESC(0x20);
static xilinx_desc fpga030 = XILINX_XC7Z030_DESC(0x30);
static xilinx_desc fpga035 = XILINX_XC7Z035_DESC(0x35);
static xilinx_desc fpga045 = XILINX_XC7Z045_DESC(0x45);
static xilinx_desc fpga100 = XILINX_XC7Z100_DESC(0x100);
#endif

#define E3XX_I2C_DB_EEPROM_ADDR 0x50
#define E3XX_I2C_MB_EEPROM_ADDR 0x51

#define E3XX_MB_SERIAL_LEN 8
#define E3XX_MB_NAME_LEN 32

struct e3xx_db_eeprom_map {
	u16 data_version_major;
	u16 data_version_minor;
	u16 hw_product;
	u16 hw_revision;
	u8 serial[8];
	u8 pad [12];
};

struct e3xx_mb_eeprom_map {
	u16 data_version_major;
	u16 data_version_minor;
	u8 mac_addr[6];
	u16 hw_product;
	u16 hw_revision;
	u8 serial[E3XX_MB_SERIAL_LEN];
	u8 pad[20 - E3XX_MB_SERIAL_LEN];
	u8 user_name[E3XX_MB_NAME_LEN];
};

#define DB_ID_E310	0x0110
#define DB_ID_E330	0x1110

#define MB_ID_E3XX_OLD 0x77d2
#define MB_ID_E3XX_NEW 0x77d3

static const u8 e3xx_mb_speedgrades[] = {
	[MB_ID_E3XX_OLD] = 1,
	[MB_ID_E3XX_NEW] = 3,
};

static void setup_i2c(void)
{
	i2c_init(0,0);
	i2c_set_bus_num(0);
}

#ifdef CONFIG_ZYNQ_E3XX_MEM_TEST
/* Memory test with two parts:
 * 1) Check data bits are valid
 * 2) Write a test pattern to RAM and check the read back values. This should
 * catch bad / stuck address lines.
 */
static const u32 test_patterns[7] = {0x5555AAAA, 0xAAAA5555, 0x5A5AA5A5, 0xA5A55A5A,
			0xF0F00F0F, 0x0F0FF0F0, 0x0000FFFF};

static int mem_test(u32 start, u32 size)
{
	u32* address;
	u32 i;

	return 0;

	/* Walking 1s pattern on a single address */
	address = (u32*)start;
	for (i = 0; i < 8*sizeof(u32); i++) {
		*address = 1 << i;
		if (*address != 1 << i) {
			return -1;
		}
	}

	/* Check test pattern */
	for (i = start; i < size/sizeof(u32); i++) {
		if (address[i] != test_patterns[i & 0x7])
			return -1;
	}

	return 0;
}
#endif /* CONFIG_ZYNQ_E3XX_MEM_TEST */

#define BOARD_SWITCH_GPIO 63
#define BOARD_SAFE_GPIO 64

static int board_set_db_mux_gpio(int is_safe, int is_e33x)
{
	int err;

	err = gpio_request(BOARD_SWITCH_GPIO,
			   "board_switch_gpio");
	if (err) {
		printf("Could not get db_mux_gpio(0)\n");
		return err;
	}

	gpio_direction_output(BOARD_SWITCH_GPIO, is_e33x);

	gpio_free(BOARD_SWITCH_GPIO);

	err = gpio_request(BOARD_SAFE_GPIO,
			   "board_safe_gpio");
	if (err) {
		printf("Could not get db_mux_gpio(1)\n");
		return err;
	}

	gpio_direction_output(BOARD_SAFE_GPIO, is_safe);

	gpio_free(BOARD_SAFE_GPIO);

	return 0;
}

static inline char num_to_letter_rev(char x)
{
	return (((char) ('A' + x)));
}

static void identify_products(void)
{
	u8 db_buf[sizeof(struct e3xx_db_eeprom_map)];
	u8 mb_buf[sizeof(struct e3xx_mb_eeprom_map)];
	u16 mb, mb_rev, db, db_rev;
	u8 speedgrade;
	char mstr[20];

	struct e3xx_db_eeprom_map *db_map =
		(struct e3xx_db_eeprom_map*) &db_buf[0];

	struct e3xx_mb_eeprom_map *mb_map =
		(struct e3xx_mb_eeprom_map*) &mb_buf[0];

	setup_i2c();

	if (i2c_probe(E3XX_I2C_MB_EEPROM_ADDR) != 0) {
		printf("Couldn't find i2c mb eeprom\n");
		return;
	};

	if (i2c_read(E3XX_I2C_MB_EEPROM_ADDR, 0, 1, mb_buf,
		     sizeof(*mb_map))) {
		printf("i2c mb eeprom read failed\n");
	};
	mb = ntohs(mb_map->hw_product);
	mb_rev = ntohs(mb_map->hw_revision);

	if (i2c_probe(E3XX_I2C_DB_EEPROM_ADDR) != 0) {
		printf("Couldn't find i2c db eeprom\n");
		return;
	};

	if (i2c_read(E3XX_I2C_DB_EEPROM_ADDR, 0, 2, db_buf,
		     sizeof(*db_map))) {
		printf("i2c db eeprom read failed\n");
	};
	db = ntohs(db_map->hw_product);
	db_rev = ntohs(db_map->hw_revision);

	/* print out motherboard info */
	if (mb == MB_ID_E3XX_OLD) {
		printf("MB: Found E3XX Rev%c - Speedgrade %u\n",
		       num_to_letter_rev(mb_rev),
		       e3xx_mb_speedgrades[MB_ID_E3XX_OLD]);
		speedgrade = e3xx_mb_speedgrades[MB_ID_E3XX_OLD];
	} else if (mb == MB_ID_E3XX_NEW) {
		printf("MB: Found E3XX Rev%c - Speedgrade %u\n",
		       num_to_letter_rev(mb_rev),
		       e3xx_mb_speedgrades[MB_ID_E3XX_NEW]);
		speedgrade = e3xx_mb_speedgrades[MB_ID_E3XX_NEW];
	} else {
		speedgrade = 0;
		printf("*** Found unknown motherboard, please update sd card ***\n");
		setenv("devicetree_image", "uImage-zynq-e3xx-factory.dtb");
	}

	/* print out daughterboard info and select correct image */
	if (speedgrade == 1) {
		if (db == DB_ID_E310) {
			setenv("devicetree_image", "uImage-zynq-e31x-1.dtb");
			printf("DB: Found E310 MIMO XCVR Rev%c\n",
			       num_to_letter_rev(db_rev));
			board_set_db_mux_gpio(1,0);
		} else if (db == DB_ID_E330) {
			setenv("devicetree_image", "uImage-zynq-e33x-1.dtb");
			printf("DB: Found E330 MIMO RCVR Rev%c\n",
			       num_to_letter_rev(db_rev));
			board_set_db_mux_gpio(1,1);
		} else {
			setenv("devicetree_image", "uImage-zynq-e3xx-factory.dtb");
			printf("*** Found unknown daughterboard, 0x%04x please update sd card ***\n", db);
			board_set_db_mux_gpio(0,0);
		}
	} else if (speedgrade == 3) {
		if (db == DB_ID_E310) {
			setenv("devicetree_image", "uImage-zynq-e31x-3.dtb");
			printf("DB: Found E310 MIMO XCVR Rev%c\n",
			       num_to_letter_rev(db_rev));
			board_set_db_mux_gpio(1,0);
		} else if (db == DB_ID_E330) {
			setenv("devicetree_image", "uImage-zynq-e33x-3.dtb");
			printf("DB: Found E330 MIMO RCVR Rev%c\n",
			       num_to_letter_rev(db_rev));
			board_set_db_mux_gpio(1,1);
		} else {
			setenv("devicetree_image", "uImage-zynq-e3xx-factory.dtb");
			printf("*** Found unknown daughterboard, 0x%04x please update sd card ***\n", db);
			board_set_db_mux_gpio(0,0);
		}
	}

	/* grab mac address */
	sprintf(mstr, "%0X:%0X:%0X:%0X:%0X:%0X",mb_map->mac_addr[0],
		mb_map->mac_addr[1], mb_map->mac_addr[2], mb_map->mac_addr[3],
		mb_map->mac_addr[4], mb_map->mac_addr[5]);
	setenv("ethaddr", mstr);
}


int board_init(void)
{
#if (defined(CONFIG_FPGA) && !defined(CONFIG_SPL_BUILD)) || \
    (defined(CONFIG_SPL_FPGA_SUPPORT) && defined(CONFIG_SPL_BUILD))
	u32 idcode;

	idcode = zynq_slcr_get_idcode();

	switch (idcode) {
	case XILINX_ZYNQ_7010:
		fpga = fpga010;
		break;
	case XILINX_ZYNQ_7015:
		fpga = fpga015;
		break;
	case XILINX_ZYNQ_7020:
		fpga = fpga020;
		break;
	case XILINX_ZYNQ_7030:
		fpga = fpga030;
		break;
	case XILINX_ZYNQ_7035:
		fpga = fpga035;
		break;
	case XILINX_ZYNQ_7045:
		fpga = fpga045;
		break;
	case XILINX_ZYNQ_7100:
		fpga = fpga100;
		break;
	}
#endif

#if (defined(CONFIG_FPGA) && !defined(CONFIG_SPL_BUILD)) || \
    (defined(CONFIG_SPL_FPGA_SUPPORT) && defined(CONFIG_SPL_BUILD))
	fpga_init();
	fpga_add(fpga_xilinx, &fpga);
#endif

	return 0;
}

int board_late_init(void)
{
	int err;

	switch ((zynq_slcr_get_boot_mode()) & ZYNQ_BM_MASK) {
	case ZYNQ_BM_NOR:
		setenv("modeboot", "norboot");
		break;
	case ZYNQ_BM_SD:
		setenv("modeboot", "sdboot");
		break;
	case ZYNQ_BM_JTAG:
		setenv("modeboot", "jtagboot");
		break;
	default:
		setenv("modeboot", "");
		break;
	}

	identify_products();

#ifdef CONFIG_ZYNQ_E3XX_MEM_TEST
	printf("RAM test... ");

	err = mem_test(CONFIG_ZYNQ_E3XX_MEM_TEST_START,
		       CONFIG_ZYNQ_E3XX_MEM_TEST_SIZE);
	if (err) {
		printf("FAILED RAM TEST!\n");
		setenv("bootdelay","-1");
	} else
		printf("PASSED RAM TEST!\n");
#endif /* CONFIG_ZYNQ_E3XX_MEM_TEST */

	return 0;
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	puts("Board:\tNI Ettus Research USRP E3xx SDR\n");
	return 0;
}
#endif

int board_eth_init(bd_t *bis)
{
	u32 ret = 0;

#ifdef CONFIG_XILINX_AXIEMAC
	ret |= xilinx_axiemac_initialize(bis, XILINX_AXIEMAC_BASEADDR,
						XILINX_AXIDMA_BASEADDR);
#endif
#ifdef CONFIG_XILINX_EMACLITE
	u32 txpp = 0;
	u32 rxpp = 0;
# ifdef CONFIG_XILINX_EMACLITE_TX_PING_PONG
	txpp = 1;
# endif
# ifdef CONFIG_XILINX_EMACLITE_RX_PING_PONG
	rxpp = 1;
# endif
	ret |= xilinx_emaclite_initialize(bis, XILINX_EMACLITE_BASEADDR,
			txpp, rxpp);
#endif

#if defined(CONFIG_ZYNQ_GEM)
# if defined(CONFIG_ZYNQ_GEM0)
	ret |= zynq_gem_initialize(bis, ZYNQ_GEM_BASEADDR0,
				   CONFIG_ZYNQ_GEM_PHY_ADDR0,
				   CONFIG_ZYNQ_GEM_EMIO0);
# endif
# if defined(CONFIG_ZYNQ_GEM1)
	ret |= zynq_gem_initialize(bis, ZYNQ_GEM_BASEADDR1,
				   CONFIG_ZYNQ_GEM_PHY_ADDR1,
				   CONFIG_ZYNQ_GEM_EMIO1);
# endif
#endif
	return ret;
}

#ifdef CONFIG_CMD_MMC
int board_mmc_init(bd_t *bd)
{
	int ret = 0;

#if defined(CONFIG_ZYNQ_SDHCI)
# if defined(CONFIG_ZYNQ_SDHCI0)
	ret = zynq_sdhci_init(ZYNQ_SDHCI_BASEADDR0);
# endif
# if defined(CONFIG_ZYNQ_SDHCI1)
	ret |= zynq_sdhci_init(ZYNQ_SDHCI_BASEADDR1);
# endif
#endif
	return ret;
}
#endif

int dram_init(void)
{
#if CONFIG_IS_ENABLED(OF_CONTROL)
	int node;
	fdt_addr_t addr;
	fdt_size_t size;
	const void *blob = gd->fdt_blob;

	node = fdt_node_offset_by_prop_value(blob, -1, "device_type",
					     "memory", 7);
	if (node == -FDT_ERR_NOTFOUND) {
		debug("ZYNQ DRAM: Can't get memory node\n");
		return -1;
	}
	addr = fdtdec_get_addr_size(blob, node, "reg", &size);
	if (addr == FDT_ADDR_T_NONE || size == 0) {
		debug("ZYNQ DRAM: Can't get base address or size\n");
		return -1;
	}
	gd->ram_size = size;
#else
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;
#endif
	zynq_ddrc_init();

	return 0;
}
