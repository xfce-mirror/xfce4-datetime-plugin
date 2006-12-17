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

enum {
  DATE = 0,
  TIME = 1
};

/* types */
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

  /* option widgets */
  GtkWidget *date_font_selector;
  GtkWidget *date_format_entry;
  GtkWidget *time_font_selector;
  GtkWidget *time_format_entry;

  /* popup calendar */
  GtkWidget *cal;
} t_datetime;

#endif /* datetime.h */
