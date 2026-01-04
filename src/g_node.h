#ifndef G_NODE
#define G_NODE

#include <openthread/thread.h>
#include <zephyr/net/openthread.h>

typedef struct g_node
{  
    uint8_t id;
    otIp6Address address;

} g_node_t;

#endif