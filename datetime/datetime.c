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

#include <libxfce4util/i18n.h>
#include <libxfce4util/debug.h>

#include <panel/global.h>
#include <panel/controls.h>
#include <panel/icons.h>
#include <panel/plugins.h>

enum {
    LAYOUT_HORIZONTAL,
    LAYOUT_VERTICAL
};

typedef struct {
    GtkWidget *eventbox;
    GtkWidget *date_label;
    GtkWidget *time_label;
    gchar *date_font;
    gchar *time_font;
    gchar *date_format;
    gchar *time_format;
    guint timeout_id;
    gint orientation;
    gint layout;
    gboolean use_xfcalendar;
    gboolean week_start_monday;

    /* option widgets */
    GtkWidget *date_font_selector;
    GtkWidget *date_format_entry;
    GtkWidget *time_font_selector;
    GtkWidget *time_format_entry;
    GtkWidget *week_start_button;

    /* popup calendar */
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

    g_get_current_time(&timeval);
    current = localtime((time_t *)&timeval.tv_sec);
    if (datetime->date_format != NULL &&
	GTK_IS_LABEL(datetime->date_label)) {
	len = strftime(buf, sizeof(buf) - 1, datetime->date_format, current);
	if (len != 0) {
	    buf[len] = '\0';  /* make sure nul terminated string */
	    utf8str = g_locale_to_utf8(buf, len, NULL, NULL, NULL);
	    if (utf8str != NULL) {
		gtk_label_set_text(GTK_LABEL(datetime->date_label), utf8str);
		g_free(utf8str);
	    }
	} else 
	    gtk_label_set_text(GTK_LABEL(datetime->date_label), _("Error"));
    }

    if (datetime->time_format != NULL &&
	GTK_IS_LABEL(datetime->time_label)) {
	len = strftime(buf, sizeof(buf) - 1, datetime->time_format, current);
	if (len != 0) {
	    buf[len] = '\0';  /* make sure nul terminated string */
	    utf8str = g_locale_to_utf8(buf, len, NULL, NULL, NULL);
	    if (utf8str != NULL) {
		gtk_label_set_text(GTK_LABEL(datetime->time_label), utf8str);
		g_free(utf8str);
	    }
	} else 
	    gtk_label_set_text(GTK_LABEL(datetime->time_label), _("Error"));
    }

    return TRUE;
}

static void
on_calendar_entry_activated(GtkWidget *widget, GtkWidget *calendar)
{
    GDate *date;
    const gchar *text;

    date = g_date_new();

    text = gtk_entry_get_text(GTK_ENTRY(widget));
    g_date_set_parse(date, text);

    if (g_date_valid(date)) {
	gtk_calendar_freeze(GTK_CALENDAR(calendar));
	gtk_calendar_select_month(GTK_CALENDAR(calendar),
				  g_date_get_month(date) - 1,
				  g_date_get_year(date));
	gtk_calendar_select_day(GTK_CALENDAR(calendar),
				g_date_get_day(date));
	gtk_calendar_thaw(GTK_CALENDAR(calendar));
    }
    g_date_free(date);
}

static void
on_calendar_realized(GtkWidget *widget, gpointer data)
{
    gint parent_x, parent_y, parent_w, parent_h;
    gint root_w, root_h;
    gint width, height, x, y;
    gint orientation;
    GdkScreen *screen;
    GtkWidget *parent;
    GtkRequisition requisition;

    orientation = GPOINTER_TO_INT(data);
    parent = g_object_get_data(G_OBJECT(widget), "calendar-parent");

    gdk_window_get_origin(GDK_WINDOW(parent->window), &parent_x, &parent_y);
    gdk_drawable_get_size(GDK_DRAWABLE(parent->window), &parent_w, &parent_h);

    screen = gdk_drawable_get_screen(GDK_DRAWABLE(widget->window));
    root_w = gdk_screen_get_width(GDK_SCREEN(screen));
    root_h = gdk_screen_get_height(GDK_SCREEN(screen));

    gtk_widget_size_request(GTK_WIDGET(widget), &requisition);
    width = requisition.width;
    height = requisition.height;

    /*
    g_print("parent: %dx%d +%d+%d\n", parent_w, parent_h, parent_x, parent_y);
    g_print("root: %dx%d\n", root_w, root_h);
    g_print("calendar: %dx%d\n", width, height);
    */

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

    gtk_window_move(GTK_WINDOW(widget), x, y);
}

