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
#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfce4panel/xfce-panel-plugin.h>

#include "datetime.h"
#include "datetime-dialog.h"

#define PLUGIN_WEBSITE  "http://goodies.xfce.org/projects/panel-plugins/xfce4-datetime-plugin"

/* Layouts */
static const gchar *layout_strs[] = {
  N_("Date, then time"),
  N_("Time, then date"),
  N_("Date only"),
  N_("Time only")
};

typedef enum {

  /* standard format item; string is replaced with an example date or time */
  DT_COMBOBOX_ITEM_TYPE_STANDARD,

  /* custom format item; text is translated */
  DT_COMBOBOX_ITEM_TYPE_CUSTOM,

  /* inactive separator */
  DT_COMBOBOX_ITEM_TYPE_SEPARATOR,

} dt_combobox_item_type;

typedef struct {
  gchar *item;
  dt_combobox_item_type type;
} dt_combobox_item;

/*
 * Builtin formats are derived from xfce4-panel-clock.patch by Nick Schermer.
 */
static const dt_combobox_item dt_combobox_date[] = {
  { "%Y-%m-%d",       DT_COMBOBOX_ITEM_TYPE_STANDARD  },
  { "%Y %B %d",       DT_COMBOBOX_ITEM_TYPE_STANDARD  },
  { "---",            DT_COMBOBOX_ITEM_TYPE_SEPARATOR },  /* placeholder */
  { "%m/%d/%Y",       DT_COMBOBOX_ITEM_TYPE_STANDARD  },
  { "%B %d, %Y",      DT_COMBOBOX_ITEM_TYPE_STANDARD  },
  { "%b %d, %Y",      DT_COMBOBOX_ITEM_TYPE_STANDARD  },
  { "%A, %B %d, %Y",  DT_COMBOBOX_ITEM_TYPE_STANDARD  },
  { "%a, %b %d, %Y",  DT_COMBOBOX_ITEM_TYPE_STANDARD  },
  { "---",            DT_COMBOBOX_ITEM_TYPE_SEPARATOR },  /* placeholder */
  { "%d/%m/%Y",       DT_COMBOBOX_ITEM_TYPE_STANDARD  },
  { "%d %B %Y",       DT_COMBOBOX_ITEM_TYPE_STANDARD  },
  { "%d %b %Y",       DT_COMBOBOX_ITEM_TYPE_STANDARD  },
  { "%A, %d %B %Y",   DT_COMBOBOX_ITEM_TYPE_STANDARD  },
  { "%a, %d %b %Y",   DT_COMBOBOX_ITEM_TYPE_STANDARD  },
  { "---",            DT_COMBOBOX_ITEM_TYPE_SEPARATOR },  /* placeholder */
  { N_("Custom..."),  DT_COMBOBOX_ITEM_TYPE_CUSTOM    }
};
#define DT_COMBOBOX_DATE_COUNT (sizeof(dt_combobox_date)/sizeof(dt_combobox_item))

static const dt_combobox_item dt_combobox_time[] = {
  { "%H:%M",          DT_COMBOBOX_ITEM_TYPE_STANDARD  },
  { "%H:%M:%S",       DT_COMBOBOX_ITEM_TYPE_STANDARD  },
  { "---",            DT_COMBOBOX_ITEM_TYPE_SEPARATOR },  /* placeholder */
  { "%l:%M %P",       DT_COMBOBOX_ITEM_TYPE_STANDARD  },
  { "%l:%M:%S %P",    DT_COMBOBOX_ITEM_TYPE_STANDARD  },
  { "---",            DT_COMBOBOX_ITEM_TYPE_SEPARATOR },  /* placeholder */
  { N_("Custom..."),  DT_COMBOBOX_ITEM_TYPE_CUSTOM    }
};
#define DT_COMBOBOX_TIME_COUNT (sizeof(dt_combobox_time)/sizeof(dt_combobox_item))

/*
 * Example timestamp to show in the dialog.
 * Compute with:
 * date +%s -u -d 'dec 31 1999 23:59:59'
 */
static const time_t example_time_t = 946684799;

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
 * Read layout from combobox and set sensitivity
 */
