
plugindir = @XFCE4_PANEL_PLUGINSDIR@
plugin_LTLIBRARIES = libdatetime.la

libdatetime_la_SOURCES = datetime.c
libdatetime_la_CFLAGS = \
	@XFCE4_PANEL_CFLAGS@ \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	-Wall

libdatetime_la_LDFLAGS = -avoid-version -export-dynamic -module
libdatetime_la_LIBADD = @XFCE4_PANEL_LIBS@
