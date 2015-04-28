/* Copyright 2015 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
/* Honeybuns board configuration */

#include "adc.h"
#include "adc_chip.h"
#include "common.h"
#include "console.h"
#include "ec_version.h"
#include "gpio.h"
#include "hooks.h"
#include "i2c.h"
#include "registers.h"
#include "task.h"
#include "timer.h"
#include "usb.h"
#include "usb_bb.h"
#include "usb_pd.h"
#include "util.h"

static volatile uint64_t hpd_prev_ts;
static volatile int hpd_prev_level;

void vbus_event(enum gpio_signal signal)
{
	ccprintf("VBUS!\n");
}


#include "gpio_list.h"

/**
 * Hotplug detect deferred task
 *
 * Called after level change on hpd GPIO to evaluate (and debounce) what event
 * has occurred.  There are 3 events that occur on HPD:
 *    1. low  : downstream display sink is deattached
 *    2. high : downstream display sink is attached
 *    3. irq  : downstream display sink signalling an interrupt.
 *
 * The debounce times for these various events are:
 *   HPD_USTREAM_DEBOUNCE_LVL : min pulse width of level value.
 *   HPD_USTREAM_DEBOUNCE_IRQ : min pulse width of IRQ low pulse.
 *
 * lvl(n-2) lvl(n-1)  lvl   prev_delta  now_delta event
 * ----------------------------------------------------
 * 1        0         1     <IRQ        n/a       low glitch (ignore)
 * 1        0         1     >IRQ        <LVL      irq
 * x        0         1     n/a         >LVL      high
 * 0        1         0     <LVL        n/a       high glitch (ignore)
 * x        1         0     n/a         >LVL      low
 */

void hpd_irq_deferred(void)
{
	pd_send_hpd(0, hpd_irq);
}
DECLARE_DEFERRED(hpd_irq_deferred);

void hpd_lvl_deferred(void)
{
	int level = gpio_get_level(GPIO_DP_HPD);

	if (level != hpd_prev_level)
		/* It's a glitch while in deferred or canceled action */
		return;

	pd_send_hpd(0, (level) ? hpd_high : hpd_low);
}
DECLARE_DEFERRED(hpd_lvl_deferred);

void hpd_event(enum gpio_signal signal)
{
	timestamp_t now = get_time();
	int level = gpio_get_level(signal);
	uint64_t cur_delta = now.val - hpd_prev_ts;

	/* store current time */
	hpd_prev_ts = now.val;

	/* All previous hpd level events need to be re-triggered */
	hook_call_deferred(hpd_lvl_deferred, -1);

	/* It's a glitch.  Previous time moves but level is the same. */
	if (cur_delta < HPD_USTREAM_DEBOUNCE_IRQ)
		return;

	if ((!hpd_prev_level && level) &&
	    (cur_delta < HPD_USTREAM_DEBOUNCE_LVL))
		/* It's an irq */
		hook_call_deferred(hpd_irq_deferred, 0);
	else if (cur_delta >= HPD_USTREAM_DEBOUNCE_LVL)
		hook_call_deferred(hpd_lvl_deferred, HPD_USTREAM_DEBOUNCE_LVL);

	hpd_prev_level = level;
}

static void honeybuns_test_led_update(void)
{
	static int toggle_count;
	toggle_count++;

	gpio_set_level(GPIO_TP6, toggle_count&1);
}
DECLARE_HOOK(HOOK_TICK, honeybuns_test_led_update, HOOK_PRIO_DEFAULT);

/* Initialize board. */
void board_config_pre_init(void)
{
	/* enable SYSCFG clock */
	STM32_RCC_APB2ENR |= 1 << 0;

	/*
	 * the DMA mapping is :
	 *  Chan 2 : TIM1_CH1  (C0 RX)
	 *  Chan 3 : SPI1_TX   (C0 TX)
	 *  Chan 4 : USART1_TX
	 *  Chan 5 : USART1_RX
	 *  Chan 6 :
	 *  Chan 7 :
	 */
	/* Remap USART DMA to match the USART driver */
	STM32_SYSCFG_CFGR1 |= (1 << 9) | (1 << 10);
}

/* Initialize board. */
static void board_init(void)
{
	timestamp_t now;

	now = get_time();
	hpd_prev_level = gpio_get_level(GPIO_DP_HPD);
	hpd_prev_ts = now.val;
	gpio_enable_interrupt(GPIO_DP_HPD);
}

DECLARE_HOOK(HOOK_INIT, board_init, HOOK_PRIO_DEFAULT);

/* ADC channels */
const struct adc_t adc_channels[] = {
	/* USB PD CC lines sensing. Converted to mV (3300mV/4096). */
	[ADC_CH_CC1_PD] = {"CC1_PD", 3300, 4096, 0, STM32_AIN(1)},
	/* VBUS sense via 100k/8.8k voltage divder 3.3V -> 40.8V */
	[ADC_CH_VIN_DIV_P] = {"VIN_DIV_P", 40800, 4096, 0, STM32_AIN(5)},
	[ADC_CH_VIN_DIV_N] = {"VIN_DIV_N", 40800, 4096, 0, STM32_AIN(6)},
};
BUILD_ASSERT(ARRAY_SIZE(adc_channels) == ADC_CH_COUNT);

