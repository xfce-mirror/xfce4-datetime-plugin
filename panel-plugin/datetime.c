/*  $Id$
 *
 *  Copyright (C) 2003 Choe Hwanjin(krisna@kldp.org)
 *  Copyright (c) 2006 Remco den Breeje <remco@sx.mine.nu>
 *  Copyright (c) 2008 Diego Ongaro <ongardie@gmail.com>
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
#include <libxfce4panel/xfce-panel-convenience.h>

#include "datetime.h"
#include "datetime-dialog.h"

#define USE_GTK_TOOLTIP_API     GTK_CHECK_VERSION(2,12,0)

#define DATETIME_MAX_STRLEN 256

/**
 *  Convert a GTimeVal to milliseconds.
 *  Fractions of a millisecond are truncated.
 *  With a 32 bit word size and some values of t.tv_sec,
 *  multiplication by 1000 could overflow a glong,
 *  so the value is cast to a gint64.
 */
static inline gint64 datetime_gtimeval_to_ms(const GTimeVal t)
{
  return ((gint64) t.tv_sec * 1000) + ((gint64) t.tv_usec / 1000);
}

/*
 * Get date/time string
 */
gchar * datetime_do_utf8strftime(const char *format, const struct tm *tm)
{
  int len;
  gchar buf[DATETIME_MAX_STRLEN];
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

/**
 *  Check whether date/time format shows seconds
 */
static gboolean datetime_format_has_seconds(const gchar *format)
{
  static struct tm time_struct = {
    .tm_sec   = 0,
    .tm_min   = 0,
    .tm_hour  = 0,
    .tm_mday  = 1,
    .tm_mon   = 0,
    .tm_year  = 70, /* use 1970 so strftime() can convert '%s' */
    .tm_wday  = 0,
    .tm_yday  = 0,
    .tm_isdst = 0
  };
  int len1;
  int len2;
  gchar buf1[DATETIME_MAX_STRLEN];
  gchar buf2[DATETIME_MAX_STRLEN];

  time_struct.tm_sec = 1;
  len1 = strftime(buf1, sizeof(buf1)-1, format, &time_struct);
  if (len1 == 0)
    return FALSE;
  buf1[len1] = '\0';

  time_struct.tm_sec = 2;
  len2 = strftime(buf2, sizeof(buf2)-1, format, &time_struct);
  if (len2 == 0)
    return FALSE;
  buf2[len2] = '\0';

  return len1 != len2 || strcmp(buf1, buf2) != 0;
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
  guint wake_interval;  /* milliseconds to next update */

  if (data == NULL)
  {
    return FALSE;
  }

  datetime = (t_datetime*)data;

  /* stop timer */
  if (datetime->timeout_id)
  {
    g_source_remove(datetime->timeout_id);
  }

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
      gtk_widget_hide(GTK_WIDGET(datetime->time_label));
      break;
    case LAYOUT_TIME:
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

  /*
   * Compute the time to the next update and start the timer.
   * The wake interval is the time remaining
   * to the next larger integral multiple of the update interval.
   * Setting the timer to this value schedules the next update
   * to occur on the next second or minute
   * when the update interval is 1 or 60 seconds, respectively.
   */
  wake_interval = datetime->update_interval - datetime_gtimeval_to_ms(timeval) % datetime->update_interval;
  datetime->timeout_id = g_timeout_add(wake_interval, datetime_update, datetime);

  return TRUE;
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

static gboolean close_calendar_window(t_datetime *datetime)
{
  gtk_widget_destroy(datetime->cal);
  datetime->cal = NULL;

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(datetime->button), FALSE);

  return TRUE;
}

/*
 * call the gtk calendar
 */
static GtkWidget * pop_calendar_window(t_datetime *datetime, int orientation)
{
  GtkWidget *window;
  GtkWidget *frame;
  GtkWidget *cal;
  GtkWidget *parent = datetime->button;
  GdkScreen *screen;
  GtkCalendarDisplayOptions display_options;
  int num;

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
  gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), TRUE);
  gtk_window_set_skip_pager_hint(GTK_WINDOW(window), TRUE);
  gtk_window_stick(GTK_WINDOW(window));
  g_object_set_data(G_OBJECT(window), "calendar-parent", parent);

  /* set screen number */
  screen = gtk_widget_get_screen(parent);
  num = gdk_screen_get_monitor_at_window(screen, parent->window);
  gtk_window_set_screen(GTK_WINDOW(window), screen);

  frame = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type (GTK_FRAME(frame), GTK_SHADOW_OUT);
  gtk_container_add (GTK_CONTAINER(window), frame);

  cal = gtk_calendar_new();
  display_options = GTK_CALENDAR_SHOW_HEADING |
    GTK_CALENDAR_SHOW_WEEK_NUMBERS |
    GTK_CALENDAR_SHOW_DAY_NAMES;
  gtk_calendar_display_options(GTK_CALENDAR (cal), display_options);
  gtk_container_add (GTK_CONTAINER(frame), cal);

  g_signal_connect_after(G_OBJECT(window), "realize",
      G_CALLBACK(on_calendar_realized),
      GINT_TO_POINTER(orientation));
  g_signal_connect_swapped(G_OBJECT(window), "delete-event",
      G_CALLBACK(close_calendar_window),
      datetime);
  gtk_widget_show_all(window);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(datetime->button), TRUE);

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

  if (event->button != 1 || event->state & GDK_CONTROL_MASK)
    return FALSE;

  if (datetime == NULL)
    return FALSE;

  if (datetime->cal != NULL)
  {
    close_calendar_window(datetime);
  }
  else
  {
    /* get orientation before drawing the calendar */
    orientation = xfce_panel_plugin_get_orientation(datetime->plugin);

    /* draw calendar */
    datetime->cal = pop_calendar_window(datetime,
                                        orientation);
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

  if (datetime_format_has_seconds(datetime->date_format) ||
      datetime_format_has_seconds(datetime->time_format))
  {
    datetime->update_interval = 1000; /* 1 second */
  }
  else
  {
    datetime->update_interval = 60000; /* 1 minute */
  }
}

/*
 * Function only called by the signal handler.
 */
static int datetime_set_size(XfcePanelPlugin *plugin,
    gint size,
    t_datetime *datetime)
{
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
      layout      = xfce_rc_read_int_entry(rc, "layout", layout);
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
  /* create button */
  datetime->button = xfce_create_panel_toggle_button();

  /* create vertical box */
  datetime->vbox = gtk_vbox_new(TRUE, 0);
  gtk_container_add(GTK_CONTAINER(datetime->button), datetime->vbox);

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

  /* connect widget signals to functions */
  g_signal_connect(datetime->button, "button-press-event",
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
  gtk_widget_show_all(datetime->button);

  /* set date and time labels */
  datetime->timeout_id = 0;
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
  gtk_widget_destroy(datetime->button);

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
  gtk_container_add(GTK_CONTAINER(plugin), datetime->button);
  xfce_panel_plugin_add_action_widget(plugin, datetime->button);

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