static GtkWidget *
pop_calendar_window(GtkWidget *parent,
		    int orientation,
		    gboolean week_start_monday,
		    const char *date_string)
{
    GtkWidget *window;
    GtkWidget *frame;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *cal;
    GtkWidget *entry;
    GtkWidget *label;
    GtkCalendarDisplayOptions display_options;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), TRUE);
    gtk_window_set_skip_pager_hint(GTK_WINDOW(window), TRUE);
    g_object_set_data(G_OBJECT(window), "calendar-parent", parent);

    frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type (GTK_FRAME(frame), GTK_SHADOW_OUT);
    gtk_container_add (GTK_CONTAINER(window), frame);

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add (GTK_CONTAINER(frame), vbox);

    cal = gtk_calendar_new();
    display_options = GTK_CALENDAR_SHOW_HEADING |
		      GTK_CALENDAR_SHOW_WEEK_NUMBERS |
		      GTK_CALENDAR_SHOW_DAY_NAMES;
    if (week_start_monday)
	display_options |= GTK_CALENDAR_WEEK_START_MONDAY;
    gtk_calendar_display_options(GTK_CALENDAR (cal), display_options);
    gtk_box_pack_start(GTK_BOX(vbox), cal, TRUE, TRUE, 0);

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    label = gtk_label_new(_("Date:"));
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

    entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry), date_string);
    gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);
    g_signal_connect(G_OBJECT(entry), "activate",
		     G_CALLBACK(on_calendar_entry_activated), cal);

    g_signal_connect_after(G_OBJECT(window), "realize",
			   G_CALLBACK(on_calendar_realized),
			   GINT_TO_POINTER(orientation));
    gtk_widget_show_all(window);

    return window;
}

static gboolean
on_button_press_event_cb(GtkWidget *widget,
			 GdkEventButton *event,
			 DatetimePlugin *datetime)
{
    if (event->button != 1)
	return FALSE;

    if (datetime == NULL)
	return FALSE;

    if (datetime->use_xfcalendar) {
	/* popup XFCalendar */
    } else {
	if (datetime->cal != NULL) {
	    gtk_widget_destroy(datetime->cal);
	    datetime->cal = NULL;
	} else {
	    datetime->cal = pop_calendar_window(datetime->eventbox,
						datetime->orientation,
						datetime->week_start_monday,
		       gtk_label_get_text(GTK_LABEL(datetime->date_label)));
	}
    }

    return TRUE;
}

static void
datetime_update_date_font(DatetimePlugin *datetime)
{
    PangoFontDescription *font;
    font = pango_font_description_from_string(datetime->date_font);
    gtk_widget_modify_font(datetime->date_label, font);
}

static void
datetime_update_time_font(DatetimePlugin *datetime)
{
    PangoFontDescription *font;
    font = pango_font_description_from_string(datetime->time_font);
    gtk_widget_modify_font(datetime->time_label, font);
}

static void
datetime_apply_font(DatetimePlugin *datetime,
		    const gchar *date_font_name,
		    const gchar *time_font_name)
{
    if (date_font_name != NULL) {
	g_free(datetime->date_font);
	datetime->date_font = g_strdup(date_font_name);
	datetime_update_date_font(datetime);
    }

    if (time_font_name != NULL) {
	g_free(datetime->time_font);
	datetime->time_font = g_strdup(time_font_name);
	datetime_update_time_font(datetime);
    }
}

