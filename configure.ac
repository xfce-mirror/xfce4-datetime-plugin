#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(xfce4-panel-datetime-plugin, 0.2, krisna@users.berlios.de)
AM_INIT_AUTOMAKE([xfce4-datetime-plugin], [0.2])
AC_CONFIG_SRCDIR([datetime/datetime.c])
AC_CONFIG_HEADER([config.h])

AC_DISABLE_STATIC

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_TIME
AC_STRUCT_TM

# checks for the xfce panel plugin
PKG_CHECK_MODULES(XFCE4_PANEL, [xfce4-panel-1.0 >= 4.0.0],, 
		  AC_MSG_ERROR([xfce4-panel-date needs xfce4-panel 4.0.0 or higher]))
XFCE4_PANEL_PLUGINSDIR=`$PKG_CONFIG --variable=pluginsdir xfce4-panel-1.0`
AC_SUBST(XFCE4_PANEL_PLUGINSDIR)

# Checks for library functions.
AC_FUNC_STRFTIME

AC_CONFIG_FILES([Makefile datetime/Makefile])
AC_OUTPUT
