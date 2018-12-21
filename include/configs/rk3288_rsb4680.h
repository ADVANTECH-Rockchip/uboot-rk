/*
 * Configuation settings for the rk312x chip platform.
 *
 * (C) Copyright 2008-2015 Fuzhou Rockchip Electronics Co., Ltd
 * Peter, Software Engineering, <superpeter.cai@gmail.com>.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __RK3288_RSB4680_CONFIG_H
#define __RK3288_RSB4680_CONFIG_H

/*
 * uboot ram config.
 */
#include <linux/sizes.h>
#define CONFIG_RAM_PHY_START		0x00000000
#define CONFIG_RAM_PHY_SIZE		SZ_128M
#define CONFIG_RAM_PHY_END		(CONFIG_RAM_PHY_START + CONFIG_RAM_PHY_SIZE)

/* reserve iomap memory. */
#define CONFIG_MAX_MEM_ADDR		0xFE000000

/*
 * 		define uboot loader addr.
 * notice: CONFIG_SYS_TEXT_BASE must be an immediate,
 * so if CONFIG_RAM_PHY_START is changed, also update CONFIG_SYS_TEXT_BASE define.
 *
 */
#define CONFIG_SYS_TEXT_BASE    	0x00000000

#define CONFIG_BOARD_EARLY_INIT_F

/*
 * rk plat default configs.
 */
#include <configs/rk_default_config.h>

/* config for sha256 image check */
#define CONFIG_SECUREBOOT_SHA256

/* undef some module for rk chip */
#if defined(CONFIG_RKCHIP_RK3288)
	#define CONFIG_MERGER_TRUSTOS
	#define CONFIG_SECUREBOOT_CRYPTO

	#undef CONFIG_RK_UMS_BOOT_EN
	#undef CONFIG_RK_PL330_DMAC

	#undef CONFIG_CMD_NET
	#undef CONFIG_RK_GMAC
#ifdef CONFIG_NORMAL_WORLD
	#define CONFIG_OPTEE_CLIENT
	#define CONFIG_OPTEE_V1
/*
 * we should enable this macro when we use emmc and secure store
 * data to security partition, not to rpmb.
 */
	/* #define CONFIG_OPTEE_ALWAYS_USE_SECURITY_PARTITION */
#endif
#endif

/* if working normal world, secure efuse can't read,
 * MiniLoader copy RSA KEY to sdram for uboot.
 */
#if defined(CONFIG_SECUREBOOT_CRYPTO)
#if defined(CONFIG_SECOND_LEVEL_BOOTLOADER) && defined(CONFIG_NORMAL_WORLD)
	#define CONFIG_SECURE_RSA_KEY_IN_RAM
	#define CONFIG_SECURE_RSA_KEY_ADDR	(CONFIG_RKNAND_API_ADDR + SZ_2K)
#endif /* CONFIG_NORMAL_WORLD && CONFIG_SECOND_LEVEL_BOOTLOADER */
#endif /* CONFIG_SECUREBOOT_CRYPTO */

/* mod it to enable console commands.	*/
#define CONFIG_BOOTDELAY		0
#define CONFIG_ZERO_BOOTDELAY_CHECK
#define CONFIG_DIRECT_LOGO

/* switch debug port to normal uart */
#define CONFIG_SWITCH_DEBUG_PORT_TO_UART
#define CONFIG_DISABLE_CONSOLE
#define DEBUG_SWITCH_GPIO	(GPIO_BANK2 | GPIO_A0)
#define DEBUG_SWITCH_GPIO_ACTIVE 1

/* reset pmic to reset all system */
#define CONFIG_RESET_PMIC_GPIO	(GPIO_BANK0 | GPIO_B2)

/* HW board ID */
#define CONFIG_DISPLAY_BOARD_ID
#ifdef CONFIG_DISPLAY_BOARD_ID
#define HW_BOARD_ID0	(GPIO_BANK0 | GPIO_B1)
#define HW_BOARD_ID1	(GPIO_BANK0 | GPIO_B0)
#define HW_BOARD_ID2	(GPIO_BANK0 | GPIO_A7)
#endif

/* UBOOT version */
#define CONFIG_IDENT_STRING " V1.000"

/* mac in spi*/
#define CONFIG_CMD_NET
#define CONFIG_ENV_OVERWRITE
#define CONFIG_MAC_IN_SPI
#define CONFIG_SPI_MAC_OFFSET (896*1024)
#define CONFIG_DP83867_PHY_ID

