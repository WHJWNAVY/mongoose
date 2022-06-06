#include "mongoose.h"

static int tcn = 0;
static char tbuf[512] = {0};

static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
  MG_DEBUG(("%p got event: %d %p %p", c, ev, ev_data, fn_data));
  /*if (ev == MG_EV_OPEN) {
    c->is_hexdumping = 1;
  } else */
  if (ev == MG_EV_READ) {
    MG_INFO(("Recved [%s]\n", c->recv.buf));
    mg_iobuf_del(&c->recv, 0, c->recv.len);
  }
}


static void tfn(void *param) {
  struct mg_connection *c = param;
  if (c == NULL) return;
  memset(tbuf, 0 ,sizeof(tbuf));
  mg_snprintf(tbuf, (sizeof(tbuf) - 1), "MSG [%d]", tcn++);
  MG_INFO(("Sending [%s]", tbuf));
  mg_printf(c, "%s", tbuf);
}

int main(void) {
  struct mg_mgr mgr;
  static struct mg_connection *c;
  mg_mgr_init(&mgr);
  c = mg_connect(&mgr, "tcp://0.0.0.0:1234", fn, NULL);
  mg_timer_add(&mgr, 2000, MG_TIMER_REPEAT | MG_TIMER_RUN_NOW, tfn, c);
  while (true) mg_mgr_poll(&mgr, 200);
  mg_mgr_free(&mgr);
  return 0;
}
