#include <stdlib.h>
#include <stddef.h>

#include <uv.h>
#include <uv-private/ngx-queue.h>

#include "muv.h"
#include "internal.h"

int muv_req_queue_push(muv_t* mid, muv_req_t* req) {
  int r;
  node_t* node;

  node = (node_t*) malloc(sizeof(node_t));
  node->req = req;
  ngx_queue_init(&(node->node));

  uv_mutex_lock(&(mid->mutex));
  ngx_queue_insert_tail(&(mid->req_queue), &(node->node));
  uv_mutex_unlock(&(mid->mutex));

  r = uv_async_send(&(mid->async));
  if (r) {
    if (mid->error_cb) {
      uv_err_t err = uv_last_error(mid->loop);
      mid->error_cb(mid, err);
    }
    return r;
  }

  return 0;
}

void muv__async_cb(uv_async_t* async, int status) {
  int r;
  muv_t* mid;
  ngx_queue_t* q;
  node_t* node;

  mid = (muv_t*) async->data;

  uv_mutex_lock(&(mid->mutex));

  ngx_queue_foreach(q, &(mid->req_queue)) {
    node = ngx_queue_data(q, node_t, node);

    switch (node->req->type) {
    case MUV_TCP_CONNECT:
      {
        muv_tcp_connect_t* req = (muv_tcp_connect_t*) node->req;
        r = uv_tcp_connect(req->req, req->handle, req->address, req->cb);
        if (r) {
          if (req->cb) {
            req->cb(req->req, -1);
          }
        }
        break;
      }
    case MUV_TCP_CONNECT6:
      {
        muv_tcp_connect6_t* req = (muv_tcp_connect6_t*) node->req;
        r = uv_tcp_connect6(req->req, req->handle, req->address, req->cb);
        if (r) {
          if (req->cb) {
            req->cb(req->req, -1);
          }
        }
        break;
      }
    case MUV_LISTEN:
      {
        muv_listen_t* req = (muv_listen_t*) node->req;
        r = uv_listen(req->stream, req->backlog, req->cb);
        if (r) {
          if (req->cb) {
            req->cb(req->stream, -1);
          }
        }
        break;
      }
    case MUV_ACCEPT:
      {
        muv_accept_t* req = (muv_accept_t*) node->req;
        r = uv_accept(req->server, req->client);
        if (r) {
          if (mid->error_cb) {
            uv_err_t err = uv_last_error(mid->loop);
            mid->error_cb(mid, err);
          }
        }
        break;
      }
    case MUV_WRITE:
      {
        muv_write_t* req = (muv_write_t*) node->req;
        r = uv_write(req->req, req->handle, req->bufs, req->bufcnt, req->cb);
        if (r) {
          if (req->cb) {
            req->cb(req->req, -1);
          }
        }
        break;
      }
    case MUV_SHUTDOWN:
      {
        muv_shutdown_t* req = (muv_shutdown_t*) node->req;
        r = uv_shutdown(req->req, req->handle, req->cb);
        if (r) {
          if (req->cb) {
            req->cb(req->req, -1);
          }
        }
        break;
      }
    case MUV_CLOSE:
      {
        muv_close_t* req = (muv_close_t*) node->req;
        uv_close(req->handle, req->cb);
        break;
      }
    default:
      {
        uv_err_t err;
        err.code = UV_UNKNOWN;
        mid->error_cb(mid, err);
        break;
      }
    }

    ngx_queue_remove(q);
    free(node);
  }

  uv_mutex_unlock(&(mid->mutex));

}
void muv__thread_cb(void* arg) {
  muv_t* mid;

  mid = (muv_t*) arg;

  uv_run(mid->loop);
}
int muv_init(muv_t* mid, uv_loop_t* loop, muv_error_cb cb) {
  int r;

  mid->loop = loop;
  mid->error_cb = cb;
  mid->async.data = mid;

  r = uv_async_init(loop, &(mid->async), muv__async_cb);
  if (r) {
    if (mid->error_cb) {
      uv_err_t err = uv_last_error(mid->loop);
      mid->error_cb(mid, err);
    }
    return r;
  }

  r = uv_mutex_init(&(mid->mutex));
  if (r) {
    if (mid->error_cb) {
      uv_err_t err = uv_last_error(mid->loop);
      mid->error_cb(mid, err);
    }
    return r;
  }

  ngx_queue_init(&(mid->req_queue));

  r = uv_thread_create(&(mid->loop_tid), muv__thread_cb, mid);
  if (r) {
    if (mid->error_cb) {
      uv_err_t err = uv_last_error(mid->loop);
      mid->error_cb(mid, err);
    }
    return r;
  }

  return 0;
}
int muv_destroy(muv_t* mid) {
  int r;

  r = muv_close(mid, (uv_handle_t*) &(mid->async), NULL);
  if (r) {
    return r;
  }

  r = uv_thread_join(&(mid->loop_tid));
  if (r) {
    if (mid->error_cb) {
      uv_err_t err = uv_last_error(mid->loop);
      mid->error_cb(mid, err);
    }
    return r;
  }

  uv_mutex_destroy(&(mid->mutex));

  return 0;
}

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

  return muv_req_queue_push(mid, (muv_req_t*) tcp_connect_req);
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

  return muv_req_queue_push(mid, (muv_req_t*) tcp_connect6_req);
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
int muv_listen(muv_t* mid, uv_stream_t* stream, int backlog, uv_connection_cb cb) {
  muv_listen_t* listen_req;

  listen_req = (muv_listen_t*) malloc(sizeof(muv_listen_t));
  listen_req->type = MUV_LISTEN;
  listen_req->stream = stream;
  listen_req->backlog = backlog;
  listen_req->cb = cb;

  return muv_req_queue_push(mid, (muv_req_t*) listen_req);
}
int muv_accept(muv_t* mid, uv_stream_t* server, uv_stream_t* client) {
  muv_accept_t* accept_req;

  accept_req = (muv_accept_t*) malloc(sizeof(muv_accept_t));
  accept_req->type = MUV_ACCEPT;
  accept_req->server = server;
  accept_req->client = client;

  return muv_req_queue_push(mid, (muv_req_t*) accept_req);
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

  return muv_req_queue_push(mid, (muv_req_t*) write_req);
}
int muv_shutdown(muv_t* mid, uv_shutdown_t* req, uv_stream_t* handle,
    uv_shutdown_cb cb) {
  muv_shutdown_t* shutdown_req;

  shutdown_req = (muv_shutdown_t*) malloc(sizeof(muv_shutdown_t));
  shutdown_req->type = MUV_SHUTDOWN;
  shutdown_req->req = req;
  shutdown_req->handle = handle;
  shutdown_req->cb = cb;

  return muv_req_queue_push(mid, (muv_req_t*) shutdown_req);
}
int muv_close(muv_t* mid, uv_handle_t* handle, uv_close_cb close_cb) {
  muv_close_t* close_req;

  close_req = (muv_close_t*) malloc(sizeof(muv_close_t));
  close_req->type = MUV_CLOSE;
  close_req->handle = handle;
  close_req->cb = close_cb;

  return muv_req_queue_push(mid, (muv_req_t*) close_req);
}
