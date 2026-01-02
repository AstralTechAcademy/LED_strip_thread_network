/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/openthread.h>
#include <zephyr/net/net_l2.h>
#include <zephyr/drivers/gpio.h>
#include <openthread/thread.h>

#if defined(CONFIG_CLI_SAMPLE_MULTIPROTOCOL)
#include "ble.h"
#endif

#if defined(CONFIG_CLI_SAMPLE_LOW_POWER)
#include "low_power.h"
#endif

#include <zephyr/drivers/uart.h>

LOG_MODULE_REGISTER(cli_sample, CONFIG_OT_COMMAND_LINE_INTERFACE_LOG_LEVEL);


#define WELLCOME_TEXT \
	"\n\r"\
	"\n\r"\
	"OpenThread Command Line Interface is now running.\n\r" \
	"Use the 'ot' keyword to invoke OpenThread commands e.g. " \
	"'ot thread start.'\n\r" \
	"For the full commands list refer to the OpenThread CLI " \
	"documentation at:\n\r" \
	"https://github.com/openthread/openthread/blob/master/src/cli/README.md\n\r"

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

struct otInstance *ot;

void openthread_network_start()
{
	openthread_init();

	int net_res = openthread_run();

	ot = openthread_get_default_instance();

	if(net_res != OT_ERROR_NONE)
		LOG_ERR("Openthread starting error [%d]", net_res);
		return;
	
	LOG_INF("Openthread network %s started ", otThreadGetNetworkName(ot));
	return;
}

extern void blink_led(void *p0, void *p1, void *p2)
{
	int toggle = 1;
	gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);


	while(1)
	{
		int role = otThreadGetDeviceRole(ot);
		LOG_INF("Device role %s", otThreadDeviceRoleToString(role));

		if(role == OT_DEVICE_ROLE_ROUTER)
		{
			toggle = 1;
		}
		else if(role == OT_DEVICE_ROLE_CHILD)
		{
			toggle = !toggle;
		}
		else
		{
			toggle = 0;
		}

		gpio_pin_set_dt(&led, toggle);
		k_sleep(K_MSEC(1000));
	}
}

K_THREAD_DEFINE(blink_led_tid, 1024, blink_led, NULL, NULL, NULL, K_LOWEST_APPLICATION_THREAD_PRIO, 0,0);

int main(void)
{
	LOG_INF(WELLCOME_TEXT);

	openthread_network_start();

	if (!gpio_is_ready_dt(&led)) {
		return 0;
	}

	k_thread_start(blink_led);
	
	return 0;
}
