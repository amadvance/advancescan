# Process this file with autoconf to produce a configure script.
AC_INIT([advancescan],[1.2])
AM_INIT_AUTOMAKE([advancescan], [1.2])
AM_CONFIG_HEADER([config.h])
AC_CONFIG_SRCDIR([scan.cc])
AC_CANONICAL_HOST

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_AWK
AC_CHECK_PROGS(TAR, tar)
AC_CHECK_PROGS(GZIP, gzip)
AC_CHECK_PROGS(GROFF, groff)
AC_CHECK_PROGS(COL, col)

# Checks for libraries.
AC_CHECK_LIB([z], [adler32])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_DEFINE(USE_7Z,1,[Use the 7z library])
AC_DEFINE(USE_LZMA,1,[Use the lzma compression])

AC_ARG_ENABLE(
	bzip2,
	[  --enable-bzip2       Use the bzip2 compression],
	AC_DEFINE(USE_BZIP2,1,[Use the bzip2 compression])
	AC_CHECK_LIB([bz2],[BZ2_bzBuffToBuffCompress])
)

# Define PREFIX
CPPFLAGS="$CPPFLAGS -DPREFIX=\\\"\$(prefix)\\\""

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