static void
datetime_apply_format(DatetimePlugin *datetime,
		      const char *date_format,
		      const char *time_format)
{
    if (datetime == NULL)
	return;

    if (date_format != NULL) {
	g_free(datetime->date_format);
	datetime->date_format = g_strcompress(date_format);
    }

    if (time_format != NULL) {
	g_free(datetime->time_format);
	datetime->time_format = g_strcompress(time_format);
    }

    if (datetime->timeout_id)
	g_source_remove(datetime->timeout_id);

    if (strstr(datetime->date_format, "%S") != NULL ||
	strstr(datetime->date_format, "%s") != NULL ||
	strstr(datetime->date_format, "%r") != NULL ||
	strstr(datetime->date_format, "%T") != NULL ||
        strstr(datetime->time_format, "%S") != NULL ||
	strstr(datetime->time_format, "%s") != NULL ||
	strstr(datetime->time_format, "%r") != NULL ||
	strstr(datetime->time_format, "%T") != NULL)
	datetime->timeout_id = g_timeout_add(1000, datetime_update, datetime);
    else
	datetime->timeout_id = g_timeout_add(10000, datetime_update, datetime);
}

static void
create_main_widget (DatetimePlugin *datetime)
{
    GtkWidget *box;
    GtkWidget *align;

    datetime->eventbox = gtk_event_box_new();
    g_signal_connect(G_OBJECT(datetime->eventbox), "button-press-event",
	    	     G_CALLBACK(on_button_press_event_cb), datetime);

    align = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(datetime->eventbox), align);

    if (datetime->layout == LAYOUT_VERTICAL)
	box = gtk_vbox_new(FALSE, 0);
    else
	box = gtk_hbox_new(FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(box), border_width);
    gtk_container_add(GTK_CONTAINER(align), box);

    datetime->time_label = gtk_label_new("");
    gtk_label_set_justify(GTK_LABEL(datetime->time_label), GTK_JUSTIFY_CENTER);
    datetime->date_label = gtk_label_new("");
    gtk_label_set_justify(GTK_LABEL(datetime->date_label), GTK_JUSTIFY_CENTER);

    if (datetime->layout == LAYOUT_VERTICAL) {
	gtk_box_pack_start(GTK_BOX(box), datetime->time_label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), datetime->date_label, FALSE, FALSE, 0);
    } else {
	gtk_box_pack_start(GTK_BOX(box), datetime->date_label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), datetime->time_label, FALSE, FALSE, 5);
    }
}

