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

#ifndef DATETIME_H
#define DATETIME_H

/* enums */
enum {
  DATE = 0,
  TIME
};

/* typedefs */
typedef enum
{
  LAYOUT_DATE = 0,
  LAYOUT_TIME,
  LAYOUT_DATE_TIME,
  LAYOUT_TIME_DATE,
  LAYOUT_COUNT
} t_layout;

typedef struct {
  XfcePanelPlugin * plugin;
  GtkWidget *eventbox;
  GtkWidget *frame;
  GtkWidget *vbox;
  GtkWidget *date_label;
  GtkWidget *time_label;
  guint timeout_id;

  /* settings */
  gchar *date_font;
  gchar *time_font;
  gchar *date_format;
  gchar *time_format;
  t_layout layout;

  /* option widgets */
  GtkWidget *date_font_selector;
  GtkWidget *date_format_combobox;
  GtkWidget *date_format_entry;
  GtkWidget *time_font_selector;
  GtkWidget *time_format_combobox;
  GtkWidget *time_format_entry;

  /* popup calendar */
  GtkWidget *cal;
} t_datetime;

gboolean
datetime_update(gpointer data);

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

