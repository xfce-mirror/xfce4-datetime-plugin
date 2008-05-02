/*  $Id$
 *
 *  Copyright (C) 2003 Choe Hwanjin(krisna@kldp.org)
 *  Copyright (c) 2006 Remco den Breeje <remco@sx.mine.nu>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published
 *  by the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* local includes */
#include <time.h>
#include <string.h>

/* xfce includes */
#include <libxfcegui4/libxfcegui4.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfce4panel/xfce-panel-plugin.h>

#include "datetime.h"
#include "datetime-dialog.h"

/*
 * Get date/time string
 */
gchar * datetime_do_utf8strftime(const char *format, const struct tm *tm)
{
  int len;
  gchar buf[256];
  gchar *utf8str = NULL;

  /* get formatted date/time */
  len = strftime(buf, sizeof(buf)-1, format, tm);
  if (len == 0)
    return g_strdup(_("Invalid format"));

  buf[len] = '\0';  /* make sure nul terminated string */
  utf8str = g_locale_to_utf8(buf, -1, NULL, NULL, NULL);
  if(utf8str == NULL)
    return g_strdup(_("Error"));

  return utf8str;
}

/*
 * set date and time labels
 */
gboolean datetime_update(gpointer data)
{
  GTimeVal timeval;
  gchar *utf8str;
  struct tm *current;
  t_datetime *datetime;

  if (data == NULL)
  {
    return FALSE;
  }

  datetime = (t_datetime*)data;

  g_get_current_time(&timeval);
  current = localtime((time_t *)&timeval.tv_sec);
  if (datetime->date_format != NULL && GTK_IS_LABEL(datetime->date_label))
  {
    utf8str = datetime_do_utf8strftime(datetime->date_format, current);
    gtk_label_set_text(GTK_LABEL(datetime->date_label), utf8str);
    g_free(utf8str);
  }

  if (datetime->time_format != NULL && GTK_IS_LABEL(datetime->time_label))
  {
    utf8str = datetime_do_utf8strftime(datetime->time_format, current);
    gtk_label_set_text(GTK_LABEL(datetime->time_label), utf8str);
    g_free(utf8str);
  }

  /* hide labels based on layout-selection */
  gtk_widget_show(GTK_WIDGET(datetime->time_label));
  gtk_widget_show(GTK_WIDGET(datetime->date_label));
  switch(datetime->layout)
  {
    case LAYOUT_DATE:
    case LAYOUT_DATE_TIMETT:
      gtk_widget_hide(GTK_WIDGET(datetime->time_label));
      break;
    case LAYOUT_TIME:
    case LAYOUT_TIME_DATETT:
      gtk_widget_hide(GTK_WIDGET(datetime->date_label));
      break;
    default:
      break;
  }

  /* set order based on layout-selection */
  switch(datetime->layout)
  {
    case LAYOUT_DATE_TIME:
      gtk_box_reorder_child(GTK_BOX(datetime->vbox), datetime->time_label, 1);
      gtk_box_reorder_child(GTK_BOX(datetime->vbox), datetime->date_label, 0);
      break;

    default:
      gtk_box_reorder_child(GTK_BOX(datetime->vbox), datetime->time_label, 0);
      gtk_box_reorder_child(GTK_BOX(datetime->vbox), datetime->date_label, 1);
  }

  /* update tooltip */
  switch(datetime->layout)
  {
    case LAYOUT_DATE_TIMETT:
      gtk_tooltips_set_tip(GTK_TOOLTIPS(datetime->tips), datetime->eventbox,
          gtk_label_get_text(GTK_LABEL(datetime->time_label)), NULL);
      break;
    case LAYOUT_TIME_DATETT:
      gtk_tooltips_set_tip(GTK_TOOLTIPS(datetime->tips), datetime->eventbox,
          gtk_label_get_text(GTK_LABEL(datetime->date_label)), NULL);
      break;
    default:
      gtk_tooltips_set_tip(GTK_TOOLTIPS(datetime->tips), datetime->eventbox,
          NULL, NULL);
      break;
  }

  return TRUE;
}


static void on_calendar_entry_activated(GtkWidget *widget, GtkWidget *calendar)
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


