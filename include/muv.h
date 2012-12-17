#ifndef MUV_H_
#define MUV_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <uv.h>
#include <uv-private/ngx-queue.h>

typedef struct muv_s muv_t;

typedef void (*muv_error_cb)(muv_t* mid, uv_err_t err);

#define MUV_FIELDS        \
  void*        data;      \
  uv_loop_t*   loop;      \
  uv_async_t   async;     \
  uv_mutex_t   mutex;     \
  ngx_queue_t  req_queue; \
  uv_thread_t  loop_tid;  \
  muv_error_cb error_cb;  \

struct muv_s {
  MUV_FIELDS
};

int muv_init(muv_t* mid, uv_loop_t* loop, muv_error_cb cb);
int muv_destroy(muv_t* mid);

int muv_tcp_init(muv_t* mid, uv_loop_t* loop, uv_tcp_t* handle);
int muv_tcp_connect(muv_t* mid, uv_connect_t* req, uv_tcp_t* handle,
    struct sockaddr_in address, uv_connect_cb cb);
int muv_tcp_connect6(muv_t* mid, uv_connect_t* req, uv_tcp_t* handle,
    struct sockaddr_in6 address, uv_connect_cb cb);
int muv_tcp_bind(muv_t* mid, uv_tcp_t* handle, struct sockaddr_in address);
int muv_tcp_bind6(muv_t* mid, uv_tcp_t* handle, struct sockaddr_in6 address);
int muv_listen(muv_t* mid, uv_stream_t* stream, int backlog, uv_connection_cb cb);
int muv_accept(muv_t* mid, uv_stream_t* server, uv_stream_t* client);
int muv_write(muv_t* mid, uv_write_t* req, uv_stream_t* handle,
    uv_buf_t bufs[], int bufcnt, uv_write_cb cb);
int muv_shutdown(muv_t* mid, uv_shutdown_t* req, uv_stream_t* handle, uv_shutdown_cb cb);
int muv_close(muv_t* mid, uv_handle_t* handle, uv_close_cb close_cb);

#ifdef __cplusplus
}
#endif

#endif  /* MUV_H_ */
