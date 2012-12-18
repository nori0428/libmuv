#include <stdlib.h>

#include <uv.h>

#include "muv.h"
#include "internal.h"

int muv_listen(muv_t* mid, uv_stream_t* stream, int backlog, uv_connection_cb cb) {
  muv_listen_t* listen_req;

  listen_req = (muv_listen_t*) malloc(sizeof(muv_listen_t));
  listen_req->type = MUV_LISTEN;
  listen_req->stream = stream;
  listen_req->backlog = backlog;
  listen_req->cb = cb;

  muv_req_queue_push(mid, (muv_req_t*) listen_req);
  return muv_req_queue_flush(mid);
}
int muv_accept(muv_t* mid, uv_stream_t* server, uv_stream_t* client) {
  muv_accept_t* accept_req;

  accept_req = (muv_accept_t*) malloc(sizeof(muv_accept_t));
  accept_req->type = MUV_ACCEPT;
  accept_req->server = server;
  accept_req->client = client;

  muv_req_queue_push(mid, (muv_req_t*) accept_req);
  return muv_req_queue_flush(mid);
}
int muv_read_start(muv_t* mid, uv_stream_t* handle, uv_alloc_cb alloc_cb,
    uv_read_cb read_cb) {
  muv_read_start_t* read_start_req;

  read_start_req = (muv_read_start_t*) malloc(sizeof(muv_read_start_t));
  read_start_req->type = MUV_READ_START;
  read_start_req->handle = handle;
  read_start_req->alloc_cb = alloc_cb;
  read_start_req->read_cb = read_cb;

  muv_req_queue_push(mid, (muv_req_t*) read_start_req);
  return muv_req_queue_flush(mid);
}
int muv_read_stop(muv_t* mid, uv_stream_t* handle) {
  muv_read_stop_t* read_stop_req;

  read_stop_req = (muv_read_stop_t*) malloc(sizeof(muv_read_stop_t));
  read_stop_req->type = MUV_READ_STOP;
  read_stop_req->handle = handle;

  muv_req_queue_push(mid, (muv_req_t*) read_stop_req);
  return muv_req_queue_flush(mid);
}
int muv_write(muv_t* mid, uv_write_t* req, uv_stream_t* handle,
    uv_buf_t bufs[], int bufcnt, uv_write_cb cb) {
  muv_write_t* write_req;

  write_req = (muv_write_t*) malloc(sizeof(muv_write_t));
  write_req->type = MUV_WRITE;
  write_req->req = req;
  write_req->handle = handle;
  write_req->bufs = bufs;
  write_req->bufcnt = bufcnt;
  write_req->cb = cb;

  muv_req_queue_push(mid, (muv_req_t*) write_req);
  return muv_req_queue_flush(mid);
}
int muv_shutdown(muv_t* mid, uv_shutdown_t* req, uv_stream_t* handle,
    uv_shutdown_cb cb) {
  muv_shutdown_t* shutdown_req;

  shutdown_req = (muv_shutdown_t*) malloc(sizeof(muv_shutdown_t));
  shutdown_req->type = MUV_SHUTDOWN;
  shutdown_req->req = req;
  shutdown_req->handle = handle;
  shutdown_req->cb = cb;

  muv_req_queue_push(mid, (muv_req_t*) shutdown_req);
  return muv_req_queue_flush(mid);
}

void muv__listen(muv_t* mid, muv_listen_t* req) {
  int r;

  r = uv_listen(req->stream, req->backlog, req->cb);
  if (r) {
    if (req->cb) {
      req->cb(req->stream, -1);
    }
  }
}
void muv__accept(muv_t* mid, muv_accept_t* req) {
  int r;

  r = uv_accept(req->server, req->client);
  if (r) {
    if (mid->error_cb) {
      uv_err_t err = uv_last_error(mid->loop);
      mid->error_cb(mid, err);
    }
  }
}
void muv__read_start(muv_t* mid, muv_read_start_t* req) {
  int r;

  r = uv_read_start(req->handle, req->alloc_cb, req->read_cb);
  if (r) {
    if (mid->error_cb) {
      uv_err_t err = uv_last_error(mid->loop);
      mid->error_cb(mid, err);
    }
  }
}
void muv__read_stop(muv_t* mid, muv_read_stop_t* req) {
  int r;

  r = uv_read_stop(req->handle);
  if (r) {
    if (mid->error_cb) {
      uv_err_t err = uv_last_error(mid->loop);
      mid->error_cb(mid, err);
    }
  }
}
void muv__write(muv_t* mid, muv_write_t* req) {
  int r;

  r = uv_write(req->req, req->handle, req->bufs, req->bufcnt, req->cb);
  if (r) {
    if (req->cb) {
      req->cb(req->req, -1);
    }
  }
}
void muv__shutdown(muv_t* mid, muv_shutdown_t* req) {
  int r;

  r = uv_shutdown(req->req, req->handle, req->cb);
  if (r) {
    if (req->cb) {
      req->cb(req->req, -1);
    }
  }
}
