#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <netdb.h>

#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "mongoose.h"

#define TCP_ECHO_IFNAME ("eth0")
#define TCP_ECHO_PORT (1234)

struct mg_mgr mgr = {0};

static int32_t ifaddr_get(const char *ifname, char *ipaddr)
{
    int sck = -1;
    int32_t ecode = 0;
    struct ifreq ifr = {0};
    struct sockaddr_in *ip = NULL;

    if (!ifname || !ipaddr)
    {
        ecode = -1;
        goto err;
    }

    if (0 == strlen(ifname))
    {
        ecode = -1;
        goto err;
    }

    if ((sck = socket(AF_INET, SOCK_DGRAM, 0)) <= 0)
    {
        ecode = -2;
        goto err;
    }

    strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));

    if (0 != ioctl(sck, SIOCGIFADDR, &ifr))
    {
        ecode = -2;
        goto err;
    }

    ip = (struct sockaddr_in *)&ifr.ifr_addr;
    if (inet_ntop(AF_INET, &(ip->sin_addr.s_addr), ipaddr, INET_ADDRSTRLEN) == NULL)
    {
        ecode = -2;
        goto err;
    }

    ecode = 0;
err:
    if (sck > 0)
    {
        close(sck);
    }
    return ecode;
}

static int tcn = 0;
static char tbuf[512] = {0};

static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
    MG_DEBUG(("%p got event: %d %p %p", c, ev, ev_data, fn_data));
    /* if (ev == MG_EV_OPEN) {
      c->is_hexdumping = 1;
    } else */
    if (ev == MG_EV_READ)
    {
        MG_INFO(("Recved [%s]\n", c->recv.buf));
        mg_iobuf_del(&c->recv, 0, c->recv.len);
    }
}

static void tfn(void *param)
{
    struct mg_connection *c = param;
    if (c == NULL)
        return;
    memset(tbuf, 0, sizeof(tbuf));
    mg_snprintf(tbuf, (sizeof(tbuf) - 1), "MSG %d FROM %d", tcn++, getpid());
    MG_INFO(("Sending [%s]", tbuf));
    mg_printf(c, "%s", tbuf);
}

static void stop(int signo)
{
    MG_INFO(("EXIT"));
    mg_mgr_free(&mgr); // Cleanup
    exit(0);
}

int main(void)
{
    static struct mg_connection *c;
    char ipaddr[INET_ADDRSTRLEN + 1] = {0};
    char neturl[512] = {0};

    signal(SIGINT, stop);

    if (ifaddr_get(TCP_ECHO_IFNAME, ipaddr) != 0)
    {
        MG_ERROR(("get ipaddr for [%s] failed!\n", TCP_ECHO_IFNAME));
        return -1;
    }
    sprintf(neturl, "tcp://%s:%d", ipaddr, TCP_ECHO_PORT);

    // mg_log_set("4");
    mg_mgr_init(&mgr);
    if ((c = mg_connect(&mgr, neturl, fn, NULL)) == NULL)
    {
        MG_ERROR(("connect to [%s] failed!\n", neturl));
        return -1;
    }
    MG_INFO(("connect to [%s] success!\n", neturl));
    mg_timer_add(&mgr, 2000, MG_TIMER_REPEAT | MG_TIMER_RUN_NOW, tfn, c);
    while (true)
        mg_mgr_poll(&mgr, 200);
    mg_mgr_free(&mgr);
    return 0;
}
