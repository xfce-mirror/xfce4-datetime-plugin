/*  date.c
 *
 *  Copyright (C) 2003 Choe Hwanjin(krisna@kldp.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <time.h>
#include <gdk/gdkkeysyms.h>

#include <panel/global.h>
#include <panel/controls.h>
#include <panel/icons.h>
#include <panel/plugins.h>

typedef struct {
    GtkWidget *eventbox;
    GtkWidget *label;
    gchar *font;
    gchar *format;
    guint timeout_id;
    gint orientation;

    GtkWidget *font_selector;
    GtkWidget *format_entry;
    GtkWidget *cal;
} DatetimePlugin;

static gboolean
datetime_update(gpointer data)
{
    GTimeVal timeval;
    gchar buf[256];
    gchar *utf8str;
    int len;
    struct tm *current;
    DatetimePlugin *datetime;

    if (data == NULL)
	return FALSE;

    datetime = (DatetimePlugin*)data;
    if (!GTK_IS_LABEL(datetime->label))
	return FALSE;

    g_get_current_time(&timeval);
    current = localtime((time_t *)&timeval.tv_sec);
    len = strftime(buf, sizeof(buf) - 1, datetime->format, current);
    if (len != 0) {
	buf[len] = '\0';  /* make sure nul terminated string */
	utf8str = g_locale_to_utf8(buf, len, NULL, NULL, NULL);
	if (utf8str != NULL) {
	    gtk_label_set_text(GTK_LABEL(datetime->label), utf8str);
	    g_free(utf8str);
	    return TRUE;
	}
    }
    gtk_label_set_text(GTK_LABEL(datetime->label), "Error");
    return TRUE;
}

static GtkWidget *
pop_calendar_window(GtkWidget *parent, int orientation)
{
    GtkWidget *window;
    GtkWidget *cal;
    gint parent_x, parent_y, parent_w, parent_h;
    gint root_w, root_h;
    gint width, height, x, y;
    GtkRequisition requisition;
    GtkAllocation allocation;

    window = gtk_window_new(GTK_WINDOW_POPUP);

    cal = gtk_calendar_new();
    gtk_container_add(GTK_CONTAINER(window), cal);

    gdk_window_get_origin(GDK_WINDOW(parent->window), &parent_x, &parent_y);
    gdk_drawable_get_size(GDK_DRAWABLE(parent->window), &parent_w, &parent_h);

    root_w = gdk_screen_width();
    root_h = gdk_screen_height();

    gtk_widget_realize(GTK_WIDGET(window));

    gtk_widget_size_request(GTK_WIDGET(cal), &requisition);

    allocation.x = requisition.width;
    allocation.y = requisition.height;
    gtk_widget_size_allocate(GTK_WIDGET(cal), &allocation);

    gtk_widget_size_request(GTK_WIDGET(cal), &requisition);
    width = requisition.width;
    height = requisition.height;

    g_print("parent: %dx%d +%d+%d\n", parent_w, parent_h, parent_x, parent_y);
    g_print("root: %dx%d\n", root_w, root_h);
    g_print("calendar: %dx%d\n", width, height);

    if (orientation == GTK_ORIENTATION_VERTICAL) {
        if (parent_x < root_w / 2) {
            if (parent_y < root_h / 2) {
                /* upper left */
                x = parent_x + parent_w;
                y = parent_y;
            } else {
                /* lower left */
                x = parent_x + parent_w;
                y = parent_y + parent_h - height;
            }
        } else {
            if (parent_y < root_h / 2) {
                /* upper right */
                x = parent_x - width;
                y = parent_y;
            } else {
                /* lower right */
                x = parent_x - width;
                y = parent_y + parent_h - height;
            }
        }
    } else {
        if (parent_x < root_w / 2) {
            if (parent_y < root_h / 2) {
                /* upper left */
                x = parent_x;
                y = parent_y + parent_h;
            } else {
                /* lower left */
                x = parent_x;
                y = parent_y - height;
            }
        } else {
            if (parent_y < root_h / 2) {
                /* upper right */
                x = parent_x + parent_w - width;
                y = parent_y + parent_h;
            } else {
                /* lower right */
                x = parent_x + parent_w - width;
                y = parent_y - height;
            }
        }
    }

    gtk_window_move(GTK_WINDOW(window), x, y);
    gtk_widget_show(cal);
    gtk_widget_show(window);

    return window;
}

