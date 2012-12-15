AM_CFLAGS = \
	-I$(UVDIR)/include \
	-I$(top_srcdir)/include

bin_PROGRAMS = \
	echo-client-sample \
	echo-server-sample

echo_client_sample_SOURCES = \
    echo-client-sample.c

echo_client_sample_LDADD = \
	$(UVDIR)/libuv.a \
	$(top_srcdir)/src/libmuv.la

echo_server_sample_SOURCES = \
    echo-server-sample.c

echo_server_sample_LDADD = \
	$(UVDIR)/libuv.a \
	$(top_srcdir)/src/libmuv.la