/*SPI*/
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_WINBOND
#define CONFIG_RK_SPI
#define CONFIG_CMD_SF
#define CONFIG_SF_DEFAULT_BUS   2
#define CONFIG_SF_DEFAULT_CS 0
#define CONFIG_SF_DEFAULT_SPEED 24000000
#define CONFIG_SF_DEFAULT_MODE SPI_MODE_3

/* saveenv in spi flash */
#ifdef CONFIG_ENV_IS_IN_RK_STORAGE
#undef CONFIG_ENV_IS_IN_RK_STORAGE
#endif
#ifdef CONFIG_ENV_OFFSET
#undef CONFIG_ENV_OFFSET
#endif
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SPI_BUS	2
#define CONFIG_ENV_SPI_CS	0
#define CONFIG_ENV_SPI_MAX_HZ	24000000
#define CONFIG_ENV_SPI_MODE	SPI_MODE_3
#define CONFIG_ENV_OFFSET		(768 * 1024)
#define CONFIG_ENV_SECT_SIZE	(64 * 1024)

#define CONFIG_RTC_S35390A
#define CONFIG_S35390A_ADDR	0x30
#define CONFIG_RTC_I2C_BUS 0

/* efuse version */
#ifdef CONFIG_RK_EFUSE
#define CONFIG_RKEFUSE_V2
#endif

/* mmc using dma */
#define CONFIG_RK_MMC_DMA
#define CONFIG_RK_MMC_IDMAC	/* internal dmac */
#undef CONFIG_RK_MMC_DDR_MODE	/* mmc using ddr mode */

/* net command support */
#ifdef CONFIG_CMD_NET
#define CONFIG_CMD_PING
#endif

/* Ethernet support */
#ifdef CONFIG_RK_GMAC
#define CONFIG_DESIGNWARE_ETH		/* GMAC can use designware driver */
#define CONFIG_DW_AUTONEG
#define CONFIG_PHY_TI
#define CONFIG_PHY_ADDR		1
#define CONFIG_RGMII			/* RGMII PHY management		*/
#define CONFIG_PHYLIB
#endif

/* more config for rockusb */
#ifdef CONFIG_CMD_ROCKUSB

/* support rockusb timeout check */
#define CONFIG_ROCKUSB_TIMEOUT_CHECK	1

/* rockusb VID/PID should the same as maskrom */
#define CONFIG_USBD_VENDORID			0x2207
#if defined(CONFIG_RKCHIP_RK3288)
	#define CONFIG_USBD_PRODUCTID_ROCKUSB	0x320A
#else
	#error "PLS config rk chip for rockusb PID!"
#endif

#endif /* CONFIG_CMD_ROCKUSB */


/* more config for fastboot */
#ifdef CONFIG_CMD_FASTBOOT

#define CONFIG_USBD_PRODUCTID_FASTBOOT	0x0006
#define CONFIG_USBD_MANUFACTURER	"Rockchip"
#define CONFIG_USBD_PRODUCT_NAME	"rk32xx"

#define FASTBOOT_PRODUCT_NAME		"fastboot" /* Fastboot product name */

#define CONFIG_FASTBOOT_LOG
#define CONFIG_FASTBOOT_LOG_SIZE	(SZ_2M)

#endif /* CONFIG_CMD_FASTBOOT */


#ifdef CONFIG_RK_UMS_BOOT_EN
/*
 * USB Host support, default no using
 * Please first select USB host controller if you want to use UMS Boot
 * Up to one USB host controller could be selected to enable for booting
 * from USB Mass Storage device.
 *
 * PLS define a host controler from:
 *	RKUSB_UMS_BOOT_FROM_DWC2_OTG
 *	RKUSB_UMS_BOOT_FROM_DWC2_HOST
 *	RKUSB_UMS_BOOT_FROM_EHCI_HOST1
 *
 * First define the host controller here
 */
#undef RKUSB_UMS_BOOT_FROM_DWC2_OTG
#undef RKUSB_UMS_BOOT_FROM_DWC2_HOST
#undef RKUSB_UMS_BOOT_FROM_EHCI_HOST1


/* Check UMS Boot Host define */
#define RKUSB_UMS_BOOT_CNT (defined(RKUSB_UMS_BOOT_FROM_DWC2_OTG) + \
			    defined(RKUSB_UMS_BOOT_FROM_DWC2_HOST) + \
			    defined(RKUSB_UMS_BOOT_FROM_EHCI_HOST1))