static void on_calendar_realized(GtkWidget *widget, gpointer data)
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

  DBG("orientation: %s", (orientation ? "vertical" : "horizontal"));
  DBG("parent: %dx%d +%d+%d", parent_w, parent_h, parent_x, parent_y);
  DBG("root: %dx%d", root_w, root_h);
  DBG("calendar: %dx%d", width, height);

  if (orientation == GTK_ORIENTATION_VERTICAL)
  {
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
  }
  else
  {
    if (parent_x < root_w / 2)
    {
      if (parent_y < root_h / 2)
      {
	/* upper left */
	x = parent_x;
	y = parent_y + parent_h;
      }
      else
      {
	/* lower left */
	x = parent_x;
	y = parent_y - height;
      }
    }
    else
    {
      if (parent_y < root_h / 2)
      {
	/* upper right */
	x = parent_x + parent_w - width;
	y = parent_y + parent_h;
      }
      else
      {
	/* lower right */
	x = parent_x + parent_w - width;
	y = parent_y - height;
      }
    }
  }
  gtk_window_move(GTK_WINDOW(widget), x, y);
}


/*
 * call the gtk calendar
 */
static GtkWidget * pop_calendar_window(GtkWidget *parent,
    int orientation,
    const char *date_string)
{
  GtkWidget *window;
  GtkWidget *frame;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *cal;
  GtkWidget *entry;
  GtkWidget *label;
  GdkScreen *screen;
  GtkCalendarDisplayOptions display_options;
  int num;

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
  gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), TRUE);
  gtk_window_set_skip_pager_hint(GTK_WINDOW(window), TRUE);
  g_object_set_data(G_OBJECT(window), "calendar-parent", parent);

  /* set screen number */
  screen = gtk_widget_get_screen(parent);
  num = gdk_screen_get_monitor_at_window(screen, parent->window);
  gtk_window_set_screen(GTK_WINDOW(window), screen);

  frame = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type (GTK_FRAME(frame), GTK_SHADOW_OUT);
  gtk_container_add (GTK_CONTAINER(window), frame);

  vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add (GTK_CONTAINER(frame), vbox);

  cal = gtk_calendar_new();
  display_options = GTK_CALENDAR_SHOW_HEADING |
    GTK_CALENDAR_SHOW_WEEK_NUMBERS |
    GTK_CALENDAR_SHOW_DAY_NAMES;
  gtk_calendar_display_options(GTK_CALENDAR (cal), display_options);
  gtk_box_pack_start(GTK_BOX(vbox), cal, TRUE, TRUE, 0);

  hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

  label = gtk_label_new(_("Date:"));
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 5);

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

/*
 * when the dialog is clicked, show the calendar
 */
static gboolean datetime_clicked(GtkWidget *widget,
    GdkEventButton *event,
    t_datetime *datetime)
{
  gint orientation;

  if (event->button != 1)
    return FALSE;

  if (datetime == NULL)
    return FALSE;

  if (datetime->cal != NULL)
  {
    gtk_widget_destroy(datetime->cal);
    datetime->cal = NULL;
  }
  else
  {
    /* get orientation before drawing the calendar */
    orientation = xfce_panel_plugin_get_orientation(datetime->plugin);

    /* draw calendar */
    datetime->cal = pop_calendar_window(datetime->eventbox,
	                                orientation,
	                                gtk_label_get_text(GTK_LABEL(datetime->date_label)));
  }
  return TRUE;
}

static void datetime_update_date_font(t_datetime *datetime)
{
  PangoFontDescription *font;
  font = pango_font_description_from_string(datetime->date_font);

  if (G_LIKELY (font))
  {
    gtk_widget_modify_font(datetime->date_label, font);
    pango_font_description_free (font);
  }
}

static void datetime_update_time_font(t_datetime *datetime)
{
  PangoFontDescription *font;
  font = pango_font_description_from_string(datetime->time_font);

  if (G_LIKELY (font))
  {
    gtk_widget_modify_font(datetime->time_label, font);
    pango_font_description_free (font);
  }
}

/*
 * set layout after doing some checks
 */
void datetime_apply_layout(t_datetime *datetime, t_layout layout)
{
  if(0 <= layout && layout < LAYOUT_COUNT)
  {
    datetime->layout = layout;
  }
}

/*
 * set the date and time font type
 */
void datetime_apply_font(t_datetime *datetime,
    const gchar *date_font_name,
    const gchar *time_font_name)
{
  if (date_font_name != NULL)
  {
    g_free(datetime->date_font);
    datetime->date_font = g_strdup(date_font_name);
    datetime_update_date_font(datetime);
  }

  if (time_font_name != NULL)
  {
    g_free(datetime->time_font);
    datetime->time_font = g_strdup(time_font_name);
    datetime_update_time_font(datetime);
  }
}

