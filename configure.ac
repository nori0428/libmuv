AC_PREREQ([2.68])
AC_INIT([libmuv], [0.1.0], [shsfre09@gmail.com])
AC_CONFIG_SRCDIR([src/muv.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([.])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])

# Checks for programs.
AC_PROG_CC

m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

# Checks for libraries.
# AC_PROG_RANLIB
AC_PROG_LIBTOOL
# LT_INIT

# FIXME: Replace `main' with a function in `-lm':
AC_CHECK_LIB([m], [main])
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [main])
# FIXME: Replace `main' with a function in `-lrt':
AC_CHECK_LIB([rt], [main])
# FIXME: Replace `main' with a function in `-lws2_32':
AC_CHECK_LIB([ws2_32], [main])
# FIXME: Replace `main' with a function in `-lpsapi':
AC_CHECK_LIB([psapi], [main])
# FIXME: Replace `main' with a function in `-liphlpapi':
AC_CHECK_LIB([iphlpapi], [main])

AC_CANONICAL_HOST
case $host_os in
    darwin* )
        CFLAGS="$CFLAGS -framework CoreServices"
        ;;
    linux*)
        ;;
    *BSD*)
        ;;
    mingw*)
        CFLAGS="$CFLAGS -DWINVER=0x0501"
        ;;
    *)
        ;;
esac

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h locale.h mach/mach.h malloc.h netdb.h netinet/in.h paths.h stddef.h stdint.h stdlib.h string.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h termios.h unistd.h utime.h utmp.h wchar.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_CHECK_MEMBERS([struct stat.st_rdev])
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_CHOWN
AC_FUNC_FORK
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([bzero clock_gettime dup2 fdatasync floor ftruncate getcwd gethrtime getpagesize gettimeofday memmove memset mkdir pathconf putenv realpath rmdir select socket strchr strdup strndup strrchr strstr sysinfo utime utmpname])

# Defines options
PWD=`pwd`
AC_ARG_WITH([libuv],
            [AC_HELP_STRING([--with-libuv], [build with external libuv])],
            [UVDIR=${withval}],
            [UVDIR=${PWD}/deps/libuv])
AC_SUBST(UVDIR)

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 sample/Makefile])
AC_OUTPUT