static void
datetime_layout_changed(GtkComboBox *cbox, t_datetime *dt)
{
  t_layout layout;

  /* read layout */
  layout = gtk_combo_box_get_active(cbox);

#if USE_GTK_TOOLTIP_API
  switch(layout)
  {
    case LAYOUT_DATE:
      gtk_widget_show(dt->date_font_hbox);
      gtk_widget_hide(dt->date_tooltip_label);

      gtk_widget_hide(dt->time_font_hbox);
      gtk_widget_show(dt->time_tooltip_label);
      break;

    case LAYOUT_TIME:
      gtk_widget_hide(dt->date_font_hbox);
      gtk_widget_show(dt->date_tooltip_label);

      gtk_widget_show(dt->time_font_hbox);
      gtk_widget_hide(dt->time_tooltip_label);
      break;

    default:
      gtk_widget_show(dt->date_font_hbox);
      gtk_widget_hide(dt->date_tooltip_label);

      gtk_widget_show(dt->time_font_hbox);
      gtk_widget_hide(dt->time_tooltip_label);
  }
#else
  switch(layout)
  {
    case LAYOUT_DATE:
      gtk_widget_set_sensitive(dt->date_frame, TRUE);
      gtk_widget_set_sensitive(dt->time_frame, FALSE);
      break;

    case LAYOUT_TIME:
      gtk_widget_set_sensitive(dt->date_frame, FALSE);
      gtk_widget_set_sensitive(dt->time_frame, TRUE);
      break;

    default:
      gtk_widget_set_sensitive(dt->date_frame, TRUE);
      gtk_widget_set_sensitive(dt->time_frame, TRUE);
  }
#endif

  datetime_apply_layout(dt, layout);
  datetime_update(dt);
}

/*
 * Row separator for format-comboboxes of date and time
 * derived from xfce4-panel-clock.patch by Nick Schermer
 */
static gboolean
combo_box_row_separator(GtkTreeModel *model,
                        GtkTreeIter  *iter,
                        gpointer data)
{
  const dt_combobox_item  *items = (dt_combobox_item *)data;
  gint                    current;
  GtkTreePath             *path;

  path = gtk_tree_model_get_path(model, iter);
  current = gtk_tree_path_get_indices(path)[0];
  gtk_tree_path_free(path);

  return items[current].type == DT_COMBOBOX_ITEM_TYPE_SEPARATOR;
}

/*
 * Read date format from combobox and set sensitivity
 */
static void
date_format_changed(GtkComboBox *cbox, t_datetime *dt)
{
  const gint active = gtk_combo_box_get_active(cbox);

  switch(dt_combobox_date[active].type)
  {
    case DT_COMBOBOX_ITEM_TYPE_STANDARD:
      /* hide custom text entry box and tell datetime which format is selected */
      gtk_widget_hide(dt->date_format_entry);
      datetime_apply_format(dt, dt_combobox_date[active].item, NULL);
      break;
    case DT_COMBOBOX_ITEM_TYPE_CUSTOM:
      /* initialize custom text entry box with current format and show the box */
      gtk_entry_set_text(GTK_ENTRY(dt->date_format_entry), dt->date_format);
      gtk_widget_show(dt->date_format_entry);
      break;
    default:
      break; /* separators should never be active */
  }

  datetime_update(dt);
}

/*
 * Read time format from combobox and set sensitivity
 */
static void
time_format_changed(GtkComboBox *cbox, t_datetime *dt)
{
  const gint active = gtk_combo_box_get_active(cbox);

  switch(dt_combobox_time[active].type)
  {
    case DT_COMBOBOX_ITEM_TYPE_STANDARD:
      /* hide custom text entry box and tell datetime which format is selected */
      gtk_widget_hide(dt->time_format_entry);
      datetime_apply_format(dt, NULL, dt_combobox_time[active].item);
      break;
    case DT_COMBOBOX_ITEM_TYPE_CUSTOM:
      /* initialize custom text entry box with current format and show the box */
      gtk_entry_set_text(GTK_ENTRY(dt->time_format_entry), dt->time_format);
      gtk_widget_show(dt->time_format_entry);
      break;
    default:
      break; /* separators should never be active */
  }

  datetime_update(dt);
}

/*
 * read values from date and time entry and inform datetime about it
 */
static gboolean
datetime_entry_change_cb(GtkWidget *widget, GdkEventFocus *ev, t_datetime *dt)
{
  const gchar *format;
  format = gtk_entry_get_text(GTK_ENTRY(widget));
  if (format != NULL)
  {
    if(widget == dt->date_format_entry)         /* date */
      datetime_apply_format(dt, format, NULL);
    else if(widget == dt->time_format_entry)    /* or time */
      datetime_apply_format(dt, NULL, format);
  }
  datetime_update(dt);
  return FALSE;
}

