#ifndef _XFCE46_COMPAT
#define _XFCE46_COMPAT

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef HAVE_LIBXFCE4PANEL_46

#include <gtk/gtk.h>
#include <libxfce4panel/xfce-panel-plugin.h>

void                 xfce_panel_plugin_position_widget      (XfcePanelPlugin  *plugin,
                                                             GtkWidget        *menu_widget,
                                                             GtkWidget        *attach_widget,
                                                             gint             *x,
                                                             gint             *y);

#endif
#endif
