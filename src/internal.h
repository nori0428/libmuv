#ifndef INTERNAL_H_
#define INTERNAL_H_

#include <stdio.h>

#include <uv.h>

#include "muv.h"

typedef struct muv_req_s muv_req_t;

typedef struct {
  ngx_queue_t node;
  muv_req_t*  req;
} node_t;

typedef enum {
  MUV_TCP_CONNECT,
  MUV_LISTEN,
  MUV_WRITE,
  MUV_CLOSE
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
  uv_stream_t* stream;
  int backlog;
  uv_connection_cb cb;
} muv_listen_t;

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
  uv_handle_t* handle;
  uv_close_cb cb;
} muv_close_t;

int muv_req_queue_push(muv_t* mid, muv_req_t* req);
void muv__async_cb(uv_async_t* async, int status);
void muv__thread_cb(void* arg);

#endif  /* INTERNAL_H_ */