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

#ifndef _DATETIME_DIALOG_H
#define _DATETIME_DIALOG_H	1

#include "datetime.h"

static void datetime_font_selection_cb(GtkWidget *widget, t_datetime *dt);
static gboolean datetime_entry_change_cb(GtkWidget *widget, GdkEventFocus *ev,
								t_datetime *dt);
static void datetime_dialog_response(GtkWidget *dlg, int foo, t_datetime *dt);
void datetime_properties_dialog(XfcePanelPlugin *plugin, t_datetime * datetime);

#endif /* datetime-dialog.h */

