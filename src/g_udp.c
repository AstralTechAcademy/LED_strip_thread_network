#include "g_udp.h"
#include "g_node.h"
#include <openthread/thread.h>
#include <zephyr/net/openthread.h>

LOG_MODULE_REGISTER(g_udp, CONFIG_OT_COMMAND_LINE_INTERFACE_LOG_LEVEL);

typedef struct node {
    g_node_t value;
    struct node *next;
    uint8_t size;
} node_t;

node_t* nodes = NULL;

node_t* addNode(node_t *head, g_node_t node)
{
    node_t *n = malloc(sizeof *n);
    if (!n) return head;
    n->value = node;
    n->next = head;
    return n;
}

void freeNodes(node_t *head) {
    while (head) {
        node_t *next = head->next;
        free(head);
        head = next;
    }
}

void printAddress(const otIp6Address*const addr)
{
	LOG_INF("%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x", 
            addr->mFields.m8[0],
            addr->mFields.m8[1],
            addr->mFields.m8[2],
            addr->mFields.m8[3],
            addr->mFields.m8[4],
            addr->mFields.m8[5],
            addr->mFields.m8[6],
            addr->mFields.m8[7],
            addr->mFields.m8[8],
            addr->mFields.m8[9],
            addr->mFields.m8[10],
            addr->mFields.m8[11],
            addr->mFields.m8[12],
            addr->mFields.m8[13],
            addr->mFields.m8[14],
            addr->mFields.m8[15]);
}

/* Callback when a message is received in the socket */
void g_control_handler(void *context,
                           otMessage *message,
                           const otMessageInfo *message_info)
{
    uint16_t length = otMessageGetLength(message);
    char buffer[length];
    memset(buffer, 0, sizeof(buffer));
    otMessageRead(message, 0, &buffer, length);
    
    if(buffer[0] == 0)
    {   
        uint8_t nodeId = atoi(buffer[1]);
        otIp6Address addr = message_info->mPeerAddr;
        g_node_t node = {1, addr};
        nodes = addNode(nodes, node);
    }


    return;
}

/* Callback when a message is received in the socket */
void g_node_handler(void *context,
                           otMessage *message,
                           const otMessageInfo *message_info)
{
    otIp6Address addr = message_info->mPeerAddr;
    printAddress(&message_info->mPeerAddr);
    g_node_t node = {1, addr};
    nodes = addNode(nodes, node);

    printAddress(&nodes->value.address);
    uint16_t length = otMessageGetLength(message);
    char buffer[length+1];
    memset(buffer, 0, sizeof(buffer));
    otMessageRead(message, 0, &buffer, length);
    buffer[length+1] = '\0'; 

    LOG_INF("Message type: %c", buffer[0]);

    if(buffer[0] == '0')
    {   
        int nodeId = atoi(&buffer[1]);
        otIp6Address addr = message_info->mPeerAddr;
        g_node_t node = {nodeId, addr};
        nodes = addNode(nodes, node);
        LOG_INF("NodeID: %d", nodeId);
    }

	return;
}

otError g_udp_start_server()
{
    otError error;
    otSockAddr addr = {0};

    addr.mPort = APP_PORT;

    ot = openthread_get_default_instance();

    error = otUdpOpen(ot, &socket, g_node_handler, ot);
    error = otUdpBind(ot, &socket, &addr, OT_NETIF_THREAD);
    return error;
}