/*
 * user closed the properties dialog
 */
static void
datetime_dialog_response(GtkWidget *dlg, int response, t_datetime *dt)
{
  gboolean result;

  if(dt == NULL)
    return;

  if (response == GTK_RESPONSE_HELP)
  {
      /* show help */
      result = g_spawn_command_line_async("exo-open --launch WebBrowser " PLUGIN_WEBSITE, NULL);

      if (G_UNLIKELY(result == FALSE))
          g_warning(_("Unable to open the following url: %s"), PLUGIN_WEBSITE);
  }
  else
  {
    g_object_set_data(G_OBJECT(dt->plugin), "dialog", NULL);

    gtk_widget_destroy(dlg);
    xfce_panel_plugin_unblock_menu(dt->plugin);
    datetime_write_rc_file(dt->plugin, dt);
  }
}

/*
 * show datetime properties dialog
 */
void
datetime_properties_dialog(XfcePanelPlugin *plugin, t_datetime * datetime)
{
  guint i;
  gchar *str;
  struct tm *exampletm = gmtime(&example_time_t);
  GtkWidget *dlg,
            *frame,
            *vbox,
            *hbox,
            *layout_combobox,
            *time_combobox,
            *date_combobox,
            *label,
            *button,
            *entry,
            *bin;
  GtkSizeGroup  *sg;
  gint i_custom; /* index of custom menu item */

  xfce_textdomain (GETTEXT_PACKAGE, LOCALEDIR, "UTF-8");

  xfce_panel_plugin_block_menu(plugin);

  dlg = xfce_titled_dialog_new_with_buttons(_("Datetime"),
      NULL, /* or: GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(plugin))), */
      GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR,
      GTK_STOCK_HELP, GTK_RESPONSE_HELP,
      GTK_STOCK_CLOSE, GTK_RESPONSE_OK,
      NULL);

  g_object_set_data(G_OBJECT(plugin), "dialog", dlg);

  gtk_window_set_position (GTK_WINDOW (dlg), GTK_WIN_POS_CENTER);
  gtk_window_set_icon_name (GTK_WINDOW (dlg), "xfce4-settings");

  gtk_container_set_border_width(GTK_CONTAINER(dlg), 2);

  /* size group */
  sg = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

  /*
   * layout frame
   */
  frame = xfce_gtk_frame_box_new(_("Layout"), &bin);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dlg)->vbox), frame,
      FALSE, FALSE, 0);
  gtk_container_set_border_width(GTK_CONTAINER(frame), 6);

  /* vbox */
  vbox = gtk_vbox_new(FALSE, 8);
  gtk_container_add(GTK_CONTAINER(bin),vbox);

  /* hbox */
  hbox = gtk_hbox_new(FALSE, 2);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

  /* Format label */
  label = gtk_label_new(_("Format:"));
  gtk_misc_set_alignment(GTK_MISC (label), 0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
  gtk_size_group_add_widget(sg, label);

  /* Layout combobox */
  layout_combobox = gtk_combo_box_new_text();
  gtk_box_pack_start(GTK_BOX(hbox), layout_combobox, TRUE, TRUE, 0);
  for(i=0; i < LAYOUT_COUNT; i++)
    gtk_combo_box_append_text(GTK_COMBO_BOX(layout_combobox), _(layout_strs[i]));
  gtk_combo_box_set_active(GTK_COMBO_BOX(layout_combobox), datetime->layout);
  g_signal_connect(G_OBJECT(layout_combobox), "changed",
      G_CALLBACK(datetime_layout_changed), datetime);

  /* show frame */
  gtk_widget_show_all(frame);

  /*
   * Date frame
   */
  datetime->date_frame = xfce_gtk_frame_box_new(_("Date"), &bin);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dlg)->vbox), datetime->date_frame,
      FALSE, FALSE, 0);
  gtk_container_set_border_width(GTK_CONTAINER(datetime->date_frame), 6);

  /* vbox */
  vbox = gtk_vbox_new(FALSE, 8);
  gtk_container_add(GTK_CONTAINER(bin),vbox);

