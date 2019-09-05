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
#ifdef CONFIG_OPTEE_CLIENT
#include "../common/rkloader/attestation_key.h"
#endif

#ifdef CONFIG_MAC_IN_SPI
#include <version.h>
#include <spi_flash.h>

struct boardcfg_t {
    unsigned char mac[12];
    unsigned char sn_len;
    unsigned char *sn;
    unsigned char time_len;
    unsigned char *Manufactoring_Time;
    unsigned char info_len;
    unsigned char *guest_info;
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
	int sn_len,time_len,info_len;

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
		info_len = enetaddr[14+sn_len+time_len];
		if(sn_len && (sn_len != 0xff)) {
			enetaddr[13+sn_len] = '\0';
			setenv("boardsn", &enetaddr[13]);
			if(time_len && (time_len != 0xff)) {
				enetaddr[14+sn_len+time_len] = '\0';
				setenv("androidboot.factorytime", &enetaddr[14+sn_len]);
				if(info_len && (info_len != 0xff)) {
					enetaddr[15+sn_len+time_len+info_len] = '\0';
					setenv("androidboot.serialno", &enetaddr[15+sn_len+time_len]);
				}else
					setenv("androidboot.serialno", NULL);
			}else
				setenv("androidboot.factorytime", NULL);
		} else {
			setenv("boardsn", NULL);
			setenv("androidboot.factorytime", NULL);
			setenv("androidboot.serialno", NULL);
		}
	} else
		return -1;

	return 0;
}
#endif 

static int board_version_config(void)
{
	unsigned char version[10];
	unsigned char ver=0;

	memset(version,0,sizeof(version));
	snprintf(version,sizeof(version),"%s",strrchr(PLAIN_VERSION,'V'));
	if(version[0]=='V')
		setenv("swversion",version);
	else
		setenv("swversion",NULL);
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

#ifdef CONFIG_RTC_S35390A
	i2c_set_bus_num(CONFIG_RTC_I2C_BUS);
	i2c_init(100000, CONFIG_S35390A_ADDR);
	/* disable interrupt */
	i2c_write(CONFIG_S35390A_ADDR+1, 0, 0, &buf, 1);
	/* clear pending interrupt, if any */
	i2c_read(CONFIG_S35390A_ADDR, 0, 0, &buf, 1);
#endif
}

static void adv_parse_drm_env(void)
{
	char *p, *e;
	int node,node1,node2;
	int use_dts_screen=0;
	int phandle;

	node = fdt_path_offset(gd->fdt_blob, "/display-timings");
	use_dts_screen = fdtdec_get_int(gd->fdt_blob, node, "use-dts-screen", 0);
	if(!use_dts_screen || getenv("use_env_screen")){
		p = getenv("prmry_screen");
		e = getenv("extend_screen");
		if(!p || !e) {
			phandle = fdt_getprop_u32_default_node(gd->fdt_blob, node, 0, "native-mode", -1);
			if(-1 != phandle) {
				node = fdt_node_offset_by_phandle_node(gd->fdt_blob, node, phandle);
				setenv("prmry_screen",fdt_get_name(gd->fdt_blob, node, NULL));
			} else 
				setenv("prmry_screen","edp-1920x1080");
			setenv("extend_screen","hdmi-1080p");
		}
	} else {
		phandle = fdt_getprop_u32_default_node(gd->fdt_blob, node, 0, "extend-screen", -1);
		if(-1 != phandle)
			node2 = fdt_node_offset_by_phandle_node(gd->fdt_blob, node, phandle);
		else
			node2 = 0;
		phandle = fdt_getprop_u32_default_node(gd->fdt_blob, node, 0, "prmry-screen", -1);
		if(-1 != phandle)
			node1 = fdt_node_offset_by_phandle_node(gd->fdt_blob, node, phandle);
		else
			node1 = 0;
		if((node2 > 0) && (node1 > 0)) {
			setenv("extend_screen",fdt_get_name(gd->fdt_blob, node2, NULL));
			setenv("prmry_screen",fdt_get_name(gd->fdt_blob, node1, NULL));
		} else {
			setenv("extend_screen","hdmi-1080p");
			setenv("prmry_screen","edp-1920x1080");
		}
	}
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
	//debug("fixed_init\n");
	//fixed_regulator_init();
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

#ifdef CONFIG_OPTEE_CLIENT
	load_attestation_key();
#endif

	debug("idb init\n");
	//TODO:set those buffers in a better way, and use malloc?
	rkidb_setup_space(gd->arch.rk_global_buf_addr);

	/* after setup space, get id block data first */
	rkidb_get_idblk_data();

	/* Secure boot check after idb data get */
	SecureBootCheck();

	adv_parse_drm_env();
#ifdef CONFIG_MAC_IN_SPI
	board_info_in_spi();
#endif
	//board_version_config();

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

	return 0;
}
#endif

