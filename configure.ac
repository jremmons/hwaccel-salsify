#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([hw-salsify], [0.1], [excamera@cs.stanford.edu])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_HEADERS([config.h])

# Add picky CXXFLAGS
CXX14_FLAGS="-std=c++14 -pthread"
PICKY_CXXFLAGS="-pedantic -Wall -Wextra -Weffc++ -Wno-non-virtual-dtor"
AC_SUBST([CXX14_FLAGS])
AC_SUBST([PICKY_CXXFLAGS])

# Checks for programs.
AC_PROG_CXX
AC_PROG_RANLIB

# Checks for libraries.
PKG_CHECK_MODULES([AVFORMAT], [libavformat])
PKG_CHECK_MODULES([AVCODEC], [libavcodec])
PKG_CHECK_MODULES([AVUTIL], [libavutil])
PKG_CHECK_MODULES([AVFILTER], [libavfilter])
PKG_CHECK_MODULES([AVDEVICE], [libavdevice])
PKG_CHECK_MODULES([AVDEVICE], [libavdevice])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile
         third_party/Makefile
         src/Makefile
         src/libx264/Makefile
         src/util/Makefile
	 ])
AC_OUTPUT
