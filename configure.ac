#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(xfce4-panel-datetime-plugin, 0.3.1, krisna@users.berlios.de)
AM_INIT_AUTOMAKE([xfce4-datetime-plugin], [0.3.1])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_MAINTAINER_MODE

AC_DISABLE_STATIC

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_TIME
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([strstr])

# checks for xfce stuff
# checks for the xfce panel plugin
XFCE_PANEL_PLUGIN([XFCE4_PANEL], [4.0.0])

# Check for i18n support
BM_I18N([xfce4-datetime], [ca eu fr hu ja ko lt pl pt_BR vi])

# Check for debugging support
BM_DEBUG_SUPPORT

AC_CONFIG_FILES([
Makefile
datetime/Makefile
po/Makefile.in
m4/Makefile
])
AC_OUTPUT