/*
 * set the date and time format
 */
void datetime_apply_format(t_datetime *datetime,
    const gchar *date_format,
    const gchar *time_format)
{
  if (datetime == NULL)
    return;

  if (date_format != NULL)
  {
    g_free(datetime->date_format);
    datetime->date_format = g_strdup(date_format);
    if (strlen(date_format) == 0)
      gtk_widget_hide(GTK_WIDGET(datetime->date_label));
    else
      gtk_widget_show(GTK_WIDGET(datetime->date_label));
  }

  if (time_format != NULL)
  {
    g_free(datetime->time_format);
    datetime->time_format = g_strdup(time_format);
    if (strlen(time_format) == 0)
      gtk_widget_hide(GTK_WIDGET(datetime->time_label));
    else
      gtk_widget_show(GTK_WIDGET(datetime->time_label));
  }

  if (datetime->timeout_id)
  {
    g_source_remove(datetime->timeout_id);
  }

  if (strstr(datetime->date_format, "%S") != NULL ||
      strstr(datetime->date_format, "%s") != NULL ||
      strstr(datetime->date_format, "%r") != NULL ||
      strstr(datetime->date_format, "%T") != NULL ||
      strstr(datetime->time_format, "%S") != NULL ||
      strstr(datetime->time_format, "%s") != NULL ||
      strstr(datetime->time_format, "%r") != NULL ||
      strstr(datetime->time_format, "%T") != NULL)
  {
    datetime->timeout_id = g_timeout_add(1000, datetime_update, datetime);
  }
  else
  {
    datetime->timeout_id = g_timeout_add(10000, datetime_update, datetime);
  }
}

/*
 * Set a border - Function only called by the signal handler.
 * A border is only set, if there is enough space for it (size > 26)
 */
static int datetime_set_size(XfcePanelPlugin *plugin,
    gint size,
    t_datetime *datetime)
{
  if(size > 26)
    gtk_container_set_border_width(GTK_CONTAINER(datetime->frame), 2);
  else
    gtk_container_set_border_width(GTK_CONTAINER(datetime->frame), 0);

  /* return true to please the signal handler ;) */
  return TRUE;
}

/*
 * Read the settings from the config file
 */
static void datetime_read_rc_file(XfcePanelPlugin *plugin, t_datetime *dt)
{
  gchar *file;
  XfceRc *rc;
  t_layout layout;
  const gchar *date_font, *time_font, *date_format, *time_format;

  /* load defaults */
  layout = LAYOUT_DATE_TIME;
  date_font = "Bitstream Vera Sans 8";
  time_font = "Bitstream Vera Sans 10";
  date_format = "%Y/%m/%d";
  time_format = "%H:%M";

  /* open file */
  if((file = xfce_panel_plugin_lookup_rc_file(plugin)) != NULL)
  {
    rc = xfce_rc_simple_open(file, TRUE);
    g_free(file);

    if(rc != NULL)
    {
      layout	  = xfce_rc_read_int_entry(rc, "layout", layout);
      date_font   = xfce_rc_read_entry(rc, "date_font", date_font);
      time_font   = xfce_rc_read_entry(rc, "time_font", time_font);
      date_format = xfce_rc_read_entry(rc, "date_format", date_format);
      time_format = xfce_rc_read_entry(rc, "time_format", time_format);

      xfce_rc_close(rc);
    }
  }

  date_font   = g_strdup(date_font);
  time_font   = g_strdup(time_font);
  date_format = g_strdup(date_format);
  time_format = g_strdup(time_format);

  /* set values in dt struct */
  datetime_apply_layout(dt, layout);
  datetime_apply_font(dt, date_font, time_font);
  datetime_apply_format(dt, date_format, time_format);
}

/*
 * write the settings to the config file
 */
void datetime_write_rc_file(XfcePanelPlugin *plugin, t_datetime *dt)
{
  char *file;
  XfceRc *rc;

  if(!(file = xfce_panel_plugin_save_location(plugin, TRUE)))
    return;

  rc = xfce_rc_simple_open(file, FALSE);
  g_free(file);

  if(rc != NULL)
  {
    xfce_rc_write_int_entry(rc, "layout", dt->layout);
    xfce_rc_write_entry(rc, "date_font", dt->date_font);
    xfce_rc_write_entry(rc, "time_font", dt->time_font);
    xfce_rc_write_entry(rc, "date_format", dt->date_format);
    xfce_rc_write_entry(rc, "time_format", dt->time_format);

    xfce_rc_close(rc);
  }

}

