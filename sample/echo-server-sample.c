#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <uv.h>

#include "muv.h"

#define MAX_SIZE (256)

void error_cb(muv_t* mid, uv_err_t err) {
  printf("error occurred. %s\n", uv_strerror(err));
}

void close_cb(uv_handle_t* handle) {
  free(handle);
}
void write_cb(uv_write_t* req, int status) {
  free(req->data);
  free(req);
}
uv_buf_t alloc_cb(uv_handle_t* handle, size_t suggested_size) {
  return uv_buf_init((char*) malloc(suggested_size), suggested_size);
}
void read_cb(uv_stream_t* stream, ssize_t nread, uv_buf_t buf) {
  if (nread > 0) {
    int i, r;
    for (i = 0; i < nread; i++) {
      printf("%c", buf.base[i]);
    }

    uv_buf_t write_buf = uv_buf_init(buf.base, nread);
    uv_write_t* write_req = (uv_write_t*) malloc(sizeof(uv_write_t));
    write_req->data = write_buf.base;
    r = uv_write(write_req, stream, &write_buf, 1, write_cb);
    if (r) {
      uv_err_t err = uv_last_error(stream->loop);
      printf("error occurred. %s\n", uv_strerror(err));
    }
  } else if (nread == 0) {
    free(buf.base);
  } else {
    uv_err_t err = uv_last_error(uv_default_loop());
    if (err.code != UV_EOF) {
      printf("error occurred. %s\n", uv_strerror(err));
    }
    uv_close((uv_handle_t*) stream, close_cb);
    free(buf.base);
  }
}
void connection_cb(uv_stream_t* server, int status) {
  int r;
  if (!status) {
    uv_tcp_t* client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
    r = uv_tcp_init(server->loop, client);
    if (r) {
      uv_err_t err = uv_last_error(server->loop);
      printf("error occurred. %s\n", uv_strerror(err));
    }
    r = uv_accept(server, (uv_stream_t*) client);
    if (r) {
      uv_err_t err = uv_last_error(server->loop);
      printf("error occurred. %s\n", uv_strerror(err));
    }
    r = uv_read_start((uv_stream_t*) client, alloc_cb, read_cb);
    if (r) {
      uv_err_t err = uv_last_error(server->loop);
      printf("error occurred. %s\n", uv_strerror(err));
    }
  } else {
    uv_err_t err = uv_last_error(server->loop);
    printf("error occurred. %s\n", uv_strerror(err));
  }
}

int main() {
  int r;
  char c;
  muv_t mid;
  char input[MAX_SIZE];
  char* p;

  r = muv_init(&mid, uv_default_loop(), error_cb);
  if (r) {
    return -1;
  }

  struct sockaddr_in address = uv_ip4_addr("127.0.0.1", 8080);
  uv_tcp_t* handle = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
  r = muv_tcp_init(&mid, mid.loop, handle);
  if (r) {
    return -1;
  }
  r = muv_tcp_bind(&mid, handle, address);
  if (r) {
    return -1;
  }
  r = muv_listen(&mid, (uv_stream_t*) handle, 10, connection_cb);
  if (r) {
    return -1;
  }

  printf("press any key to exit.\n");
  p = fgets(input, MAX_SIZE, stdin);

  r = muv_close(&mid, (uv_handle_t*) handle, close_cb);
  if (r) {
    return -1;
  }

  r = muv_destroy(&mid);
  if (r) {
    return -1;
  }

  return 0;
}
