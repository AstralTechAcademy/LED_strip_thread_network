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
#include <zephyr/syscalls/device.h>
#include <nrfx_gpiote.h>
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

// LED 1
#define GPIO003_PIN NRF_GPIO_PIN_MAP(0, 3) // For PCA10056
#define GPIO004_PIN NRF_GPIO_PIN_MAP(0, 4) // For PCA10056
#define GPIO028_PIN NRF_GPIO_PIN_MAP(0, 27) // For PCA10056
#define GPIO110_PIN NRF_GPIO_PIN_MAP(1, 10) // For PCA10056
#define GPIO111_PIN NRF_GPIO_PIN_MAP(1, 11) // For PCA10056
#define GPIO112_PIN NRF_GPIO_PIN_MAP(1, 12) // For PCA10056
#define GPIO113_PIN NRF_GPIO_PIN_MAP(1, 13) // For PCA10056
#define GPIO114_PIN NRF_GPIO_PIN_MAP(1, 14) // For PCA10056

// LED 2 
#define GPIO101_PIN NRF_GPIO_PIN_MAP(1, 1) // For PCA10056
#define GPIO102_PIN NRF_GPIO_PIN_MAP(1, 2) // For PCA10056
#define GPIO103_PIN NRF_GPIO_PIN_MAP(1, 3) // For PCA10056
#define GPIO104_PIN NRF_GPIO_PIN_MAP(1, 4) // For PCA10056
#define GPIO105_PIN NRF_GPIO_PIN_MAP(1, 5)// For PCA10056
#define GPIO107_PIN NRF_GPIO_PIN_MAP(1, 7) // For PCA10056
#define GPIO108_PIN NRF_GPIO_PIN_MAP(1, 8) // For PCA10056
#define GPIO109_PIN NRF_GPIO_PIN_MAP(1, 9) // For PCA10056

#define MAX_GPIOS 8

static uint8_t hour = 23;
static uint8_t minute = 40;

const uint8_t MASK_0 = 1 << 7;
const uint8_t MASK_1 = 1 << 6;
const uint8_t MASK_2 = 1 << 5;
const uint8_t MASK_3 = 1 << 4;
const uint8_t MASK_4 = 1 << 3;
const uint8_t MASK_5 = 1 << 2;
const uint8_t MASK_6 = 1 << 1;
const uint8_t MASK_7 = 1 << 0;

static bool showHour = false;

typedef struct {
	uint8_t gpios[MAX_GPIOS];
    uint8_t numbers[10];
} sled_t ; 

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

/*extern void controlTime(void *p0, void *p1, void *p2)
{
	gpio_pin_configure_dt(&disp1_0_27, GPIO_OUTPUT_ACTIVE);
	
	while(1)
	{
		gpio_pin_toggle_dt(&disp1_0_27);
	}
}*/

void activate_gpios(uint8_t number, bool pointActive, sled_t* led)
{
    uint8_t gpios_pos = led->numbers[number];
	for(int index = 0 ; index < MAX_GPIOS ; index++)
		nrf_gpio_pin_clear(led->gpios[index]);
	
	if(pointActive)
		nrf_gpio_pin_set(led->gpios[4]);

    if (gpios_pos & MASK_0)
        nrf_gpio_pin_set(led->gpios[0]);
    if (gpios_pos & MASK_1)
        nrf_gpio_pin_set(led->gpios[1]);
    if (gpios_pos & MASK_2)
        nrf_gpio_pin_set(led->gpios[2]);
    if (gpios_pos & MASK_3)
        nrf_gpio_pin_set(led->gpios[3]);
    if (gpios_pos & MASK_4)
        nrf_gpio_pin_set(led->gpios[4]);
    if (gpios_pos & MASK_5)
        nrf_gpio_pin_set(led->gpios[5]);
    if (gpios_pos & MASK_6)
        nrf_gpio_pin_set(led->gpios[6]);
    if (gpios_pos & MASK_7)
        nrf_gpio_pin_set(led->gpios[7]);
}


