/*
 * (C) Copyright 2008-2015 Fuzhou Rockchip Electronics Co., Ltd
 * Peter, Software Engineering, <superpeter.cai@gmail.com>.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <version.h>
#include <errno.h>
#include <fastboot.h>
#include <fdtdec.h>
#include <fdt_support.h>
#include <power/pmic.h>

#include <asm/io.h>
#include <asm/arch/rkplat.h>

#include "../common/config.h"

#ifdef CONFIG_MAC_IN_SPI
#include <version.h>
#include <spi_flash.h>

struct boardcfg_t {
    unsigned char mac[12];
    unsigned char sn_len;
    unsigned char *sn;
    unsigned char time_len;
    unsigned char *Manufactoring_Time;
};
#endif

DECLARE_GLOBAL_DATA_PTR;
static ulong get_sp(void)
{
	ulong ret;

	asm("mov %0, sp" : "=r"(ret) : );
	return ret;
}

void board_lmb_reserve(struct lmb *lmb) {
	ulong sp;
	sp = get_sp();
	debug("## Current stack ends at 0x%08lx ", sp);

	/* adjust sp by 64K to be safe */
	sp -= 64<<10;
	lmb_reserve(lmb, sp,
			gd->bd->bi_dram[0].start + gd->bd->bi_dram[0].size - sp);

	//reserve 48M for kernel & 8M for nand api.
	lmb_reserve(lmb, gd->bd->bi_dram[0].start, CONFIG_LMB_RESERVE_SIZE);
}

int board_storage_init(void)
{
	int ret = 0;

	if (StorageInit() == 0) {
		printf("storage init OK!\n");
		ret = 0;
	} else {
		printf("storage init fail!\n");
		ret = -1;
	}

	return ret;
}


/*****************************************
 * Routine: board_init
 * Description: Early hardware init.
 *****************************************/
int board_init(void)
{
	/* Set Initial global variables */

	gd->bd->bi_arch_number = MACH_TYPE_RK32xx;
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x88000;

	return 0;
}

int board_early_init_f(void)
{
#ifdef CONFIG_SWITCH_DEBUG_PORT_TO_UART
	gpio_direction_input(DEBUG_SWITCH_GPIO);
	if (gpio_get_value(DEBUG_SWITCH_GPIO) == DEBUG_SWITCH_GPIO_ACTIVE) {
		gd->flags |= GD_FLG_DISABLE_CONSOLE;
		//reconfig iomux to defalt gpio
		grf_writel((7 << 28) | (3 << 24), GRF_GPIO7CH_IOMUX);
	}
#endif

	/* pull disable to fix 83867 phy address*/
#ifdef CONFIG_DP83867_PHY_ID
	grf_writel((3 << 28) | (3 << 20), GRF_GPIO3D_P);
#endif

#ifdef CONFIG_RESET_PMIC_GPIO
	if(SYS_LOADER_REBOOT_FLAG == IReadLoaderFlag())
	{
		gpio_direction_output(CONFIG_RESET_PMIC_GPIO,1);
		mdelay(5);
		gpio_direction_output(CONFIG_RESET_PMIC_GPIO,0);
		while(1);
	}
#endif

	return 0;
}

#ifdef CONFIG_DISPLAY_BOARDINFO
/**
 * Print board information
 */
int checkboard(void)
{
	uint32 ver=0;

#ifdef CONFIG_DISPLAY_BOARD_ID
	gpio_direction_input(HW_BOARD_ID2);
	gpio_direction_input(HW_BOARD_ID1);
	gpio_direction_input(HW_BOARD_ID0);
	ver = gpio_get_value(HW_BOARD_ID2);
	ver = (ver<<1) | gpio_get_value(HW_BOARD_ID1);
	ver = (ver<<1) | gpio_get_value(HW_BOARD_ID0);
#endif
	printf("Board:Advantech %s Board,HW version:%d\n",CONFIG_SYS_CONFIG_NAME,ver);
#ifdef CONFIG_SECOND_LEVEL_BOOTLOADER
	printf("Uboot as second level loader\n");
#endif
	return 0;
}
#endif


#ifdef CONFIG_ARCH_EARLY_INIT_R
int arch_early_init_r(void)
{
	debug("arch_early_init_r\n");

	 /* set up exceptions */
	interrupt_init();
	/* enable exceptions */
	enable_interrupts();

	/* rk pl330 dmac init */
#ifdef CONFIG_RK_PL330_DMAC
	rk_pl330_dmac_init_all();
#endif /* CONFIG_RK_PL330_DMAC */

#ifdef CONFIG_RK_PWM_REMOTE
	RemotectlInit();
#endif

	return 0;
}
#endif