static DatetimePlugin *
datetime_new (void)
{
    DatetimePlugin *datetime;

    datetime = g_new (DatetimePlugin, 1);
    datetime->orientation = GTK_ORIENTATION_HORIZONTAL;
    datetime->layout = LAYOUT_VERTICAL;
    datetime->date_font = NULL;
    datetime->date_format = NULL;
    datetime->time_font = NULL;
    datetime->time_format = NULL;
    datetime->use_xfcalendar = FALSE;
    datetime->week_start_monday = FALSE;

    create_main_widget(datetime);

    /* calendar */
    datetime->cal = NULL;

    datetime_apply_font(datetime,
			"Bitstream Vera Sans 9",
			"Bitstream Vera Sans 12");
    /* This is default date/time format, (See strftime(3))
       translaters may change this to familiar format for their language */
    datetime_apply_format(datetime, _("%Y-%m-%d"), _("%H:%M"));

    datetime_update(datetime);

    gtk_widget_show_all(datetime->eventbox);

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
    if (node == NULL)
	return;

    while (node != NULL) {
	if (xmlStrEqual(node->name, (const xmlChar *)"Date")) {
	    xmlNodePtr tmp = node->children;
	    while (tmp != NULL) {
		if (xmlStrEqual(tmp->name, (const xmlChar *)"Font")) {
		    value = xmlNodeListGetString(xmlconfig,
						 tmp->children, 1);
		    if (value != NULL) {
			datetime_apply_font(datetime, value, NULL);
			xmlFree(value);
		    }
		} else if (xmlStrEqual(tmp->name, (const xmlChar *)"Format")) {
		    value = xmlNodeListGetString(xmlconfig, tmp->children, 1);
		    if (value != NULL) {
			datetime_apply_format(datetime, value, NULL);
			xmlFree(value);
		    }
		}
		tmp = tmp->next;
	    }
	} else if (xmlStrEqual(node->name, (const xmlChar *)"Time")) {
	    xmlNodePtr tmp = node->children;
	    while (tmp != NULL) {
		if (xmlStrEqual(tmp->name, (const xmlChar *)"Font")) {
		    value = xmlNodeListGetString(xmlconfig, tmp->children, 1);
		    if (value != NULL) {
			datetime_apply_font(datetime, NULL, value);
			xmlFree(value);
		    }
		} else if (xmlStrEqual(tmp->name, (const xmlChar *)"Format")) {
		    value = xmlNodeListGetString(xmlconfig, tmp->children, 1);
		    if (value != NULL) {
			datetime_apply_format(datetime, NULL, value);
			xmlFree(value);
		    }
		}
		tmp = tmp->next;
	    }
	} else if (xmlStrEqual(node->name, (const xmlChar *)"Calendar")) {
	    value = xmlGetProp(node, (const xmlChar *)"UseXFCalendar");
	    if (g_ascii_strcasecmp("true", value) == 0)
		datetime->use_xfcalendar = TRUE;
	    else
		datetime->use_xfcalendar = FALSE;
	    value = xmlGetProp(node, (const xmlChar *)"WeekStartsMonday");
	    if (g_ascii_strcasecmp("true", value) == 0)
		datetime->week_start_monday = TRUE;
	    else
		datetime->week_start_monday = FALSE;
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
    xmlNewTextChild(node, NULL, (const xmlChar *)"Font", datetime->date_font);
    format = g_strescape(datetime->date_format, NULL);
    xmlNewTextChild(node, NULL, (const xmlChar *)"Format", format);
    g_free(format);

    node = xmlNewTextChild(parent, NULL, (const xmlChar *)"Time", NULL);
    xmlNewTextChild(node, NULL, (const xmlChar *)"Font", datetime->time_font);
    format = g_strescape(datetime->time_format, NULL);
    xmlNewTextChild(node, NULL, (const xmlChar *)"Format", format);
    g_free(format);

    node = xmlNewTextChild(parent, NULL, (const xmlChar *)"Calendar", NULL);
    if (datetime->use_xfcalendar)
	xmlSetProp(node, "UseXFCalendar", "true");
    else
	xmlSetProp(node, "UseXFCalendar", "false");
    if (datetime->week_start_monday)
	xmlSetProp(node, "WeekStartsMonday", "true");
    else
	xmlSetProp(node, "WeekStartsMonday", "false");
}

static void
datetime_attach_callback(Control *control, const char *signal,
		     GCallback callback, gpointer data)
{
}

static void
datetime_date_font_selection_cb(GtkWidget *widget, gpointer data)
{
    DatetimePlugin *datetime;
    GtkWidget *dialog;
    gint result;

    g_return_if_fail (data != NULL);

    datetime = (DatetimePlugin*)data;

    dialog = gtk_font_selection_dialog_new(_("Select font"));
    gtk_font_selection_dialog_set_font_name(GTK_FONT_SELECTION_DIALOG(dialog),
	   	 			    datetime->date_font);
    gtk_font_selection_dialog_set_preview_text(GTK_FONT_SELECTION_DIALOG(dialog),
			gtk_label_get_text(GTK_LABEL(datetime->date_label)));

    result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_OK || result == GTK_RESPONSE_ACCEPT) {
	gchar *font_name;
	font_name = gtk_font_selection_dialog_get_font_name(GTK_FONT_SELECTION_DIALOG(dialog));
	if (font_name != NULL) {
	    gtk_button_set_label(GTK_BUTTON(widget), font_name);
	    datetime_apply_font(datetime, font_name, NULL);
	}
    }
    gtk_widget_destroy(dialog);
}

static void
datetime_time_font_selection_cb(GtkWidget *widget, gpointer data)
{
    DatetimePlugin *datetime;
    GtkWidget *dialog;
    gint result;

    g_return_if_fail (data != NULL);

    datetime = (DatetimePlugin*)data;

    dialog = gtk_font_selection_dialog_new(_("Select font"));
    gtk_font_selection_dialog_set_font_name(GTK_FONT_SELECTION_DIALOG(dialog),
	   	 			    datetime->time_font);
    gtk_font_selection_dialog_set_preview_text(GTK_FONT_SELECTION_DIALOG(dialog),
			gtk_label_get_text(GTK_LABEL(datetime->time_label)));

    result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_OK || result == GTK_RESPONSE_ACCEPT) {
	gchar *font_name;
	font_name = gtk_font_selection_dialog_get_font_name(GTK_FONT_SELECTION_DIALOG(dialog));
	if (font_name != NULL) {
	    gtk_button_set_label(GTK_BUTTON(widget), font_name);
	    datetime_apply_font(datetime, NULL, font_name);
	}
    }
    gtk_widget_destroy(dialog);
}

