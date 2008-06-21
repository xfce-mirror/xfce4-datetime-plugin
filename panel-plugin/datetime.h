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

#ifndef DATETIME_H
#define DATETIME_H

#define USE_GTK_TOOLTIP_API     GTK_CHECK_VERSION(2,12,0)

/* enums */
enum {
  DATE = 0,
  TIME
};

/* typedefs */
typedef enum
{
  LAYOUT_DATE_TIME = 0,
  LAYOUT_TIME_DATE,
  LAYOUT_DATE,
  LAYOUT_TIME,
  LAYOUT_COUNT
} t_layout;

typedef struct {
  XfcePanelPlugin * plugin;
  GtkWidget *button;
  GtkWidget *vbox;
  GtkWidget *date_label;
  GtkWidget *time_label;
  guint update_interval;  /* time between updates in milliseconds */
  guint timeout_id;
#if USE_GTK_TOOLTIP_API
  guint tooltip_timeout_id;
  gulong tooltip_handler_id;
#endif

  /* settings */
  gchar *date_font;
  gchar *time_font;
  gchar *date_format;
  gchar *time_format;
  t_layout layout;

  /* option widgets */
  GtkWidget *date_frame;
#if USE_GTK_TOOLTIP_API
  GtkWidget *date_tooltip_label;
#endif
  GtkWidget *date_font_hbox;
  GtkWidget *date_font_selector;
  GtkWidget *date_format_combobox;
  GtkWidget *date_format_entry;
  GtkWidget *time_frame;
#if USE_GTK_TOOLTIP_API
  GtkWidget *time_tooltip_label;
#endif
  GtkWidget *time_font_hbox;
  GtkWidget *time_font_selector;
  GtkWidget *time_format_combobox;
  GtkWidget *time_format_entry;

  /* popup calendar */
  GtkWidget *cal;
} t_datetime;

gboolean
datetime_update(t_datetime *datetime);

gchar *
datetime_do_utf8strftime(
    const char *format,
    const struct tm *tm);

void
datetime_apply_font(t_datetime *datetime,
    const gchar *date_font_name,
    const gchar *time_font_name);

void
datetime_apply_format(t_datetime *datetime,
    const gchar *date_format,
    const gchar *time_format);

void
datetime_apply_layout(t_datetime *datetime,
    t_layout layout);

void
datetime_write_rc_file(XfcePanelPlugin *plugin,
    t_datetime *dt);

#endif /* datetime.h */