#if USE_GTK_TOOLTIP_API
  /* tooltip label */
  str = g_markup_printf_escaped("<span style=\"italic\">%s</span>",
                                _("The date will appear in a tooltip."));
  datetime->date_tooltip_label = gtk_label_new(str);
  g_free(str);
  gtk_label_set_use_markup(GTK_LABEL(datetime->date_tooltip_label), TRUE);
  gtk_misc_set_alignment(GTK_MISC(datetime->date_tooltip_label), 0.0f, 0.0f);
  gtk_box_pack_start(GTK_BOX(vbox), datetime->date_tooltip_label, FALSE, FALSE, 0);
#endif

  /* hbox */
  datetime->date_font_hbox = gtk_hbox_new(FALSE, 2);
  gtk_box_pack_start(GTK_BOX(vbox), datetime->date_font_hbox, FALSE, FALSE, 0);

  /* font label */
  label = gtk_label_new(_("Font:"));
  gtk_misc_set_alignment(GTK_MISC (label), 0, 0.5);
  gtk_box_pack_start(GTK_BOX(datetime->date_font_hbox), label, FALSE, FALSE, 0);
  gtk_size_group_add_widget(sg, label);

  /* font button */
  button = gtk_button_new_with_label(datetime->date_font);
  gtk_box_pack_start(GTK_BOX(datetime->date_font_hbox), button, TRUE, TRUE, 0);
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
  gtk_size_group_add_widget(sg, label);

  /* format combobox */
  date_combobox = gtk_combo_box_new_text();
  gtk_box_pack_start(GTK_BOX(hbox), date_combobox, TRUE, TRUE, 0);
  i_custom = 0;
  for(i=0; i < DT_COMBOBOX_DATE_COUNT; i++)
  {
    switch(dt_combobox_date[i].type)
    {
      case DT_COMBOBOX_ITEM_TYPE_STANDARD:
        str = datetime_do_utf8strftime(dt_combobox_date[i].item, exampletm);
        gtk_combo_box_append_text(GTK_COMBO_BOX(date_combobox), str);
        g_free(str);
        /* set active
         * strcmp isn't fast, but it is done only once while opening the dialog
         */
        if(strcmp(datetime->date_format, dt_combobox_date[i].item) == 0)
          gtk_combo_box_set_active(GTK_COMBO_BOX(date_combobox), i);
        break;
      case DT_COMBOBOX_ITEM_TYPE_CUSTOM:
        gtk_combo_box_append_text(GTK_COMBO_BOX(date_combobox), _(dt_combobox_date[i].item));
        i_custom = i;
        break;
      case DT_COMBOBOX_ITEM_TYPE_SEPARATOR: /* placeholder item does not need to be translated */
        gtk_combo_box_append_text(GTK_COMBO_BOX(date_combobox), dt_combobox_date[i].item);
        break;
      default:
        break;
    }
  }
  /* if no item activated -> activate custom item */
  if(gtk_combo_box_get_active(GTK_COMBO_BOX(date_combobox)) < 0)
    gtk_combo_box_set_active(GTK_COMBO_BOX(date_combobox), i_custom);
  gtk_combo_box_set_row_separator_func(GTK_COMBO_BOX(date_combobox),
                                       combo_box_row_separator,
                                       (gpointer)dt_combobox_date, NULL);
  g_signal_connect(G_OBJECT(date_combobox), "changed",
      G_CALLBACK(date_format_changed), datetime);
  datetime->date_format_combobox = date_combobox;

  /* hbox */
  hbox = gtk_hbox_new(FALSE, 2);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

  /* format entry */
  entry = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(entry), datetime->date_format);
  gtk_box_pack_end(GTK_BOX(hbox), entry, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT(entry), "focus-out-event",
                    G_CALLBACK (datetime_entry_change_cb), datetime);
  datetime->date_format_entry = entry;

  gtk_widget_show_all(datetime->date_frame);

  /*
   * time frame
   */
  datetime->time_frame = xfce_gtk_frame_box_new(_("Time"), &bin);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dlg)->vbox), datetime->time_frame,
      FALSE, FALSE, 0);
  gtk_container_set_border_width(GTK_CONTAINER(datetime->time_frame), 6);

  /* vbox */
  vbox = gtk_vbox_new(FALSE, 8);
  gtk_container_add(GTK_CONTAINER(bin),vbox);

