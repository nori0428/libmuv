#include <stdlib.h>
#include <stddef.h>

#include <uv.h>
#include <uv-private/ngx-queue.h>

#include "muv.h"
#include "internal.h"

void muv_req_queue_push(muv_t* mid, muv_req_t* req) {
  node_t* node;

  node = (node_t*) malloc(sizeof(node_t));
  node->req = req;
  ngx_queue_init(&(node->node));

  uv_mutex_lock(&(mid->mutex));
  ngx_queue_insert_tail(&(mid->req_queue), &(node->node));
  uv_mutex_unlock(&(mid->mutex));
}
int muv_req_queue_flush(muv_t* mid) {
  int r;

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
void muv__req_queue_flush(muv_t* mid) {
  int r;
  ngx_queue_t* q;
  node_t* node;

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
    free(node->req);
    free(node);
  }

  uv_mutex_unlock(&(mid->mutex));
}
void muv__async_cb(uv_async_t* async, int status) {
  muv_t* mid;

  mid = (muv_t*) async->data;

  muv__req_queue_flush(mid);
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
