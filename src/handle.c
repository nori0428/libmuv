#include <stdlib.h>

#include <uv.h>

#include "muv.h"
#include "internal.h"

int muv_close(muv_t* mid, uv_handle_t* handle, uv_close_cb close_cb) {
  muv_close_t* close_req;

  close_req = (muv_close_t*) malloc(sizeof(muv_close_t));
  close_req->type = MUV_CLOSE;
  close_req->handle = handle;
  close_req->cb = close_cb;

  muv_req_queue_push(mid, (muv_req_t*) close_req);
  return muv_req_queue_flush(mid);
}

void muv__close(muv_t* mid, muv_close_t* req) {
  uv_close(req->handle, req->cb);
}
