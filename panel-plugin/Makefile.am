plugindir = $(libdir)/xfce4/panel/plugins/

plugin_LTLIBRARIES = 				\
	libdatetime.la

libdatetime_la_SOURCES = 			\
	datetime.h				\
	datetime.c				\
	datetime-dialog.h			\
	datetime-dialog.c

libdatetime_la_CFLAGS = 			\
	-I$(top_srcdir)				\
	-DLOCALEDIR=\"$(localedir)\"		\
	$(LIBXFCE4PANEL_CFLAGS)			\
	$(LIBXFCE4UI_CFLAGS)

libdatetime_la_LDFLAGS = 			\
	-avoid-version				\
	-module \
	-no-undefined \
	-export-symbols-regex '^xfce_panel_module_(preinit|init|construct)' \
	$(PLATFORM_LDFLAGS)

libdatetime_la_LIBADD = 			\
	$(LIBXFCE4PANEL_LIBS)			\
	$(LIBXFCE4UI_LIBS)

desktopdir = $(datadir)/xfce4/panel/plugins
desktop_in_files = datetime.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@


EXTRA_DIST = $(desktop_in_files)

CLEANFILES = $(desktop_DATA)
