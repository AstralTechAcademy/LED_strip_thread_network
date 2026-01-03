#ifndef g_UDP
#define g_UDP
#include <openthread/udp.h>
#include <zephyr/logging/log.h>

#define APP_PORT 9090

static struct otInstance *ot;
static otUdpSocket socket;

static void g_msg_handler(void *context,
                           otMessage *message,
                           const otMessageInfo *message_info);
otError g_udp_start_server();
#endif