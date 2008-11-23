/*
 * Code was taken from libxfce4panel (LGPL2 or any later version),
 * distributed here under the GPL.
 *
 * Copyright (c) 2005-2007 Jasper Huijsmans <jasper@xfce.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef HAVE_LIBXFCE4PANEL_46

#include "xfce46-compat.h"

#include <libxfce4panel/xfce-panel-plugin.h>
#include <libxfce4panel/xfce-panel-macros.h>

/* support macros for debugging */
#ifndef NDEBUG
#define _panel_assert(expr)                  g_assert (expr)
#define _panel_assert_not_reached()          g_assert_not_reached ()
#define _panel_return_if_fail(expr)          g_return_if_fail (expr)
#define _panel_return_val_if_fail(expr, val) g_return_val_if_fail (expr, (val))
#else
#define _panel_assert(expr)                  G_STMT_START{ (void)0; }G_STMT_END
#define _panel_assert_not_reached()          G_STMT_START{ (void)0; }G_STMT_END
#define _panel_return_if_fail(expr)          G_STMT_START{ (void)0; }G_STMT_END
#define _panel_return_val_if_fail(expr, val) G_STMT_START{ (void)0; }G_STMT_END
#endif

/**
 * xfce_panel_plugin_arrow_type:
 * @plugin        : an #XfcePanelPlugin
 *
 * Determine the #GtkArrowType for a widget that opens a menu and uses
 *  xfce_panel_plugin_position_menu() to position the menu.
 *
 * Returns: The #GtkArrowType to use.
 **/
static GtkArrowType
xfce_panel_plugin_arrow_type (XfcePanelPlugin *plugin)
{
    XfceScreenPosition  position;
    GdkScreen          *screen;
    GdkRectangle        geom;
    gint                mon, x, y;

    if (!GTK_WIDGET_REALIZED (plugin))
        return GTK_ARROW_UP;

    position = xfce_panel_plugin_get_screen_position (plugin);
    switch (position)
    {
        /* top */
        case XFCE_SCREEN_POSITION_NW_H:
        case XFCE_SCREEN_POSITION_N:
        case XFCE_SCREEN_POSITION_NE_H:
            return GTK_ARROW_DOWN;

        /* left */
        case XFCE_SCREEN_POSITION_NW_V:
        case XFCE_SCREEN_POSITION_W:
        case XFCE_SCREEN_POSITION_SW_V:
            return GTK_ARROW_RIGHT;

        /* right */
        case XFCE_SCREEN_POSITION_NE_V:
        case XFCE_SCREEN_POSITION_E:
        case XFCE_SCREEN_POSITION_SE_V:
            return GTK_ARROW_LEFT;

        /* bottom */
        case XFCE_SCREEN_POSITION_SW_H:
        case XFCE_SCREEN_POSITION_S:
        case XFCE_SCREEN_POSITION_SE_H:
            return GTK_ARROW_UP;

        /* floating */
        default:
            /* get the screen information */
            screen = gtk_widget_get_screen (GTK_WIDGET (plugin));
            mon = gdk_screen_get_monitor_at_window (screen, GTK_WIDGET (plugin)->window);
            gdk_screen_get_monitor_geometry (screen, mon, &geom);
            gdk_window_get_root_origin (GTK_WIDGET (plugin)->window, &x, &y);

            /* get the position based on the screen position */
            if (position == XFCE_SCREEN_POSITION_FLOATING_H)
                return ((y < (geom.y + geom.height / 2)) ? GTK_ARROW_DOWN : GTK_ARROW_UP);
            else
                return ((x < (geom.x + geom.width / 2)) ? GTK_ARROW_RIGHT : GTK_ARROW_LEFT);
    }
}



/**
 * xfce_panel_plugin_position_widget:
 * @plugin        : an #XfcePanelPlugin
 * @menu_widget   : a #GtkWidget that will be used as popup menu
 * @attach_widget : a #GtkWidget relative to which the menu should be positioned
 * @x             : return location for the x coordinate
 * @y             : return location for the y coordinate
 *
 * The menu widget is positioned relative to @attach_widget.
 * If @attach_widget is NULL, the menu widget is instead positioned
 * relative to @panel_plugin.
 *
 * This function is intended for custom menu widgets.
 * For a regular #GtkMenu you should use xfce_panel_plugin_position_menu()
 * instead (as callback argument to gtk_menu_popup()).
 *
 * See also: xfce_panel_plugin_position_menu().
 **/
void
xfce_panel_plugin_position_widget (XfcePanelPlugin  *plugin,
                                   GtkWidget        *menu_widget,
                                   GtkWidget        *attach_widget,
                                   gint             *x,
                                   gint             *y)
{
    GtkRequisition  req;
    GdkScreen      *screen;
    GdkRectangle    geom;
    gint            mon;

    _panel_return_if_fail (XFCE_IS_PANEL_PLUGIN (plugin));
    _panel_return_if_fail (GTK_IS_WIDGET (menu_widget));
    _panel_return_if_fail (attach_widget == NULL || GTK_IS_WIDGET (attach_widget));

    if (attach_widget == NULL)
        attach_widget = GTK_WIDGET (plugin);

    if (!GTK_WIDGET_REALIZED (menu_widget))
        gtk_widget_realize (menu_widget);

    gtk_widget_size_request (menu_widget, &req);
    gdk_window_get_origin (attach_widget->window, x, y);

    switch (xfce_panel_plugin_arrow_type (plugin))
    {
        case GTK_ARROW_UP:
            *y -= req.height;
            break;

        case GTK_ARROW_DOWN:
            *y += attach_widget->allocation.height;
            break;

        case GTK_ARROW_LEFT:
            *x -= req.width;
            break;

        default: /* GTK_ARROW_RIGHT and GTK_ARROW_NONE */
            *x += attach_widget->allocation.width;
            break;
    }

    screen = gtk_widget_get_screen (attach_widget);
    mon = gdk_screen_get_monitor_at_window (screen, attach_widget->window);
    gdk_screen_get_monitor_geometry (screen, mon, &geom);

    /* keep inside the screen */
    if (*x > geom.x + geom.width - req.width)
        *x = geom.x + geom.width - req.width;
    if (*x < geom.x)
        *x = geom.x;
    if (*y > geom.y + geom.height - req.height)
        *y = geom.y + geom.height - req.height;
    if (*y < geom.y)
        *y = geom.y;

    if (G_LIKELY (GTK_IS_MENU (menu_widget)))
        gtk_menu_set_screen (GTK_MENU (menu_widget), screen);
    else if (GTK_IS_WINDOW (menu_widget))
        gtk_window_set_screen (GTK_WINDOW (menu_widget), screen);
}

#endif
