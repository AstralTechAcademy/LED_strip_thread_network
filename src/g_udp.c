#include "g_udp.h"
#include <openthread/thread.h>
#include <zephyr/net/openthread.h>

LOG_MODULE_REGISTER(g_udp, CONFIG_OT_COMMAND_LINE_INTERFACE_LOG_LEVEL);

/* Callback when a message is received in the socket */
void g_msg_handler(void *context,
                           otMessage *message,
                           const otMessageInfo *message_info)
{
	LOG_INF("HELLO UDP");
	return;
}

otError g_udp_start_server()
{
    otError error;
    otSockAddr addr = {0};

    addr.mPort = APP_PORT;

    ot = openthread_get_default_instance();

    error = otUdpOpen(ot, &socket, g_msg_handler, ot);
    error = otUdpBind(ot, &socket, &addr, OT_NETIF_THREAD);
    return error;
}