/*
 * create the gtk-part of the datetime plugin
 */
static void datetime_create_widget(t_datetime * datetime)
{
  /* create event box */
  datetime->eventbox = gtk_event_box_new();

  /* create frame */
  datetime->frame = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type(GTK_FRAME(datetime->frame), GTK_SHADOW_NONE);
  gtk_container_add(GTK_CONTAINER(datetime->eventbox), datetime->frame);

  /* create vertical box */
  datetime->vbox = gtk_vbox_new(TRUE, 0);
  gtk_container_add(GTK_CONTAINER(datetime->frame), datetime->vbox);

  /* create time and date lines */
  datetime->time_label = gtk_label_new("");
  datetime->date_label = gtk_label_new("");
  gtk_label_set_justify(GTK_LABEL(datetime->time_label), GTK_JUSTIFY_CENTER);
  gtk_label_set_justify(GTK_LABEL(datetime->date_label), GTK_JUSTIFY_CENTER);

  /* add time and date lines to the vbox */
  gtk_box_pack_start(GTK_BOX(datetime->vbox),
      datetime->time_label, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(datetime->vbox),
      datetime->date_label, FALSE, FALSE, 0);
  gtk_box_reorder_child(GTK_BOX(datetime->vbox), datetime->time_label, 0);
  gtk_box_reorder_child(GTK_BOX(datetime->vbox), datetime->date_label, 1);

  /* create tooltips */
  datetime->tips = gtk_tooltips_new ();

  /* connect widget signals to functions */
  g_signal_connect(datetime->eventbox, "button-press-event",
      G_CALLBACK(datetime_clicked), datetime);
}

/*
 * create datetime plugin
 */
static t_datetime * datetime_new(XfcePanelPlugin *plugin)
{
  t_datetime * datetime;

  DBG("Starting datetime panel plugin");

  /* alloc mem */
  datetime = panel_slice_new0 (t_datetime);

  /* set variables */
  datetime->plugin = plugin;
  datetime->date_font = NULL;
  datetime->date_format = NULL;
  datetime->time_font = NULL;
  datetime->time_format = NULL;

  /* call widget-create function */
  datetime_create_widget(datetime);

  /* set calendar variables */
  datetime->cal = NULL;

  /* load settings (default values if non-av) */
  datetime_read_rc_file(plugin, datetime);

  /* display plugin */
  gtk_widget_show_all(datetime->eventbox);

  /* set date and time labels */
  datetime_update(datetime);

  return datetime;
}

/*
 * frees the datetime struct
 */
static void datetime_free(XfcePanelPlugin *plugin, t_datetime *datetime)
{
  /* stop timeout */
  g_source_remove(datetime->timeout_id);

  /* destroy widget */
  gtk_widget_destroy(datetime->eventbox);

  /* cleanup */
  g_free(datetime->date_font);
  g_free(datetime->time_font);
  g_free(datetime->date_format);
  g_free(datetime->time_format);

  panel_slice_free(t_datetime, datetime);
}

/*
 * Construct the plugin
 */
static void datetime_construct(XfcePanelPlugin *plugin)
{
  /* create datetime plugin */
  t_datetime * datetime = datetime_new(plugin);

  /* add plugin to panel */
  gtk_container_add(GTK_CONTAINER(plugin), datetime->eventbox);
  xfce_panel_plugin_add_action_widget(plugin, datetime->eventbox);

  /* connect plugin signals to functions */
  g_signal_connect(plugin, "save",
      G_CALLBACK(datetime_write_rc_file), datetime);
  g_signal_connect(plugin, "free-data",
      G_CALLBACK(datetime_free), datetime);
  g_signal_connect(plugin, "size-changed",
      G_CALLBACK (datetime_set_size), datetime);
  g_signal_connect(plugin, "configure-plugin",
      G_CALLBACK (datetime_properties_dialog), datetime);
  xfce_panel_plugin_menu_show_configure(plugin);
}


XFCE_PANEL_PLUGIN_REGISTER_INTERNAL(datetime_construct);

