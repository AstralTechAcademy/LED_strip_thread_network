/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/net/openthread.h>
#include <zephyr/net/net_l2.h>
#include <zephyr/drivers/gpio.h>
#include <openthread/thread.h>
#include <zephyr/drivers/uart.h>
#include "g_udp.h"

#define G_SERVER 1

LOG_MODULE_REGISTER(main, CONFIG_OT_COMMAND_LINE_INTERFACE_LOG_LEVEL);

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
#define LED1_NODE DT_ALIAS(led1)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);

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
	gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
	
	while(1)
	{
		int role = otThreadGetDeviceRole(ot);

		if(role == OT_DEVICE_ROLE_ROUTER)
		{
			gpio_pin_set_dt(&led, 1);
		}
		else if(role == OT_DEVICE_ROLE_CHILD)
		{
			gpio_pin_toggle_dt(&led);
		}
		else
		{
			gpio_pin_set_dt(&led, 0);
		}

		if(role == OT_DEVICE_ROLE_LEADER)
		{
			gpio_pin_toggle_dt(&led1);
		}

		k_sleep(K_MSEC(1000));
	}
}

K_THREAD_DEFINE(blink_led_tid, 1024, blink_led, NULL, NULL, NULL, K_LOWEST_APPLICATION_THREAD_PRIO, 0,0);

int main(void)
{
	LOG_INF(WELLCOME_TEXT);

	openthread_network_start();

	if (!gpio_is_ready_dt(&led) || !gpio_is_ready_dt(&led1)) {
		return 0;
	}
	
	k_thread_start(blink_led);

#if G_SERVER
	otError error = g_udp_start_server();
	if (error != OT_ERROR_NONE) {
        LOG_ERR("g_udp_start_server error: %d", error);
        return;
    }
#endif

	return 0;
}
