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
 * show and read fonts and inform datetime about it
 */
static void datetime_font_selection_cb(GtkWidget *widget, t_datetime *dt)
{
  GtkWidget *dialog;
  gchar *fontname;
  const gchar *previewtext;
  gint target, result;
  gchar *font_name;

  if(widget == dt->date_font_selector)
  {
    target = DATE;
    fontname = dt->date_font;
    previewtext = gtk_label_get_text(GTK_LABEL(dt->date_label));
  }
  else /*time_font_selector */
  {
    target = TIME;
    fontname = dt->time_font;
    previewtext = gtk_label_get_text(GTK_LABEL(dt->time_label));
  }

  dialog = gtk_font_selection_dialog_new(_("Select font"));
  gtk_font_selection_dialog_set_font_name(GTK_FONT_SELECTION_DIALOG(dialog),
					  fontname);

  if (G_LIKELY (previewtext != NULL))
  {
    gtk_font_selection_dialog_set_preview_text(GTK_FONT_SELECTION_DIALOG(dialog),
					       previewtext);
  }

  result = gtk_dialog_run(GTK_DIALOG(dialog));
  if (result == GTK_RESPONSE_OK || result == GTK_RESPONSE_ACCEPT)
  {
    font_name =
      gtk_font_selection_dialog_get_font_name(GTK_FONT_SELECTION_DIALOG(dialog));

    if (font_name != NULL)
    {
      gtk_button_set_label(GTK_BUTTON(widget), font_name);

      if(target == DATE)
	datetime_apply_font(dt, font_name, NULL);
      else
	datetime_apply_font(dt, NULL, font_name);

      g_free (font_name);
    }
  }
  gtk_widget_destroy(dialog);
}

/*
 * read values from date and time entry and inform datetime about it
 */
static gboolean datetime_entry_change_cb(GtkWidget *widget, GdkEventFocus *ev,
								t_datetime *dt)
{
  const gchar *format;
  format = gtk_entry_get_text(GTK_ENTRY(widget));
  if (format != NULL)
  {
    if(widget == dt->date_format_entry)
      datetime_apply_format(dt, format, NULL);
    else if(widget == dt->time_format_entry)
      datetime_apply_format(dt, NULL, format);
  }
  datetime_update(dt);
  return FALSE;
}

/*
 * user closed the properties dialog
 */
static void datetime_dialog_response(GtkWidget *dlg, int foo, t_datetime *dt)
{
  if(dt == NULL)
    return;

  g_object_set_data(G_OBJECT(dt->plugin), "dialog", NULL);

  gtk_widget_destroy(dlg);
  xfce_panel_plugin_unblock_menu(dt->plugin);
  datetime_write_rc_file(dt->plugin, dt);
}

/*
 * show datetime properties dialog
 */
void datetime_properties_dialog(XfcePanelPlugin *plugin,
    t_datetime * datetime)
{
  GtkWidget *dlg,
	    *frame,
	    *vbox,
	    *hbox,
	    *label,
	    *image,
	    *button,
	    *entry,
	    *bin;

  DBG(">>>>> %s", GETTEXT_PACKAGE);
  xfce_textdomain (GETTEXT_PACKAGE, LOCALEDIR, "UTF-8");

  xfce_panel_plugin_block_menu(plugin);

  dlg = xfce_titled_dialog_new_with_buttons(_("Datetime properties"),
      NULL, /* or: GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(plugin))), */
      GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR,
      GTK_STOCK_CLOSE, GTK_RESPONSE_OK, NULL);

  g_object_set_data(G_OBJECT(plugin), "dialog", dlg);

  gtk_window_set_position (GTK_WINDOW (dlg), GTK_WIN_POS_CENTER);
  gtk_window_set_icon_name (GTK_WINDOW (dlg), "xfce4-settings");

  gtk_container_set_border_width(GTK_CONTAINER(dlg), 2);

  /*
   * time frame
   */
  frame = xfce_create_framebox(_("Time"), &bin);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dlg)->vbox), frame,
      FALSE, FALSE, 0);
  gtk_container_set_border_width(GTK_CONTAINER(frame), 6);

  /* vbox */
  vbox = gtk_vbox_new(FALSE, 8);
  gtk_container_add(GTK_CONTAINER(bin),vbox);

  /* hbox */
  hbox = gtk_hbox_new(FALSE, 2);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

  /* font label */
  label = gtk_label_new(_("Font:"));
  gtk_misc_set_alignment(GTK_MISC (label), 0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  /* font button */
  button = gtk_button_new_with_label(datetime->time_font);
  gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
  g_signal_connect(G_OBJECT(button), "clicked",
      G_CALLBACK(datetime_font_selection_cb), datetime);
  datetime->time_font_selector = button;

  /* hbox */
  hbox = gtk_hbox_new(FALSE, 2);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

  /* format label */
  label = gtk_label_new(_("Format:"));
  gtk_misc_set_alignment(GTK_MISC (label), 0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  /* format entry */
  entry = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(entry), datetime->time_format);
  gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT(entry), "focus-out-event",
                    G_CALLBACK (datetime_entry_change_cb), datetime);
  datetime->time_format_entry = entry;

  gtk_widget_show_all(frame);

  /*
   * Date frame
   */
  frame = xfce_create_framebox(_("Date"), &bin);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dlg)->vbox), frame,
      FALSE, FALSE, 0);
  gtk_container_set_border_width(GTK_CONTAINER(frame), 6);

  /* vbox */
  vbox = gtk_vbox_new(FALSE, 8);
  gtk_container_add(GTK_CONTAINER(bin),vbox);

  /* hbox */
  hbox = gtk_hbox_new(FALSE, 2);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

  /* font label */
  label = gtk_label_new(_("Font:"));
  gtk_misc_set_alignment(GTK_MISC (label), 0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  /* font button */
  button = gtk_button_new_with_label(datetime->date_font);
  gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
  g_signal_connect(G_OBJECT(button), "clicked",
      G_CALLBACK(datetime_font_selection_cb), datetime);
  datetime->date_font_selector = button;

  /* hbox */
  hbox = gtk_hbox_new(FALSE, 2);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

  /* format label */
  label = gtk_label_new(_("Format:"));
  gtk_misc_set_alignment(GTK_MISC (label), 0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  /* format entry */
  entry = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(entry), datetime->date_format);
  gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT(entry), "focus-out-event",
                    G_CALLBACK (datetime_entry_change_cb), datetime);
  datetime->date_format_entry = entry;

  gtk_widget_show_all(frame);

  /*
   * Calendar options frame
   */
  frame = xfce_create_framebox(_("Calendar"), &bin);
  gtk_container_set_border_width(GTK_CONTAINER(frame), 6);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dlg)->vbox), frame,
      FALSE, FALSE, 0);

  /* hbox */
  hbox = gtk_hbox_new(FALSE, 6);
  gtk_container_add(GTK_CONTAINER(bin),hbox);

  /* dialog info image */
  image = gtk_image_new_from_stock (GTK_STOCK_DIALOG_INFO,
                                      GTK_ICON_SIZE_DND);
  gtk_misc_set_alignment (GTK_MISC (image), 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

  /* week-start-on-monday setting is ignored by gtk since version 2.4 */
  label = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label),
      _("The information on which day the calendar week "
	"starts is derived from the locale."));
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
  gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);

  gtk_widget_show_all(frame);

  /* We're done! */
  g_signal_connect(dlg, "response",
      G_CALLBACK(datetime_dialog_response), datetime);

  /* show dialog */
  gtk_widget_show(dlg);
}

