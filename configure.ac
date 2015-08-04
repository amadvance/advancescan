# Process this file with autoconf to produce a configure script.
AC_INIT([AdvanceSCAN],[1.18], [], [], [http://www.advancemame.it])
AC_CONFIG_SRCDIR([scan.cc])
AM_INIT_AUTOMAKE([foreign no-dependencies subdir-objects])
AC_CONFIG_HEADERS([config.h])
AC_CANONICAL_HOST

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_AWK
AC_CHECK_PROGS(TAR, tar)
AC_CHECK_PROGS(GZIP, gzip)
AC_CHECK_PROGS(GROFF, groff)
AC_CHECK_PROGS(COL, col)

# Checks for architecture
AC_C_BIGENDIAN(
	[ac_is_bigendian=yes],
	[ac_is_bigendian=no]
)
if test $ac_is_bigendian = yes; then
	CFLAGS="$CFLAGS -DUSE_MSB"
	CXXFLAGS="$CXXFLAGS -DUSE_MSB"
else
	CFLAGS="$CFLAGS -DUSE_LSB"
	CXXFLAGS="$CXXFLAGS -DUSE_LSB"
fi

# Checks for libraries.
AC_CHECK_LIB([z], [adler32], [], [AC_MSG_ERROR([the libz library is missing])])

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_HEADER_DIRENT
AC_HEADER_TIME
AC_CHECK_HEADERS([unistd.h getopt.h utime.h stdarg.h varargs.h])
AC_CHECK_HEADERS([sys/types.h sys/stat.h sys/time.h sys/utime.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE

# Checks for library functions.
AC_CHECK_FUNCS([getopt getopt_long snprintf vsnprintf])

# Configure the library
CFLAGS="$CFLAGS -DUSE_ERROR_SILENT"
CXXFLAGS="$CXXFLAGS -DUSE_ERROR_SILENT"

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

