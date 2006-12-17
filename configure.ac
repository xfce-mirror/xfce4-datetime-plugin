dnl Version information
m4_define([datetime_version_major], [0])
m4_define([datetime_version_minor], [4])
m4_define([datetime_version_micro], [0])
m4_define([datetime_version], [datetime_version_major().datetime_version_minor().datetime_version_micro()])

dnl Initialize autoconf
AC_COPYRIGHT([Copyright (c) 2006
        Remco den Breeje <remco@sx.mine.nu>])
AC_INIT([datetime], [datetime_version()], [remco@sx.mine.nu])

dnl Initialize automake
AM_INIT_AUTOMAKE([AC_PACKAGE_TARNAME()], [AC_PACKAGE_VERSION()])
AM_CONFIG_HEADER([config.h])
AM_MAINTAINER_MODE()

dnl check for basic programs
AC_PROG_CC()
AC_PROG_INSTALL()
AC_PROG_INTLTOOL()
AC_PROG_LIBTOOL()

dnl Check for i18n support
XDT_I18N([nl])

dnl Check for required packages
XDT_CHECK_PACKAGE([LIBXFCE4PANEL],[libxfce4panel-1.0],[4.3.20])

dnl Check for optional packages
XDT_CHECK_OPTIONAL_PACKAGE([DBUS], [dbus-1], [0.22], [dbus], [D-BUS support])

dnl Check for debugging support
BM_DEBUG_SUPPORT()

AC_OUTPUT([
Makefile
po/Makefile.in
panel-plugin/Makefile
])