int board_modify_fdt(void)
{
#if defined(CONFIG_RKCHIP_RK3288) && defined(CONFIG_NORMAL_WORLD)
	int ret;

	/* RK3288W HDMI Revision ID is 0x1A */
	if (readl(RKIO_HDMI_PHYS + 0x4) == 0x1A) {
		ret = fdt_setprop_string((void *)gd->fdt_blob, 0,
					 "compatible", "rockchip,rk3288w");
		if (ret) {
			printf("fdt set compatible failed: %d\n", ret);
			return -1;
		}
	}
#endif
	return 0;
}

#ifdef CONFIG_CMD_NET
/*
 * Initializes on-chip ethernet controllers.
 * to override, implement board_eth_init()
 */
int board_eth_init(bd_t *bis)
{
	__maybe_unused int rc;

	debug("board_eth_init\n");

#ifdef CONFIG_RK_GMAC
	char macaddr[6];
	char ethaddr[20];
	char *env_str = NULL;

	memset(ethaddr, sizeof(ethaddr), 0);
	env_str = getenv("ethaddr");
	if (rkidb_get_mac_address(macaddr) == true) {
		sprintf(ethaddr, "%02X:%02X:%02X:%02X:%02X:%02X",
			macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);

		printf("mac address: %s\n", ethaddr);

		if (env_str == NULL)
			setenv ((char *)"ethaddr", (char *)ethaddr);
		else if (strncmp(env_str, ethaddr, strlen(ethaddr)) != 0)
			setenv ((char *)"ethaddr", (char *)ethaddr);
	} else {
		uint16_t v;

		v = (rand() & 0xfeff) | 0x0200;
		macaddr[0] = (v >> 8) & 0xff;
		macaddr[1] = v & 0xff;
		v = rand();
		macaddr[2] = (v >> 8) & 0xff;
		macaddr[3] = v & 0xff;
		v = rand();
		macaddr[4] = (v >> 8) & 0xff;
		macaddr[5] = v & 0xff;

		sprintf(ethaddr, "%02X:%02X:%02X:%02X:%02X:%02X",
			macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);

		if (env_str == NULL) {
			printf("mac address: %s\n", ethaddr);
			setenv ((char *)"ethaddr", (char *)ethaddr);
		} else {
			printf("mac address: %s\n", env_str);
		}
	}

	rc = rk_gmac_initialize(bis);
	if (rc < 0) {
		printf("rockchip: failed to initialize gmac\n");
		return rc;
	}
#endif /* CONFIG_RK_GMAC */

	return 0;
}
#endif

#ifdef CONFIG_ROCKCHIP_DISPLAY
extern void rockchip_display_fixup(void *blob);
#endif

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t * bd)
{
#ifdef CONFIG_ROCKCHIP_DISPLAY
        rockchip_display_fixup(blob);
#endif
#ifdef CONFIG_ROCKCHIP
#if defined(CONFIG_LCD) || defined(CONFIG_VIDEO)
        u64 start, size;
        int offset;

        if (!gd->uboot_logo)
                return;

        start = gd->fb_base;
        offset = gd->fb_offset;
        if (offset > 0)
                size = CONFIG_RK_LCD_SIZE;
        else
                size = CONFIG_RK_FB_SIZE;

        fdt_update_reserved_memory(blob, "rockchip,fb-logo", start, size);
#endif
#endif
}
#endif