/* I2C ports */
const struct i2c_port_t i2c_ports[] = {
	{"master", I2C_PORT_MASTER, 100,
		GPIO_MASTER_I2C_SCL, GPIO_MASTER_I2C_SDA},
};
const unsigned int i2c_ports_used = ARRAY_SIZE(i2c_ports);

const void * const usb_strings[] = {
	[USB_STR_DESC] = usb_string_desc,
	[USB_STR_VENDOR] = USB_STRING_DESC("Google Inc."),
	[USB_STR_PRODUCT] = USB_STRING_DESC("Honeybuns"),
	[USB_STR_VERSION] = USB_STRING_DESC(CROS_EC_VERSION32),
	[USB_STR_BB_URL] = USB_STRING_DESC(USB_GOOGLE_TYPEC_URL),
};
BUILD_ASSERT(ARRAY_SIZE(usb_strings) == USB_STR_COUNT);

void board_set_usb_mux(int port, enum typec_mux mux,
		       enum usb_switch usb, int polarity)
{
	if (mux == TYPEC_MUX_NONE) {
		/* put the mux in the high impedance state */
		gpio_set_level(GPIO_SS_MUX_OE_L, 1);
		/* Disable display hardware */
		gpio_set_level(GPIO_BRIDGE_RESET_L, 0);
		gpio_set_level(GPIO_SPLITTER_RESET_L, 0);
		/* Put the USB hub under reset */
		hx3_enable(0);
		return;
	}

	/* Trigger USB Hub configuration */
	hx3_enable(1);

	if ((mux == TYPEC_MUX_DOCK) || (mux == TYPEC_MUX_USB)) {
		/* Low selects USB Dock */
		gpio_set_level(GPIO_SS_MUX_SEL, 0);
	} else if (mux == TYPEC_MUX_DP) {
		/* high selects display port */
		gpio_set_level(GPIO_SS_MUX_SEL, 1);
	}

	/* clear OE line to make mux active */
	gpio_set_level(GPIO_SS_MUX_OE_L, 0);

	if (mux != TYPEC_MUX_USB) {
		/* Enable display hardware */
		gpio_set_level(GPIO_BRIDGE_RESET_L, 1);
		gpio_set_level(GPIO_SPLITTER_RESET_L, 1);
	}
}

int board_get_usb_mux(int port, const char **dp_str, const char **usb_str)
{
	int oe_disabled = gpio_get_level(GPIO_SS_MUX_OE_L);
	int dp_4lanes = gpio_get_level(GPIO_SS_MUX_SEL);

	if (oe_disabled) {
		*usb_str = NULL;
		*dp_str = NULL;
		return 0;
	}

	if (dp_4lanes) {
		*dp_str = "DP_4LANE";
		*usb_str = NULL;
	} else {
		*dp_str = "DP_2LANE";
		*usb_str = "DOCK";
	}
	return 1;
}

/**
 * USB configuration
 * Any type-C device with alternate mode capabilities must have the following
 * set of descriptors.
 *
 * 1. Standard Device
 * 2. BOS
 *    2a. Container ID
 *    2b. Billboard Caps
 */
struct my_bos {
	struct usb_bos_hdr_descriptor bos;
	struct usb_contid_caps_descriptor contid_caps;
	struct usb_bb_caps_base_descriptor bb_caps;
	struct usb_bb_caps_svid_descriptor bb_caps_svids[1];
};

static struct my_bos bos_desc = {
	.bos = {
		.bLength = USB_DT_BOS_SIZE,
		.bDescriptorType = USB_DT_BOS,
		.wTotalLength = (USB_DT_BOS_SIZE + USB_DT_CONTID_SIZE +
				 USB_BB_CAPS_BASE_SIZE +
				 USB_BB_CAPS_SVID_SIZE * 1),
		.bNumDeviceCaps = 2,  /* contid + bb_caps */
	},
	.contid_caps =	{
		.bLength = USB_DT_CONTID_SIZE,
		.bDescriptorType = USB_DT_DEVICE_CAPABILITY,
		.bDevCapabilityType = USB_DC_DTYPE_CONTID,
		.bReserved = 0,
		.ContainerID = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	},
	.bb_caps = {
		.bLength = (USB_BB_CAPS_BASE_SIZE + USB_BB_CAPS_SVID_SIZE * 1),
		.bDescriptorType = USB_DT_DEVICE_CAPABILITY,
		.bDevCapabilityType = USB_DC_DTYPE_BILLBOARD,
		.iAdditionalInfoURL = USB_STR_BB_URL,
		.bNumberOfAlternateModes = 1,
		.bPreferredAlternateMode = 1,
		.VconnPower = 0,
		.bmConfigured = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
				},
		.bReserved = 0,
	},
	.bb_caps_svids = {
		{
			.wSVID = USB_SID_DISPLAYPORT,
			.bAlternateMode = 1,
			.iAlternateModeString = USB_STR_BB_URL,
		},
	},
};

const struct bos_context bos_ctx = {
	.descp = (void *)&bos_desc,
	.size = sizeof(struct my_bos),
};