static void
date_entry_activate_cb (GtkWidget *widget, DatetimePlugin *datetime)
{
    const gchar *format;
    format = gtk_entry_get_text(GTK_ENTRY(widget));
    if (format != NULL)
	datetime_apply_format(datetime, format, NULL);
    datetime_update(datetime);
}

static void
time_entry_activate_cb (GtkWidget *widget, DatetimePlugin *datetime)
{
    const gchar *format;
    format = gtk_entry_get_text(GTK_ENTRY(widget));
    if (format != NULL)
	datetime_apply_format(datetime, NULL, format);
    datetime_update(datetime);
}

static void
xfcalendar_button_toggle_cb (GtkWidget *widget, DatetimePlugin *datetime)
{
    if (datetime == NULL)
	return;

    datetime->use_xfcalendar = 
	gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    gtk_widget_set_sensitive(datetime->week_start_button,
			     !datetime->use_xfcalendar);
}

static void
week_day_button_toggle_cb (GtkWidget *widget, DatetimePlugin *datetime)
{
    if (datetime == NULL)
	return;

    datetime->week_start_monday = 
	gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

static void
apply_options_done_cb(GtkWidget *widget, DatetimePlugin *datetime)
{
    if (datetime == NULL)
	return;

    datetime_apply_format(datetime,
		  gtk_entry_get_text(GTK_ENTRY(datetime->date_format_entry)),
		  gtk_entry_get_text(GTK_ENTRY(datetime->time_format_entry)));
    datetime_update(datetime);
}

static void
datetime_create_options(Control *control,
			GtkContainer *container,
			GtkWidget *done)
{
    DatetimePlugin *datetime;
    GtkWidget *main_vbox;
    GtkWidget *frame;
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *button;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkSizeGroup *sg;
    gchar *format;

    g_return_if_fail (control != NULL);
    g_return_if_fail (container != NULL);
    g_return_if_fail (done != NULL);

    datetime = (DatetimePlugin*)control->data;
    g_return_if_fail (datetime != NULL);

    main_vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(container), main_vbox);

    gtk_widget_show_all(main_vbox);

    sg = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

    /* time */
    frame = xfce_framebox_new(_("Time"), TRUE);
    gtk_box_pack_start(GTK_BOX(main_vbox), frame, TRUE, TRUE, 0);

    vbox = gtk_vbox_new(FALSE, 0);
    xfce_framebox_add(XFCE_FRAMEBOX(frame), vbox);

    hbox = gtk_hbox_new(FALSE, border_width);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    label = gtk_label_new(_("Font:"));
    gtk_misc_set_alignment(GTK_MISC (label), 0, 0.5);
    gtk_size_group_add_widget(sg, label);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

    button = gtk_button_new_with_label(datetime->time_font);
    gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(button), "clicked",
	    	     G_CALLBACK(datetime_time_font_selection_cb), datetime);
    datetime->time_font_selector = button;

    hbox = gtk_hbox_new(FALSE, border_width);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    label = gtk_label_new(_("Format:"));
    gtk_misc_set_alignment(GTK_MISC (label), 0, 0.5);
    gtk_size_group_add_widget(sg, label);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

    entry = gtk_entry_new();
    format = g_strescape(datetime->time_format, NULL);
    gtk_entry_set_text(GTK_ENTRY(entry), format);
    gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 0);
    g_free(format);
    g_signal_connect (G_OBJECT(entry), "activate",
		      G_CALLBACK (time_entry_activate_cb), datetime);
    datetime->time_format_entry = entry;

    /* date */
    frame = xfce_framebox_new(_("Date"), TRUE);
    gtk_box_pack_start(GTK_BOX(main_vbox), frame, TRUE, TRUE, 0);

    vbox = gtk_vbox_new(FALSE, 0);
    xfce_framebox_add(XFCE_FRAMEBOX(frame), vbox);

    hbox = gtk_hbox_new(FALSE, border_width);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    label = gtk_label_new(_("Font:"));
    gtk_misc_set_alignment(GTK_MISC (label), 0, 0.5);
    gtk_size_group_add_widget(sg, label);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

    button = gtk_button_new_with_label(datetime->date_font);
    gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(button), "clicked",
	    	     G_CALLBACK(datetime_date_font_selection_cb), datetime);
    datetime->date_font_selector = button;

    hbox = gtk_hbox_new(FALSE, border_width);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    label = gtk_label_new(_("Format:"));
    gtk_misc_set_alignment(GTK_MISC (label), 0, 0.5);
    gtk_size_group_add_widget(sg, label);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

    entry = gtk_entry_new();
    format = g_strescape(datetime->date_format, NULL);
    gtk_entry_set_text(GTK_ENTRY(entry), format);
    gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 0);
    g_free(format);
    g_signal_connect (G_OBJECT(entry), "activate",
		      G_CALLBACK (date_entry_activate_cb), datetime);
    datetime->date_format_entry = entry;

    /* Calendar options */
    frame = xfce_framebox_new(_("Calendar"), TRUE);
    gtk_box_pack_start(GTK_BOX(main_vbox), frame, TRUE, TRUE, 0);

    vbox = gtk_vbox_new(FALSE, 0);
    xfce_framebox_add(XFCE_FRAMEBOX(frame), vbox);

    button = gtk_check_button_new_with_label(_("use XFCalendar for popup calendar"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),
				 datetime->use_xfcalendar);
    /* On current version, we do not use xfcalendar option
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0); */
    g_signal_connect(G_OBJECT(button), "toggled",
	    	     G_CALLBACK(xfcalendar_button_toggle_cb), datetime);

    button = gtk_check_button_new_with_label(_("Week day starts Monday"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),
				 datetime->week_start_monday);
    if (datetime->use_xfcalendar)
	gtk_widget_set_sensitive(button, FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(button), "toggled",
	    	     G_CALLBACK(week_day_button_toggle_cb), datetime);
    datetime->week_start_button = button;

    g_signal_connect(G_OBJECT(done), "clicked",
		     G_CALLBACK(apply_options_done_cb), datetime);
    gtk_widget_show_all(main_vbox);
}

static void
datetime_set_size(Control *control, int size)
{
    int threshold;
    int new_layout;
    DatetimePlugin *datetime;

    g_return_if_fail (control != NULL && control->data != NULL);

    datetime = control->data;

    if (size < MEDIUM)
	new_layout = LAYOUT_HORIZONTAL;
    else
	new_layout = LAYOUT_VERTICAL;

    if (new_layout != datetime->layout) {
	gtk_widget_destroy(GTK_WIDGET(datetime->eventbox));
	datetime->layout = new_layout;
	create_main_widget(datetime);
	datetime_update_date_font(datetime);
	datetime_update_time_font(datetime);
	datetime_update(datetime);
	gtk_widget_show_all(datetime->eventbox);
	gtk_container_add (GTK_CONTAINER (control->base), datetime->eventbox);
    }

    threshold = icon_size[size];
    if (datetime->orientation == GTK_ORIENTATION_VERTICAL)
	gtk_widget_set_size_request (control->base, threshold, -1);
    else
	gtk_widget_set_size_request (control->base, -1, threshold);
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
    xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

    cc->name = "datetime";
    cc->caption = _("Date and Time");

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
