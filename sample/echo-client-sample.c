#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <uv.h>

#include "muv.h"

#define MAX_SIZE (256)

typedef struct {
  unsigned char is_connected;
} state_t;

void error_cb(muv_t* mid, uv_err_t err) {
  printf("error occurred. %s\n", uv_strerror(err));
}

void close_cb(uv_handle_t* handle) {
  state_t* state = (state_t*) handle->data;
  state->is_connected = 0;
  free(handle);
}
uv_buf_t alloc_cb(uv_handle_t* handle, size_t suggested_size) {
  return uv_buf_init((char*) malloc(suggested_size), suggested_size);
}
void read_cb(uv_stream_t* stream, ssize_t nread, uv_buf_t buf) {
  if (nread > 0) {
    int i;
    for (i = 0; i < nread; i++) {
      printf("%c", buf.base[i]);
    }
  } else if (nread == 0) {
  } else {
    uv_err_t err = uv_last_error(uv_default_loop());
    if (err.code != UV_EOF) {
      printf("error occurred. %s\n", uv_strerror(err));
    }
    uv_close((uv_handle_t*) stream, close_cb);
  }
  free(buf.base);
}
void write_cb(uv_write_t* req, int status) {
  printf("written.\n");
  free(req->data);
  free(req);
}
void connect_cb(uv_connect_t* req, int status) {
  int r;
  if (!status) {
    state_t* state = (state_t*) req->handle->data;
    state->is_connected = 1;
    r = uv_read_start(req->handle, alloc_cb, read_cb);
    if (r) {
      uv_err_t err = uv_last_error(req->handle->loop);
      printf("error occurred. %s\n", uv_strerror(err));
    }
  } else {
    uv_err_t err = uv_last_error(req->handle->loop);
    printf("error occurred. %s\n", uv_strerror(err));
  }

  free(req);
}

int main() {
  int r;
  muv_t mid;
  state_t state;
  char input[MAX_SIZE];
  char* p;

  r = muv_init(&mid, uv_default_loop(), error_cb);
  if (r) {
    return -1;
  }

  struct sockaddr_in address = uv_ip4_addr("127.0.0.1", 8080);
  uv_connect_t* connect_req = (uv_connect_t*) malloc(sizeof(uv_connect_t));
  uv_tcp_t* handle = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
  r = muv_tcp_init(&mid, mid.loop, handle);
  if (r) {
    return -1;
  }
  handle->data = &state;
  state.is_connected = 0;
  r = muv_tcp_connect(&mid, connect_req, handle, address, connect_cb);
  if (r) {
    return -1;
  }
  printf("now connecting.");
  while (!state.is_connected) {
    printf(".");
    usleep(100000);
  }
  printf("connected.\n");

  do {
    printf("input: ");
    p = fgets(input, MAX_SIZE, stdin);

    if (!state.is_connected) {
      break;
    }
    size_t len = strlen(input);
    uv_buf_t write_buf = uv_buf_init((char*) malloc(len), len);
    memcpy(write_buf.base, input, len);
    uv_write_t* write_req = (uv_write_t*) malloc(sizeof(uv_write_t));
    write_req->data = write_buf.base;
    r = muv_write(&mid, write_req, (uv_stream_t*) handle, &write_buf, 1, write_cb);
    if (r) {
      return -1;
    }
    printf("now writing.");
  } while (strncmp(input, "exit", 4) != 0);

  if (state.is_connected) {
    r = muv_close(&mid, (uv_handle_t*) handle, close_cb);
    if (r) {
      return -1;
    }
    printf("now closing.");
    while (state.is_connected) {
      printf(".");
      usleep(100000);
    }
  }
  printf("closed.\n");

  r = muv_destroy(&mid);
  if (r) {
    return -1;
  }

  return 0;
}
