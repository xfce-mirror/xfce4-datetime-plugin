plugindir = $(libdir)/xfce4/panel-plugins/

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
	$(GTK_CFLAGS)				\
	$(LIBXFCE4PANEL_CFLAGS)			\
	$(LIBXFCEGUI4_CFLAGS)			\
	$(LIBXFCE4UTIL_CFLAGS)

libdatetime_la_LDFLAGS = 			\
	-avoid-version				\
	-module

if HAVE_CYGWIN
libdatetime_la_LDFLAGS += 			\
	-no-undefined
endif

libdatetime_la_LIBADD = 			\
	$(GTK_LIBS)				\
	$(LIBXFCE4PANEL_LIBS)			\
	$(LIBXFCEGUI4_LIBS)			\
	$(LIBXFCE4UTIL_LIBS)

desktopdir = $(datadir)/xfce4/panel-plugins
desktop_in_in_files = datetime.desktop.in.in
desktop_in_files = $(desktop_in_in_files:.desktop.in.in=.desktop.in)
%.desktop.in: %.desktop.in.in
	sed -e "s,\@libdir\@,$(libdir),g" < $< > $@
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@


EXTRA_DIST =					\
	$(desktop_in_in_files)

DISTCLEANFILES =				\
	$(desktop_DATA)				\
	$(desktop_in_files)