#if USE_GTK_TOOLTIP_API
  /* tooltip label */
  str = g_markup_printf_escaped("<span style=\"italic\">%s</span>",
                                _("The time will appear in a tooltip."));
  datetime->time_tooltip_label = gtk_label_new(str);
  g_free(str);
  gtk_label_set_use_markup(GTK_LABEL(datetime->time_tooltip_label), TRUE);
  gtk_misc_set_alignment(GTK_MISC(datetime->time_tooltip_label), 0.0f, 0.0f);
  gtk_box_pack_start(GTK_BOX(vbox), datetime->time_tooltip_label, FALSE, FALSE, 0);
#endif

  /* hbox */
  datetime->time_font_hbox = gtk_hbox_new(FALSE, 2);
  gtk_box_pack_start(GTK_BOX(vbox), datetime->time_font_hbox, FALSE, FALSE, 0);

  /* font label */
  label = gtk_label_new(_("Font:"));
  gtk_misc_set_alignment(GTK_MISC (label), 0, 0.5);
  gtk_box_pack_start(GTK_BOX(datetime->time_font_hbox), label, FALSE, FALSE, 0);
  gtk_size_group_add_widget(sg, label);

  /* font button */
  button = gtk_button_new_with_label(datetime->time_font);
  gtk_box_pack_start(GTK_BOX(datetime->time_font_hbox), button, TRUE, TRUE, 0);
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
  gtk_size_group_add_widget(sg, label);

  /* format combobox */
  time_combobox = gtk_combo_box_new_text();
  gtk_box_pack_start(GTK_BOX(hbox), time_combobox, TRUE, TRUE, 0);
  i_custom = 0;
  for(i=0; i < DT_COMBOBOX_TIME_COUNT; i++)
  {
    switch(dt_combobox_time[i].type)
    {
      case DT_COMBOBOX_ITEM_TYPE_STANDARD:
        str = datetime_do_utf8strftime(dt_combobox_time[i].item, exampletm);
        gtk_combo_box_append_text(GTK_COMBO_BOX(time_combobox), str);
        g_free(str);
        /* set active
         * strcmp isn't fast, but it is done only once while opening the dialog
         */
        if(strcmp(datetime->time_format, dt_combobox_time[i].item) == 0)
          gtk_combo_box_set_active(GTK_COMBO_BOX(time_combobox), i);
        break;
      case DT_COMBOBOX_ITEM_TYPE_CUSTOM:
        gtk_combo_box_append_text(GTK_COMBO_BOX(time_combobox), _(dt_combobox_time[i].item));
        i_custom = i;
        break;
      case DT_COMBOBOX_ITEM_TYPE_SEPARATOR: /* placeholder item does not need to be translated */
        gtk_combo_box_append_text(GTK_COMBO_BOX(time_combobox), dt_combobox_time[i].item);
        break;
      default:
        break;
    }
  }
  /* if no item activated -> activate custom item */
  if(gtk_combo_box_get_active(GTK_COMBO_BOX(time_combobox)) < 0)
    gtk_combo_box_set_active(GTK_COMBO_BOX(time_combobox), i_custom);
  gtk_combo_box_set_row_separator_func(GTK_COMBO_BOX(time_combobox),
                                       combo_box_row_separator,
                                       (gpointer)dt_combobox_time, NULL);
  g_signal_connect(G_OBJECT(time_combobox), "changed",
      G_CALLBACK(time_format_changed), datetime);
  datetime->time_format_combobox = time_combobox;

  /* hbox */
  hbox = gtk_hbox_new(FALSE, 2);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

  /* format entry */
  entry = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(entry), datetime->time_format);
  gtk_box_pack_end(GTK_BOX(hbox), entry, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT(entry), "focus-out-event",
                    G_CALLBACK (datetime_entry_change_cb), datetime);
  datetime->time_format_entry = entry;

  gtk_widget_show_all(datetime->time_frame);

  /* We're done! */
  g_signal_connect(dlg, "response",
      G_CALLBACK(datetime_dialog_response), datetime);

  /* set sensitivity for all widgets */
  datetime_layout_changed(GTK_COMBO_BOX(layout_combobox), datetime);
  date_format_changed(GTK_COMBO_BOX(date_combobox), datetime);
  time_format_changed(GTK_COMBO_BOX(time_combobox), datetime);

  /* show dialog */
  gtk_widget_show(dlg);
}

