/*
 * Configuation settings for the rk312x chip platform.
 *
 * (C) Copyright 2008-2015 Fuzhou Rockchip Electronics Co., Ltd
 * Peter, Software Engineering, <superpeter.cai@gmail.com>.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __RK3288_EBCRB03_CONFIG_H
#define __RK3288_EBCRB03_CONFIG_H

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

/* define CONFIG_EXTRA_ENV_SETTINGS */
#ifdef CONFIG_EXTRA_ENV_SETTINGS
#undef CONFIG_EXTRA_ENV_SETTINGS
#endif
#ifdef CONFIG_ARM64
#define CONFIG_EXTRA_ENV_SETTINGS	"verify=n\0initrd_high=0xffffffffffffffff=n\0switch_hdmi_audio=1\0"
#else
#define CONFIG_EXTRA_ENV_SETTINGS	"verify=n\0initrd_high=0xffffffff=n\0switch_hdmi_audio=1\0"
#endif

/* config for sha256 image check */
#define CONFIG_SECUREBOOT_SHA256

/* undef some module for rk chip */
#if defined(CONFIG_RKCHIP_RK3288)
	#define CONFIG_SECUREBOOT_CRYPTO

	#undef CONFIG_RK_UMS_BOOT_EN
	#undef CONFIG_RK_PL330_DMAC
#endif

/* mod it to enable console commands.	*/
#define CONFIG_BOOTDELAY		3

/* switch debug port to normal uart */
#define CONFIG_SWITCH_DEBUG_PORT_TO_UART
#define CONFIG_DISABLE_CONSOLE
#define DEBUG_SWITCH_GPIO	(GPIO_BANK2 | GPIO_A0)
#define DEBUG_SWITCH_GPIO_ACTIVE 1

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

/*SPI*/
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_WINBOND
#define CONFIG_RK_SPI
#define CONFIG_CMD_SF
#define CONFIG_SF_DEFAULT_BUS   2
#define CONFIG_SF_DEFAULT_CS 0
#define CONFIG_SF_DEFAULT_SPEED 24000000
#define CONFIG_SF_DEFAULT_MODE SPI_MODE_3

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

#define CONFIG_RK32_FB

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

#endif /* __RK3288_EBCRB03_CONFIG_H */