#if (RKUSB_UMS_BOOT_CNT == 0)
	#error "PLS Select a USB host controller!"
#elif (RKUSB_UMS_BOOT_CNT > 1)
	#error "Only one USB host controller can be selected!"
#endif


/*
 * USB Host support, default no using
 * please first check plat if you want to using usb host
 */
#if defined(RKUSB_UMS_BOOT_FROM_EHCI_HOST1)
	#define CONFIG_USB_EHCI
	#define CONFIG_USB_EHCI_RK
#elif defined(RKUSB_UMS_BOOT_FROM_DWC2_HOST) || defined(RKUSB_UMS_BOOT_FROM_DWC2_OTG)
	#define CONFIG_USB_DWC_HCD
#endif


/* enable usb config for usb host */
#define CONFIG_CMD_USB
#define CONFIG_USB_STORAGE
#define CONFIG_PARTITIONS
#endif /* CONFIG_RK_UMS_BOOT_EN */

/* memery test cmd */
#define CONFIG_CMD_MEMORY

/* more config for display */
#ifdef CONFIG_LCD
/*#define CONFIG_ROCKCHIP_DISPLAY*/
#define CONFIG_OF_BOARD_SETUP

#ifdef CONFIG_ROCKCHIP_DISPLAY
#define CONFIG_ROCKCHIP_LVDS
#define CONFIG_ROCKCHIP_VOP
#define CONFIG_ROCKCHIP_MIPI_DSI
#define CONFIG_ROCKCHIP_DW_MIPI_DSI
#define CONFIG_ROCKCHIP_ANALOGIX_DP
#define CONFIG_ROCKCHIP_PANEL
#define CONFIG_I2C_EDID
#endif

/*#define CONFIG_RK32_FB*/

#ifdef CONFIG_RK_HDMI
#define CONFIG_RK_HDMIV2
#endif

#ifdef CONFIG_RK_TVE
#define CONFIG_RK1000_TVE
#undef CONFIG_GM7122_TVE
#endif

#define CONFIG_RK32_DSI

#undef CONFIG_UBOOT_CHARGE

#else

#undef CONFIG_RK_FB
#undef CONFIG_RK_PWM_BL
#undef CONFIG_RK_HDMI
#undef CONFIG_RK_TVE
#undef CONFIG_CMD_BMP
#undef CONFIG_UBOOT_CHARGE

#endif /* CONFIG_LCD */


/* more config for charge */
#ifdef CONFIG_UBOOT_CHARGE

#define CONFIG_CMD_CHARGE_ANIM
#define CONFIG_CHARGE_DEEP_SLEEP

#endif /* CONFIG_UBOOT_CHARGE */

/* more config for power */
#ifdef CONFIG_RK_POWER

#ifdef CONFIG_RK_KEY
#undef CONFIG_RK_KEY
#endif

#define CONFIG_POWER
#define CONFIG_POWER_I2C

#define CONFIG_POWER_PMIC
/* if box product, undefine fg and battery */
#ifndef CONFIG_PRODUCT_BOX
#define CONFIG_POWER_FG
#define CONFIG_POWER_BAT
#endif /* CONFIG_PRODUCT_BOX */

#define CONFIG_SCREEN_ON_VOL_THRESD	0
#define CONFIG_SYSTEM_ON_VOL_THRESD	0

/******** pwm regulator driver ********/
/*#define CONFIG_POWER_PWM_REGULATOR*/

/******** pmic driver ********/
#ifdef CONFIG_POWER_PMIC
#undef CONFIG_POWER_RK_SAMPLE
#define CONFIG_POWER_RK808
#endif /* CONFIG_POWER_PMIC */

/******** charger driver ********/
#ifdef CONFIG_POWER_FG
#undef CONFIG_POWER_FG_CW201X
#endif /* CONFIG_POWER_FG */

/******** battery driver ********/
#ifdef CONFIG_POWER_BAT
#undef CONFIG_BATTERY_RK_SAMPLE
#undef CONFIG_BATTERY_BQ27541
#undef CONFIG_BATTERY_RICOH619
#endif /* CONFIG_POWER_BAT */

#endif /* CONFIG_RK_POWER */

#endif /* __RK3288_RSB4680_CONFIG_H */