#ifdef CONFIG_MAC_IN_SPI
static int board_info_in_spi(void)
{
    struct spi_flash *flash;
	uchar enetaddr[50];
	u32 valid;
	int sn_len,time_len;
	
	flash = spi_flash_probe(CONFIG_SF_DEFAULT_BUS, CONFIG_SF_DEFAULT_CS,
				CONFIG_SF_DEFAULT_SPEED, CONFIG_SF_DEFAULT_MODE);
	if (!flash)
		return -1;
	if(spi_flash_read(flash, CONFIG_SPI_MAC_OFFSET, 50, enetaddr)==0) {
		spi_flash_free(flash);
		valid = is_valid_ether_addr(enetaddr);

		if (valid)
			eth_setenv_enetaddr("ethaddr", enetaddr);
		else
			puts("Skipped ethaddr assignment due to invalid,using default!\n");

		sn_len = enetaddr[12];
		time_len = enetaddr[13+sn_len];
		if(sn_len && (sn_len != 0xff)) {
			enetaddr[13+sn_len] = '\0';
			setenv("androidboot.serialno", &enetaddr[13]);
			if(time_len && (time_len != 0xff)) {
				enetaddr[14+sn_len+time_len] = '\0';
				setenv("androidboot.factorytime", &enetaddr[14+sn_len]);
			}else
				setenv("androidboot.factorytime", NULL);
		} else {
			setenv("androidboot.serialno", NULL);
			setenv("androidboot.factorytime", NULL);
		}
	} else
		return -1;
}
#endif 

static void board_info_config()
{
	const disk_partition_t *ptn = NULL;
	unsigned long blksz;
	unsigned char *buf;

	ptn	= get_disk_partition(BOARD_INFO_NAME);
	if (ptn) {
		blksz = ptn->blksz;
		buf = memalign(ARCH_DMA_MINALIGN, blksz << 2);
		if (buf) {
			if (StorageReadLba(ptn->start, (void *) buf, 1) == 0) {
				buf[17] = 0;
				setenv("ethaddr",buf);
			} else
				printf("failed to read %s partition\n",BOARD_INFO_NAME);
			free(buf);
		} else
			printf("error allocating blksz(%lu) buffer\n", blksz);
	} else
		printf("failed to get %s partition\n",BOARD_INFO_NAME);
}

#define RAMDISK_ZERO_COPY_SETTING	"0xffffffff=n\0"
static void board_init_adjust_env(void)
{
	bool change = false;
	char *s;

	s = getenv("bootcmd");
	if (s != NULL) {
		debug("getenv: bootcmd = %s\n", s);
		if (strcmp(s, CONFIG_BOOTCOMMAND) != 0) {
			setenv("bootcmd", CONFIG_BOOTCOMMAND);
			change = true;
			debug("setenv: bootcmd = %s\n", CONFIG_BOOTCOMMAND);
		}
	}

	s = getenv("initrd_high");
	if (s != NULL) {
		debug("getenv: initrd_high = %s\n", s);
		if (strcmp(s, RAMDISK_ZERO_COPY_SETTING) != 0) {
			setenv("initrd_high", RAMDISK_ZERO_COPY_SETTING);
			change = true;
			debug("setenv: initrd_high = %s\n", RAMDISK_ZERO_COPY_SETTING);
		}
	}

	if (change) {
#ifdef CONFIG_CMD_SAVEENV
		debug("board init saveenv.\n");
		saveenv();
#endif
	}

#ifdef CONFIG_SWITCH_DEBUG_PORT_TO_UART
	if (gpio_get_value(DEBUG_SWITCH_GPIO) == DEBUG_SWITCH_GPIO_ACTIVE)
		setenv("switch_debug","yes");
	else
		setenv("switch_debug",NULL);
#endif
}

static void clear_rtc_irq_status(void)
{
	u8 buf=0;
	i2c_set_bus_num(CONFIG_RTC_I2C_BUS);
#ifdef CONFIG_RTC_S35390A
	i2c_init(100000, CONFIG_S35390A_ADDR);
	/* disable interrupt */
	i2c_write(CONFIG_S35390A_ADDR+1, 0, 0, &buf, 1);
	/* clear pending interrupt, if any */
	i2c_read(CONFIG_S35390A_ADDR, 0, 0, &buf, 1);
#endif
}

#ifdef CONFIG_BOARD_LATE_INIT
extern char bootloader_ver[24];
int board_late_init(void)
{
	debug("board_late_init\n");

	clear_rtc_irq_status();

	board_init_adjust_env();

	load_disk_partitions();

	debug("rkimage_prepare_fdt\n");
	rkimage_prepare_fdt();

#ifdef CONFIG_RK_KEY
	debug("key_init\n");
	key_init();
#endif

#ifdef CONFIG_RK_POWER
	debug("pmic_init\n");
	pmic_init(0);
#if defined(CONFIG_POWER_PWM_REGULATOR)
	debug("pwm_regulator_init\n");
	pwm_regulator_init();
#endif
	debug("fg_init\n");
	fg_init(0); /*fuel gauge init*/
#endif /* CONFIG_RK_POWER */

#if defined(CONFIG_RK_DCF)
	dram_freq_init();
#endif

	debug("idb init\n");
	//TODO:set those buffers in a better way, and use malloc?
	rkidb_setup_space(gd->arch.rk_global_buf_addr);

	/* after setup space, get id block data first */
	rkidb_get_idblk_data();

	/* Secure boot check after idb data get */
	SecureBootCheck();

	if (rkidb_get_bootloader_ver() == 0) {
		printf("\n#Boot ver: %s\n", bootloader_ver);
	}

	char tmp_buf[32];
	/* rk sn size 30bytes, zero buff */
	memset(tmp_buf, 0, 32);
	if (rkidb_get_sn(tmp_buf)) {
		setenv("fbt_sn#", tmp_buf);
	}

	debug("fbt preboot\n");
	board_fbt_preboot();
#ifdef CONFIG_MAC_IN_SPI
	board_info_in_spi();
#else
	board_info_config();
#endif
	return 0;
}
#endif
