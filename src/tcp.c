#include <stdlib.h>

#include <uv.h>

#include "muv.h"
#include "internal.h"

int muv_tcp_init(muv_t* mid, uv_loop_t* loop, uv_tcp_t* handle) {
  int r;

  uv_mutex_lock(&(mid->mutex));
  r = uv_tcp_init(mid->loop, handle);
  uv_mutex_unlock(&(mid->mutex));
  if (r) {
    if (mid->error_cb) {
      uv_err_t err = uv_last_error(mid->loop);
      mid->error_cb(mid, err);
    }
    return r;
  }

  return 0;
}
int muv_tcp_connect(muv_t* mid, uv_connect_t* req, uv_tcp_t* handle,
    struct sockaddr_in address, uv_connect_cb cb) {
  muv_tcp_connect_t* tcp_connect_req;

  tcp_connect_req = (muv_tcp_connect_t*) malloc(sizeof(muv_tcp_connect_t));
  tcp_connect_req->type = MUV_TCP_CONNECT;
  tcp_connect_req->req = req;
  tcp_connect_req->handle = handle;
  tcp_connect_req->address = address;
  tcp_connect_req->cb = cb;

  muv_req_queue_push(mid, (muv_req_t*) tcp_connect_req);
  return muv_req_queue_flush(mid);
}
int muv_tcp_connect6(muv_t* mid, uv_connect_t* req, uv_tcp_t* handle,
    struct sockaddr_in6 address, uv_connect_cb cb) {
  muv_tcp_connect6_t* tcp_connect6_req;

  tcp_connect6_req = (muv_tcp_connect6_t*) malloc(sizeof(muv_tcp_connect6_t));
  tcp_connect6_req->type = MUV_TCP_CONNECT6;
  tcp_connect6_req->req = req;
  tcp_connect6_req->handle = handle;
  tcp_connect6_req->address = address;
  tcp_connect6_req->cb = cb;

  muv_req_queue_push(mid, (muv_req_t*) tcp_connect6_req);
  return muv_req_queue_flush(mid);
}

int muv_tcp_bind(muv_t* mid, uv_tcp_t* handle, struct sockaddr_in address) {
  int r;

  r = uv_tcp_bind(handle, address);
  if (r) {
    if (mid->error_cb) {
      uv_err_t err = uv_last_error(mid->loop);
      mid->error_cb(mid, err);
    }
    return r;
  }

  return 0;
}
int muv_tcp_bind6(muv_t* mid, uv_tcp_t* handle, struct sockaddr_in6 address) {
  int r;

  r = uv_tcp_bind6(handle, address);
  if (r) {
    if (mid->error_cb) {
      uv_err_t err = uv_last_error(mid->loop);
      mid->error_cb(mid, err);
    }
    return r;
  }

  return 0;
}