// Find the first digit
int firstDigit(int n)
{
    // Remove last digit from number
    // till only one digit is left
    while (n >= 10) 
        n /= 10;
    
    // return the first digit
    return n;
}

// Find the last digit
int lastDigit(int n)
{
    // return the last digit
    return (n % 10);
}

void updateTime(void)
{
    while(true)
    {        
        if (minute == 59)
        {
            minute = 0;
            if (hour == 23)
                hour = 0;
            else
                hour++;
        }
        else
            minute++;
		
		k_sleep(K_MSEC(60000));  
    }
}

K_THREAD_DEFINE(blink_led_tid, 1024, blink_led, NULL, NULL, NULL, K_LOWEST_APPLICATION_THREAD_PRIO, 0,0);
K_THREAD_DEFINE(update_ime_tid, 1024, updateTime, NULL, NULL, NULL, K_LOWEST_APPLICATION_THREAD_PRIO, 0,0);
//K_THREAD_DEFINE(controlTime_tid, 1024, controlTime, NULL, NULL, NULL, K_LOWEST_APPLICATION_THREAD_PRIO, 0,0);

int main(void)
{
	LOG_INF(WELLCOME_TEXT);

	LOG_INF("%d", GPIO028_PIN);
	LOG_INF("%d", GPIO101_PIN);
	LOG_INF("%d", GPIO110_PIN);
	LOG_INF("%d", GPIO111_PIN);


    sled_t sLed1 = {
        // GPIO003_PIN, GPIO004_PIN, GPIO028_PIN, GPIO110_PIN, GPIO111_PIN, GPIO112_PIN, GPIO113_PIN, GPIO114_PIN
		.gpios = {GPIO003_PIN, GPIO004_PIN, GPIO028_PIN, GPIO110_PIN, GPIO111_PIN, GPIO112_PIN, GPIO113_PIN, GPIO114_PIN},
        .numbers = {
           231, // 0
           36, // 1
           115, // 2
           118, // 3
           180, // 4
           214, // 5
           215, // 6
           100, // 7
           247, // 8
           244 // 9
        }
    };

	sled_t sLed2 = {
        // GPIO101_PIN, GPIO102_PIN, GPIO103_PIN, GPIO104_PIN, GPIO105_PIN, GPIO107_PIN, GPIO108_PIN, GPIO109_PIN
        .gpios = {GPIO103_PIN, GPIO102_PIN, GPIO101_PIN, GPIO104_PIN, GPIO105_PIN, GPIO107_PIN, GPIO108_PIN, GPIO109_PIN},
        .numbers = {
           231, // 0
           36, // 1
           115, // 2
           118, // 3
           180, // 4
           214, // 5
           215, // 6
           100, // 7
           247, // 8
           244 // 9
        }
    };

	openthread_network_start();

	if (!gpio_is_ready_dt(&led) || !gpio_is_ready_dt(&led1) ) {
		printf("GPIO not ready");
		return 0;
	}
	
//	k_thread_start(blink_led);
//	k_thread_start(controlTime);
	k_thread_start(updateTime);

	// LED 1
	for (uint8_t index = 0; index < MAX_GPIOS; index++)
	{
		nrf_gpio_cfg_output(sLed1.gpios[index]);
		nrf_gpio_cfg_output(sLed2.gpios[index]);
	}

	while (1)
	{

		showHour = !showHour;
		uint8_t digit1, digit2 = 0U;

		if(showHour)
		{
			digit1 = (uint8_t) firstDigit(hour);
			digit2 = (uint8_t) lastDigit(hour);
		}
		else
		{
			digit1 = (uint8_t) firstDigit(minute);
			digit2 = (uint8_t) lastDigit(minute);
		}

		activate_gpios(digit1, showHour, &sLed1);
		activate_gpios(digit2, showHour, &sLed2);

		k_sleep(K_MSEC(5000));
		//nrf_gpio_pin_set(sLed2.gpios[number]);                                                                                                                                       
    }
                                      

#if G_SERVER
	otError error = g_udp_start_server();
	if (error != OT_ERROR_NONE) {
        LOG_ERR("g_udp_start_server error: %d", error);
        return;
    }
#endif

	return 0;
}