static gboolean
on_button_press_event_cb(GtkWidget *widget,
			 GdkEventButton *event, gpointer data)
{
    if (event->button == 1) {
    	DatetimePlugin *datetime;

	if (data == NULL)
	    return FALSE;

	datetime = (DatetimePlugin*)data;
	if (datetime->cal != NULL) {
	    gtk_widget_destroy(datetime->cal);
	    datetime->cal = NULL;
	} else {
	    datetime->cal = pop_calendar_window(datetime->eventbox,
						datetime->orientation);
	}
	return TRUE;
    }
    return FALSE;
}

static gboolean
on_key_press_event_cb(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    if (event->keyval == GDK_Escape) {
    	DatetimePlugin *datetime = (DatetimePlugin*)data;

	if (datetime != NULL && datetime->cal) {
	    gtk_widget_destroy (GTK_WIDGET(datetime->cal));
	    datetime->cal = NULL;
	}
	return TRUE;
    }   
    return FALSE;
}

static DatetimePlugin *
datetime_new (void)
{
    DatetimePlugin *datetime = g_new (DatetimePlugin, 1);

    datetime->eventbox = gtk_event_box_new();
    gtk_widget_add_events(datetime->eventbox, GDK_KEY_PRESS_MASK);
    g_signal_connect(G_OBJECT(datetime->eventbox), "button-press-event",
	    	     G_CALLBACK(on_button_press_event_cb), datetime);
    g_signal_connect(G_OBJECT(datetime->eventbox), "key-press-event",
	    	     G_CALLBACK(on_key_press_event_cb), datetime);

    datetime->label = gtk_label_new("");
    gtk_container_add(GTK_CONTAINER(datetime->eventbox), datetime->label);
    gtk_label_set_justify(GTK_LABEL(datetime->label), GTK_JUSTIFY_CENTER);
    datetime->font = g_strdup("Bitstream Vera Sans 11");
    datetime->format = g_strdup("%Y-%m-%d\n%H:%M");
    datetime_update(datetime);
    gtk_widget_show_all(datetime->eventbox);

    datetime->cal = NULL;
    datetime->timeout_id = g_timeout_add(15000, datetime_update, datetime);
    datetime->orientation = GTK_ORIENTATION_HORIZONTAL;

    return datetime;
}

static void
datetime_free(Control *control)
{
    DatetimePlugin *datetime;

    g_return_if_fail (control != NULL);

    datetime = control->data;
    g_return_if_fail (datetime != NULL);

    if (datetime->timeout_id)
	g_source_remove(datetime->timeout_id);

    g_free(datetime);
}

extern xmlDocPtr xmlconfig;
static void
datetime_read_config(Control *control, xmlNodePtr node)
{
    DatetimePlugin *datetime;
    xmlChar *value;

    g_return_if_fail (control != NULL);
    g_return_if_fail (node != NULL);

    datetime = (DatetimePlugin*)control->data;

    node = node->children;
    if (node == NULL || !xmlStrEqual(node->name, (const xmlChar *)"Date"))
	return;

    node = node->children;
    while (node != NULL) {
	if (xmlStrEqual(node->name, (const xmlChar *)"Font")) {
	    value = xmlNodeListGetString(xmlconfig, node->children, 1);
	    if (value != NULL) {
		PangoFontDescription *font;

		g_free(datetime->font);
		datetime->font = g_strdup(value);
		xmlFree(value);

		font = pango_font_description_from_string(datetime->font);
		gtk_widget_modify_font(datetime->label, font);
	    }
	} else if (xmlStrEqual(node->name, (const xmlChar *)"Format")) {
	    value = xmlNodeListGetString(xmlconfig, node->children, 1);
	    if (value != NULL) {
		g_free(datetime->format);
		datetime->format = g_strcompress(value);
		xmlFree(value);
	    }
	}
	node = node->next;
    }
    datetime_update(datetime);
}

static void
datetime_write_config(Control *control, xmlNodePtr parent)
{
    DatetimePlugin *datetime;
    gchar *format;
    xmlNodePtr node;

    g_return_if_fail (control != NULL);
    g_return_if_fail (parent != NULL);
   
    datetime = (DatetimePlugin*)control->data;
    g_return_if_fail (datetime != NULL);

    node = xmlNewTextChild(parent, NULL, (const xmlChar *)"Date", NULL);
    xmlNewTextChild(node, NULL, (const xmlChar *)"Font", datetime->font);
    format = g_strescape(datetime->format, NULL);
    xmlNewTextChild(node, NULL, (const xmlChar *)"Format", format);
    g_free(format);
}

static void
datetime_attach_callback(Control *control, const char *signal,
		     GCallback callback, gpointer data)
{
}

