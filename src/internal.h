#ifndef INTERNAL_H_
#define INTERNAL_H_

#include <uv.h>
#include <uv-private/ngx-queue.h>

#include "muv.h"

#define MUV_REQ_TYPE_MAP(XX)         \
  XX(TCP_CONNECT, tcp_connect)       \
  XX(TCP_CONNECT6, tcp_connect6)     \
  XX(LISTEN, listen)                 \
  XX(ACCEPT, accept)                 \
  XX(READ_START, read_start)         \
  XX(READ_STOP, read_stop)           \
  XX(WRITE, write)                   \
  XX(SHUTDOWN, shutdown)             \
  XX(CLOSE, close)                   \

typedef struct muv_req_s muv_req_t;

typedef struct {
  ngx_queue_t node;
  muv_req_t*  req;
} node_t;

typedef enum {
#define XX(uc, lc) MUV_##uc,
  MUV_REQ_TYPE_MAP(XX)
#undef XX
  MUV_REQ_TYPE_MAX
} muv_req_type;

#define MUV_REQ_PRIVATE_FIELDS \
  muv_req_type type;           \

struct muv_req_s {
  MUV_REQ_PRIVATE_FIELDS
};

typedef struct {
  MUV_REQ_PRIVATE_FIELDS
  uv_connect_t* req;
  uv_tcp_t* handle;
  struct sockaddr_in address;
  uv_connect_cb cb;
} muv_tcp_connect_t;

typedef struct {
  MUV_REQ_PRIVATE_FIELDS
  uv_connect_t* req;
  uv_tcp_t* handle;
  struct sockaddr_in6 address;
  uv_connect_cb cb;
} muv_tcp_connect6_t;

typedef struct {
  MUV_REQ_PRIVATE_FIELDS
  uv_stream_t* stream;
  int backlog;
  uv_connection_cb cb;
} muv_listen_t;

typedef struct {
  MUV_REQ_PRIVATE_FIELDS
  uv_stream_t* server;
  uv_stream_t* client;
} muv_accept_t;

typedef struct {
  MUV_REQ_PRIVATE_FIELDS
  uv_stream_t* handle;
  uv_alloc_cb alloc_cb;
  uv_read_cb read_cb;
} muv_read_start_t;

typedef struct {
  MUV_REQ_PRIVATE_FIELDS
  uv_stream_t* handle;
} muv_read_stop_t;

typedef struct {
  MUV_REQ_PRIVATE_FIELDS
  uv_write_t* req;
  uv_stream_t* handle;
  uv_buf_t* bufs;
  int bufcnt;
  uv_write_cb cb;
} muv_write_t;

typedef struct {
  MUV_REQ_PRIVATE_FIELDS
  uv_shutdown_t* req;
  uv_stream_t* handle;
  uv_shutdown_cb cb;
} muv_shutdown_t;

typedef struct {
  MUV_REQ_PRIVATE_FIELDS
  uv_handle_t* handle;
  uv_close_cb cb;
} muv_close_t;

/* core */
void muv_req_queue_push(muv_t* mid, muv_req_t* req);
int muv_req_queue_flush(muv_t* mid);
void muv__req_queue_flush(muv_t* mid);
void muv__async_cb(uv_async_t* async, int status);
void muv__thread_cb(void* arg);

/* handle */
void muv__close(muv_t* mid, muv_close_t* req);

/* stream */
void muv__listen(muv_t* mid, muv_listen_t* req);
void muv__accept(muv_t* mid, muv_accept_t* req);
void muv__read_start(muv_t* mid, muv_read_start_t* req);
void muv__read_stop(muv_t* mid, muv_read_stop_t* req);
void muv__write(muv_t* mid, muv_write_t* req);
void muv__shutdown(muv_t* mid, muv_shutdown_t* req);

/* tcp */
void muv__tcp_connect(muv_t* mid, muv_tcp_connect_t* req);
void muv__tcp_connect6(muv_t* mid, muv_tcp_connect6_t* req);

#endif  /* INTERNAL_H_ */
