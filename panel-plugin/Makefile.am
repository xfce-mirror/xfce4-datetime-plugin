plugindir = $(libdir)/xfce4/panel-plugins/

plugin_LTLIBRARIES = 				\
	libdatetime.la

libdatetime_la_LDFLAGS = 			\
	-avoid-version				\
	-module
#-export-dynamic

libdatetime_la_SOURCES = 			\
	datetime.h				\
	datetime.c				\
	datetime-dialog.h			\
	datetime-dialog.c

libdatetime_la_CFLAGS = 			\
	-I$(top_srcdir)				\
	-DLOCALEDIR=\"$(localedir)\"		\
	@LIBXFCE4PANEL_CFLAGS@

libdatetime_la_LIBADD = 			\
	@LIBXFCE4PANEL_LIBS@
#	../../libxfce4panel/libxfce4panel.la

#libdatetime_la_DEPENDENCIES = 			\
#	../../libxfce4panel/libxfce4panel.la

%.desktop.in: %.desktop.in.in
	sed -e "s^@INTERNAL_PLUGIN_PATH@^$(libdir)/xfce4/panel-plugins^"  < $< > $@

desktop_in_in_files = datetime.desktop.in.in
desktop_in_files = $(desktop_in_in_files:.desktop.in.in=.desktop.in)

desktopdir = $(datadir)/xfce4/panel-plugins
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@


EXTRA_DIST =					\
	$(desktop_in_in_files)

DISTCLEANFILES =				\
	$(desktop_DATA) $(desktop_in_files)