static void
datetime_apply_options_cb(GtkWidget *widget, gpointer data)
{
    DatetimePlugin *datetime;
    const gchar *font_name;
    const gchar *format;

    g_return_if_fail (data != NULL);

    datetime = (DatetimePlugin *)data;

    font_name = gtk_button_get_label(GTK_BUTTON(datetime->font_selector));
    if (font_name != NULL) {
	PangoFontDescription *font;

	g_free(datetime->font);
	datetime->font = g_strdup(font_name);
	font = pango_font_description_from_string(datetime->font);
	gtk_widget_modify_font(datetime->label, font);
    }

    format = (gchar *)gtk_entry_get_text(GTK_ENTRY(datetime->format_entry));
    if (format != NULL) {
	g_free(datetime->format);
	datetime->format = g_strcompress(format);
    }
    datetime_update(datetime);
}

static void
datetime_font_selection_cb(GtkWidget *widget, gpointer data)
{
    DatetimePlugin *datetime;
    GtkWidget *dialog;
    gint result;

    g_return_if_fail (data != NULL);

    datetime = (DatetimePlugin*)data;

    dialog = gtk_font_selection_dialog_new("Select font");
    gtk_font_selection_dialog_set_font_name(GTK_FONT_SELECTION_DIALOG(dialog),
	   	 			    datetime->font);
    gtk_font_selection_dialog_set_preview_text(GTK_FONT_SELECTION_DIALOG(dialog),
				    gtk_label_get_text(GTK_LABEL(datetime->label)));
    result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_OK || result == GTK_RESPONSE_ACCEPT) {
	gchar *font_name;
	font_name = gtk_font_selection_dialog_get_font_name(GTK_FONT_SELECTION_DIALOG(dialog));
	if (font_name != NULL)
	    gtk_button_set_label(GTK_BUTTON(widget), font_name);
    }
    gtk_widget_destroy(dialog);
}

static void
datetime_create_options(Control *control, GtkContainer *container, GtkWidget *done)
{
    DatetimePlugin *datetime;
    GtkWidget *table;
    GtkWidget *label;
    gchar *format;

    g_return_if_fail (control != NULL);
    g_return_if_fail (container != NULL);
    g_return_if_fail (done != NULL);

    datetime = (DatetimePlugin*)control->data;
    g_return_if_fail (datetime != NULL);

    table = gtk_table_new(2, 2, FALSE);

    label = gtk_label_new("Font:");
    gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 0, 1);
    datetime->font_selector = gtk_button_new_with_label(datetime->font);
    g_signal_connect(G_OBJECT(datetime->font_selector), "clicked",
	    	     G_CALLBACK(datetime_font_selection_cb), datetime);
    gtk_table_attach_defaults(GTK_TABLE(table),
	    		      datetime->font_selector, 1, 2, 0, 1);

    label = gtk_label_new("Format:");
    gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 1, 2);
    datetime->format_entry = gtk_entry_new();
    format = g_strescape(datetime->format, NULL);
    gtk_entry_set_text(GTK_ENTRY(datetime->format_entry), format);
    g_free(format);
    gtk_table_attach_defaults(GTK_TABLE(table),
	    		      datetime->format_entry, 1, 2, 1, 2);
    
    g_signal_connect(G_OBJECT(done), "clicked",
	    	     G_CALLBACK(datetime_apply_options_cb), datetime);

    gtk_widget_show_all(table);
    gtk_container_add(container, table);
}

static void
datetime_set_size(Control *control, int size)
{
    g_return_if_fail (control != NULL);

    gtk_widget_set_size_request (control->base, -1, -1);
}

static void
datetime_set_orientation(Control *control, int orientation)
{
    DatetimePlugin *datetime = control->data;

    datetime->orientation = orientation;
}

/*  Date panel control
 *  -------------------
*/
static gboolean
create_datetime_control (Control * control)
{
    DatetimePlugin *datetime = datetime_new();

    gtk_container_add (GTK_CONTAINER (control->base), datetime->eventbox);

    control->data = (gpointer) datetime;
    control->with_popup = FALSE;

    gtk_widget_set_size_request (control->base, -1, -1);

    return TRUE;
}

G_MODULE_EXPORT void
xfce_control_class_init (ControlClass * cc)
{
    cc->name = "datetime";
    cc->caption = "Date and Time";

    cc->create_control = (CreateControlFunc) create_datetime_control;

    cc->free = datetime_free;
    cc->read_config = datetime_read_config;
    cc->write_config = datetime_write_config;
    cc->attach_callback = datetime_attach_callback;

    cc->create_options = datetime_create_options;

    cc->set_orientation = datetime_set_orientation;

    cc->set_size = datetime_set_size;
}

/* macro defined in plugins.h */
XFCE_PLUGIN_CHECK_INIT

// vim: set ts=8 sw=4 :
