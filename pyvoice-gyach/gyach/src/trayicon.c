/* most of this ripped from gaim */
/*****************************************************************************
 * trayicon.c
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 *
 * Copyright (C) 2003-2006, Erica Andrews
 * (Phrozensmoke ['at'] yahoo.com)
 * http://phpaint.sourceforge.net/pyvoicechat/
 * 
 * Released under the terms of the GPL.
 * *NO WARRANTY*
 *
 * Support for FreeDesktop (Gnome/KDE/IceWM) system tray icons
 *****************************************************************************/

#include <string.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <gtk/gtkplug.h>
#include <gdk/gdkx.h>

#include "images.h"
#include "callbacks.h"
#include "users.h"
#include "util.h"
#include "trayicon.h"

G_BEGIN_DECLS

#define TYPE_TRAY_ICON (tray_icon_get_type ())
#define TRAY_ICON(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_TRAY_ICON, TrayIcon))
#define TRAY_ICON_CLASS(class) (G_TYPE_CHECK_CLASS_CAST ((class), TYPE_TRAY_ICON, TrayIconClass))
#define IS_TRAY_ICON(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_TRAY_ICON))
#define TRAY_ICON_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_TRAY_ICON, TrayIconClass))

typedef struct {
  GtkPlug parent_instance;
  guint stamp;
  Atom selection_atom;
  Atom manager_atom;
  Atom system_tray_opcode_atom;
  Window manager_window;
} TrayIcon;

typedef struct {
  GtkPlugClass parent_class;
} TrayIconClass;

GType tray_icon_get_type (void);

TrayIcon *tray_icon_new (const gchar *name);
guint tray_icon_send_message (TrayIcon *icon, gint timeout, const gchar *msg, gint len);
void tray_icon_cancel_messsage (TrayIcon *icon, guint id);

G_END_DECLS


#define SYSTEM_TRAY_REQUEST_DOCK 0
#define SYSTEM_TRAY_BEGIN_MESSAGE 1
#define SYSTEM_TRAY_CANCEL_MESSAGE 2

static GtkPlugClass *parent_class = NULL;
static void tray_icon_init (TrayIcon *icon);
static void tray_icon_class_init (TrayIconClass *class);
static void tray_icon_unrealize (GtkWidget *widget);
static void tray_icon_update_manager_window (TrayIcon *icon);

extern void on_leave_room_or_conf(GtkMenuItem *widget, gpointer user_data);
extern void on_yahoo_mail_act(GtkMenuItem *widget, gpointer user_data);
extern void on_rejoin_room  (GtkMenuItem *menuitem, gpointer  user_data);
extern void on_status_idle_activate (GtkMenuItem *menuitem, gpointer user_data);
extern void on_joinaroom  (GtkMenuItem *menuitem, gpointer  user_data);
extern void open_perm_ignore (GtkMenuItem *menuitem, gpointer  user_data);
extern void open_temp_friend_editor (GtkMenuItem *menuitem, gpointer  user_data); 
extern void open_flooder_editor (GtkMenuItem *menuitem, gpointer  user_data);
extern void open_ignore_editor (GtkMenuItem *menuitem, gpointer  user_data);
extern void write_config();

extern void set_tooltip(GtkWidget *somewid, char *somechar);

extern int my_status;
extern int  ignore_guests;

static TrayIcon *systray = NULL;
static GtkWidget *trayimage = NULL;

int enable_trayicon=1;  /* configurable option, setup window */

static char tray_budcount[56]="";
static char tray_mailcount[32]="";
int tray_have_mail=0;
int tray_current_status=0;
int tray_connected=0;
static char menbuf[56]="";
static GtkWidget *menu = NULL;
static GdkPixbuf *imbuf=NULL;

GType tray_icon_get_type (void)
{
  static GType our_type = 0;

  our_type = g_type_from_name ("TrayIcon");

  if (our_type == 0) {
	static const GTypeInfo our_info = {
	  sizeof (TrayIconClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL,
	  (GClassInitFunc) tray_icon_class_init, NULL, NULL, sizeof (TrayIcon), 0,
	  (GInstanceInitFunc) tray_icon_init};

	our_type = g_type_register_static (GTK_TYPE_PLUG, "TrayIcon", &our_info, 0);
  } else if (parent_class == NULL) {
	tray_icon_class_init ((TrayIconClass *)g_type_class_peek (our_type));
  }

  return our_type;
}

static void tray_icon_init (TrayIcon *icon)
{
  icon->stamp = 1;
  gtk_widget_add_events (GTK_WIDGET (icon), GDK_PROPERTY_CHANGE_MASK);
}

static void tray_icon_class_init (TrayIconClass *class)
{
  GtkWidgetClass *widget_class = (GtkWidgetClass *)class;
  parent_class = g_type_class_peek_parent (class);
  widget_class->unrealize = tray_icon_unrealize;
}

static GdkFilterReturn tray_icon_manager_filter (GdkXEvent *xevent, GdkEvent *event, gpointer data)
{
  TrayIcon *icon = data;
  XEvent *xev = (XEvent *)xevent;

  if (xev->xany.type == ClientMessage && xev->xclient.message_type == icon->manager_atom &&
	  xev->xclient.data.l[1] == icon->selection_atom) {
	tray_icon_update_manager_window (icon);
  } else if (xev->xany.window == icon->manager_window) {
	if (xev->xany.type == DestroyNotify) {
	  tray_icon_update_manager_window (icon);
	}
  }

  return GDK_FILTER_CONTINUE;
}

static void tray_icon_unrealize (GtkWidget *widget)
{
  TrayIcon *icon = TRAY_ICON (widget);
  GdkWindow *root_window;

  if (icon->manager_window != None) {
	GdkWindow *gdkwin;
	gdkwin = gdk_window_lookup (icon->manager_window);
	gdk_window_remove_filter (gdkwin, tray_icon_manager_filter, icon);
  }

  root_window = gdk_window_lookup (gdk_x11_get_default_root_xwindow ());  
  gdk_window_remove_filter (root_window, tray_icon_manager_filter, icon);

  if (GTK_WIDGET_CLASS (parent_class)->unrealize) {
	(* GTK_WIDGET_CLASS (parent_class)->unrealize) (widget);
  }
}

static void tray_icon_send_manager_message (TrayIcon *icon, long message, Window window,
											long data1, long data2, long data3)
{
  XClientMessageEvent ev;
  Display *display;

  ev.type = ClientMessage;
  ev.window = window;
  ev.message_type = icon->system_tray_opcode_atom;
  ev.format = 32;
  ev.data.l[0] = gdk_x11_get_server_time (GTK_WIDGET (icon)->window);
  ev.data.l[1] = message;
  ev.data.l[2] = data1;
  ev.data.l[3] = data2;
  ev.data.l[4] = data3;

  display = GDK_DISPLAY ();

  gdk_error_trap_push ();
  XSendEvent (display, icon->manager_window, False, NoEventMask, (XEvent *)&ev);
  gdk_error_trap_pop ();
}

static void tray_icon_send_dock_request (TrayIcon *icon)
{
  tray_icon_send_manager_message (icon, SYSTEM_TRAY_REQUEST_DOCK,
								  icon->manager_window, gtk_plug_get_id (GTK_PLUG (icon)), 0, 0);
}

static void tray_icon_update_manager_window (TrayIcon *icon)
{
  Display *xdisplay;
  xdisplay = GDK_DISPLAY ();

  if (icon->manager_window != None) {
	GdkWindow *gdkwin;
	gdkwin = gdk_window_lookup (icon->manager_window);
	gdk_window_remove_filter (gdkwin, tray_icon_manager_filter, icon);
  }

  XGrabServer (xdisplay);
  icon->manager_window = XGetSelectionOwner (xdisplay, icon->selection_atom);

  if (icon->manager_window != None) {
	XSelectInput (xdisplay, icon->manager_window, StructureNotifyMask);
  }

  XUngrabServer (xdisplay);
  XFlush (xdisplay);

  if (icon->manager_window != None) {
	GdkWindow *gdkwin;
	gdkwin = gdk_window_lookup (icon->manager_window);
	gdk_window_add_filter (gdkwin, tray_icon_manager_filter, icon);
	tray_icon_send_dock_request (icon);
  }
}

TrayIcon *tray_icon_new_for_xscreen (Screen *xscreen, const char *name)
{
  TrayIcon *icon;
  char buffer[48];
  GdkWindow *root_window;

  g_return_val_if_fail (xscreen != NULL, NULL);

  icon = g_object_new (TYPE_TRAY_ICON, NULL);
  gtk_window_set_title (GTK_WINDOW (icon), name);

  gtk_plug_construct (GTK_PLUG (icon), 0);

  gtk_widget_realize (GTK_WIDGET (icon));
  g_snprintf (buffer, 46 , "_NET_SYSTEM_TRAY_S%d", XScreenNumberOfScreen (xscreen));	
  icon->selection_atom = XInternAtom (DisplayOfScreen (xscreen), buffer, False);
  icon->system_tray_opcode_atom = XInternAtom (DisplayOfScreen (xscreen), "_NET_SYSTEM_TRAY_OPCODE", False);
  tray_icon_update_manager_window (icon);

  root_window = gdk_window_lookup (gdk_x11_get_default_root_xwindow ());

  gdk_window_add_filter (root_window, tray_icon_manager_filter, icon);

  return icon;
}

TrayIcon *tray_icon_new (const gchar *name)
{
  TrayIcon *icon;
  icon = tray_icon_new_for_xscreen (DefaultScreenOfDisplay (GDK_DISPLAY ()), name);
  return icon;
}

guint tray_icon_send_message (TrayIcon *icon, gint timeout, const gchar *msg, gint len)
{
  guint stamp;

  g_return_val_if_fail (IS_TRAY_ICON (icon), 0);
  g_return_val_if_fail (timeout >= 0, 0);
  g_return_val_if_fail (msg != NULL, 0);

  if (icon->manager_window == None) {
	return 0;
  }

  if (len < 0) {
	len = strlen (msg);
  }

  stamp = icon->stamp++;

  tray_icon_send_manager_message (icon, SYSTEM_TRAY_BEGIN_MESSAGE, (Window)gtk_plug_get_id (GTK_PLUG (icon)), timeout, len, stamp);

  gdk_error_trap_push ();
  while (len > 0) {
	XClientMessageEvent ev;
	Display *xdisplay;
	xdisplay = GDK_DISPLAY ();
	ev.type = ClientMessage;
	ev.window = (Window)gtk_plug_get_id (GTK_PLUG (icon));
	ev.format = 8;
	ev.message_type = XInternAtom (xdisplay, "_NET_SYSTEM_TRAY_MESSAGE_DATA", False);

	if (len > 20) {
	  memcpy (&ev.data, msg, 20);
	  len -= 20;
	  msg += 20;
	} else {
	  memcpy (&ev.data, msg, len);
	  len = 0;
	}

	XSendEvent (xdisplay, icon->manager_window, False, StructureNotifyMask, (XEvent *)&ev);
	XSync (xdisplay, False);
  }
  gdk_error_trap_pop ();
  return stamp;
}

void tray_icon_cancel_message (TrayIcon *icon, guint id)
{
  g_return_if_fail (IS_TRAY_ICON (icon));
  g_return_if_fail (id > 0);

  tray_icon_send_manager_message (icon, SYSTEM_TRAY_CANCEL_MESSAGE, (Window)gtk_plug_get_id (GTK_PLUG (icon)), id, 0, 0);
}



void tray_cb1 (GtkMenuItem *menuitem, gpointer  user_data) {
	enable_sound_events=gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem));
	write_config();
}
void tray_cb2 (GtkMenuItem *menuitem, gpointer  user_data) {
	enable_sound_events_pm=gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem));
	write_config();
}
void tray_cb3 (GtkMenuItem *menuitem, gpointer  user_data) {
	show_emoticons=gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem));
	write_config();
}
void tray_cb4 (GtkMenuItem *menuitem, gpointer  user_data) {
	enable_animations=gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem));
	write_config();
}
void tray_cb5 (GtkMenuItem *menuitem, gpointer  user_data) {
	show_blended_colors=gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem));
	write_config();
}
void tray_cb6 (GtkMenuItem *menuitem, gpointer  user_data) {
	enable_tuxvironments=gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem));
	write_config();
}
void tray_cb7 (GtkMenuItem *menuitem, gpointer  user_data) {
	show_avatars=gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem));
	write_config();
}
void tray_cb8 (GtkMenuItem *menuitem, gpointer  user_data) {
	pm_from_friends=gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem));
	write_config();
}

void tray_cb9 (GtkMenuItem *menuitem, gpointer  user_data) {
	pm_from_users=gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem));
	write_config();
}
void tray_cb10 (GtkMenuItem *menuitem, gpointer  user_data) {
	pm_from_all=gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem));
	write_config();
}
void tray_cb11 (GtkMenuItem *menuitem, gpointer  user_data) {
	enable_chat_spam_filter=gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem));
	if (enable_chat_spam_filter) {ignore_guests=1;}
	write_config();
}
void tray_cb12 (GtkMenuItem *menuitem, gpointer  user_data) {
	disallow_random_friend_add=gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem));
	write_config();
}
void tray_cb13 (GtkMenuItem *menuitem, gpointer  user_data) {
	allow_no_sent_files=gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem));
	write_config();
}

void tray_cb14 (GtkMenuItem *menuitem, gpointer  user_data) {
	enable_audibles=gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem));
	write_config();
}
void tray_cb15 (GtkMenuItem *menuitem, gpointer  user_data) {
	show_bimages=gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem));
	write_config();
}
void tray_cb16 (GtkMenuItem *menuitem, gpointer  user_data) {
	show_yavatars=gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem));
	write_config();
	update_buddy_clist();
}
void tray_cb17 (GtkMenuItem *menuitem, gpointer  user_data) {
	auto_reject_invitations=gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem));
	write_config();
}

void tray_disable_cb (GtkMenuItem *menuitem, gpointer  user_data) {
	enable_trayicon=0;
	write_config();
       trayicon_destroy ();
}



static void systray_menu (GdkEventButton *event)
{
  GtkWidget *submenu = NULL;
  GSList *status_group = NULL;
  GtkWidget *entry;

  if (menu) {
  	gtk_widget_show_all (menu);
  	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, event->button, event->time);
	return; 
	/* gtk_widget_destroy (menu);  */
  }

  menu = gtk_menu_new ();

  entry =  gtk_menu_item_new_with_label (_("Status"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);
  /* gtk_widget_set_sensitive(entry,tray_connected); */

  submenu= gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (entry), submenu);
  gtk_widget_set_sensitive(submenu,tray_connected);

  entry = gtk_radio_menu_item_new_with_label (status_group, _("Here"));
  status_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (entry));
  if (my_status==0) {gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(entry), 1 );}
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (on_status_here_activate), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry = gtk_radio_menu_item_new_with_label (status_group, _("Be Right Back"));
  status_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (entry));
  if (my_status==1) {gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(entry), 1 );}
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (on_status_be_right_back_activate), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry = gtk_radio_menu_item_new_with_label (status_group, _("Busy"));
  status_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (entry));
  if (my_status==2) {gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(entry), 1 );}
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (on_status_busy_activate), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry = gtk_radio_menu_item_new_with_label (status_group, _("Not At Home"));
  status_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (entry));
  if (my_status==3) {gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(entry), 1 );}
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (on_status_not_at_home_activate), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry = gtk_radio_menu_item_new_with_label (status_group, _("Not At My Desk"));
  status_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (entry));
  if (my_status==4) {gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(entry), 1 );}
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (on_status_not_at_my_desk_activate), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry = gtk_radio_menu_item_new_with_label (status_group, _("Not In The Office"));
  status_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (entry));
  if (my_status==5) {gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(entry), 1 );}
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (on_status_not_in_the_office_activate), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry = gtk_radio_menu_item_new_with_label (status_group, _("On The Phone"));
  status_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (entry));
  if (my_status==6) {gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(entry), 1 );}
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (on_status_on_the_phone_activate), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry = gtk_radio_menu_item_new_with_label (status_group, _("On Vacation"));
  status_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (entry));
  if (my_status==7) {gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(entry), 1 );}
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (on_status_on_vacation_activate), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry = gtk_radio_menu_item_new_with_label (status_group, _("Out To Lunch"));
  status_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (entry));
  if (my_status==8) {gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(entry), 1 );}
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (on_status_out_to_lunch_activate), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry = gtk_radio_menu_item_new_with_label (status_group, _("Stepped Out"));
  status_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (entry));
  if (my_status==9) {gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(entry), 1 );}
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (on_status_stepped_out_activate), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry = gtk_radio_menu_item_new_with_label (status_group, _("Invisible"));
  status_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (entry));
  if (my_status==12) {gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(entry), 1 );}
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (on_status_invisible_activate), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry = gtk_radio_menu_item_new_with_label (status_group, _("Auto-Away"));
  status_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (entry));
  if (my_status==11) {gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(entry), 1 );}
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (on_status_autoaway_activate), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry = gtk_radio_menu_item_new_with_label (status_group, _("Idle"));
  status_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (entry));
  if (my_status==999) {gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(entry), 1 );}
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (on_status_idle_activate), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry = gtk_radio_menu_item_new_with_label (status_group, _("Custom Message"));
  status_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (entry));
  if (my_status==10) {gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(entry), 1 );}
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (on_status_custom_activate), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry = gtk_menu_item_new_with_label (_("New Custom Message..."));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (on_make_custom_away), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);



  entry =  gtk_menu_item_new_with_label (_("Protection"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);
  /* gtk_widget_set_sensitive(entry,tray_connected); */ /* should always be sensitive */

  submenu= gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (entry), submenu);
  /* gtk_widget_set_sensitive(submenu,tray_connected); */ /* should always be sensitive */


  entry =  gtk_menu_item_new_with_label (_("Accept Private Messages,\nInvitations, and Files From:"));
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry =  gtk_check_menu_item_new_with_label (_("Friends/Buddies"));
  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(entry), pm_from_friends);
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb8), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry =  gtk_check_menu_item_new_with_label (_("Users in current room"));
  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(entry), pm_from_users);
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb9), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry =  gtk_check_menu_item_new_with_label (_("All Yahoo! users"));
  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(entry), pm_from_all);
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb10), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry =  gtk_check_menu_item_new_with_label (_("Enable spam protection (PM & Chat)"));
  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(entry), enable_chat_spam_filter);
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb11), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry =  gtk_check_menu_item_new_with_label (_("Do not allow anyone who\nis not a friend to add me\nto their friend's list"));
  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(entry), disallow_random_friend_add);
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb12), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry =  gtk_check_menu_item_new_with_label (_("Do not accept any sent files"));
  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(entry), allow_no_sent_files);
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb13), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry =  gtk_check_menu_item_new_with_label (_("Automatically reject all invitations"));
  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(entry), auto_reject_invitations);
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb17), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry =  gtk_image_menu_item_new_with_label (_(" Gyach Enhanced Ignore List... "));
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DIALOG_ERROR, GTK_ICON_SIZE_MENU) ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (open_ignore_editor), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry =  gtk_image_menu_item_new_with_label (_(" Yahoo! Ignore List... "));
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DIALOG_ERROR, GTK_ICON_SIZE_MENU) ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (open_perm_ignore), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);
  gtk_widget_set_sensitive(entry,tray_connected);

  entry =  gtk_image_menu_item_new_with_label (_(" Flooders/Booters... "));
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_PREFERENCES, GTK_ICON_SIZE_MENU) ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (open_flooder_editor), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry =  gtk_image_menu_item_new_with_label (_(" Temporary Friends... "));
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_PREFERENCES, GTK_ICON_SIZE_MENU) ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (open_temp_friend_editor), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);




  entry =  gtk_menu_item_new_with_label (_("Options"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);
  /* gtk_widget_set_sensitive(entry,tray_connected); */ /* should always be sensitive */

  submenu= gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (entry), submenu);
  /* gtk_widget_set_sensitive(submenu,tray_connected); */ /* should always be sensitive */

  entry =  gtk_check_menu_item_new_with_label (_("Enable sound events"));
  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(entry), enable_sound_events);
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb1), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry =  gtk_check_menu_item_new_with_label (_("Enable PM sound events"));
  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(entry), enable_sound_events_pm);
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb2), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry =  gtk_check_menu_item_new_with_label (_("Enable Audibles"));
  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(entry), enable_audibles);
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb14), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry =  gtk_check_menu_item_new_with_label (_("Show smileys"));
  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(entry), show_emoticons);
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb3), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry =  gtk_check_menu_item_new_with_label (_("Show animated smileys"));
  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(entry), enable_animations );
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb4), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);


  entry =  gtk_check_menu_item_new_with_label (_("Show Buddy Images"));
  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(entry), show_bimages);
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb15), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry =  gtk_check_menu_item_new_with_label (_("Faders"));
  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(entry), show_blended_colors);
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb5), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry =  gtk_check_menu_item_new_with_label (_("Enable TUXVironments"));
  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(entry), enable_tuxvironments);
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb6), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry =  gtk_check_menu_item_new_with_label (_("Show Avatars"));
  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(entry), show_avatars);
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb7), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);


  entry =  gtk_check_menu_item_new_with_label (_("Show buddy list avatars"));
  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(entry), show_yavatars);
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb16), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry =  gtk_image_menu_item_new_with_label (_("Disable Tray Icon"));
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DIALOG_ERROR, GTK_ICON_SIZE_MENU) ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_disable_cb), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);



  entry =  gtk_menu_item_new_with_label (_("Rooms"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);
  /* gtk_widget_set_sensitive(entry,tray_connected);  */

  submenu= gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (entry), submenu);
  gtk_widget_set_sensitive(submenu,tray_connected);

  entry =  gtk_image_menu_item_new_with_label (_(" Join Chat Room... "));
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_JUMP_TO, GTK_ICON_SIZE_MENU) ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (on_joinaroom), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry =  gtk_image_menu_item_new_with_label (_("Rejoin Current Chat Room"));
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_UNDO, GTK_ICON_SIZE_MENU) ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (on_rejoin_room), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry =  gtk_image_menu_item_new_with_label (_("Leave Room or Conference"));
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_QUIT, GTK_ICON_SIZE_MENU) ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (on_leave_room_or_conf), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry =  gtk_image_menu_item_new_with_label (_("Favorites"));
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU) ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (on_favorites_activate), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry =  gtk_image_menu_item_new_with_label (_("Room List"));
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
  GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_FIND_AND_REPLACE, GTK_ICON_SIZE_MENU) ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (on_room_list_activate), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry =  gtk_image_menu_item_new_with_label (_("Enable Voice Chat"));
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_YES, GTK_ICON_SIZE_MENU) ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (on_py_voice_enable), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry =  gtk_image_menu_item_new_with_label (_("Launch pY! Voice Chat"));
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_CDROM, GTK_ICON_SIZE_MENU) ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (on_launch_py_voice), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);


  entry = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);

  snprintf(menbuf,53, "%s...",_(" Instant Message "));
  entry =  gtk_image_menu_item_new_with_label (menbuf);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_NEW, GTK_ICON_SIZE_MENU) ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (on_blank_pm_window), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);
    gtk_widget_set_sensitive(entry,tray_connected);

  entry =  gtk_image_menu_item_new_with_label (_(" Get Profile... "));
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_MENU) ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (on_open_profile_cb), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);
    gtk_widget_set_sensitive(entry,tray_connected);


  entry =  gtk_image_menu_item_new_with_label (_(" Yahoo! Mail "));
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_FIND, GTK_ICON_SIZE_MENU) ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (on_yahoo_mail_act), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);

  snprintf(menbuf,53, "%s...",_("Setup"));
  entry =  gtk_image_menu_item_new_with_label (menbuf);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_PREFERENCES, GTK_ICON_SIZE_MENU) ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (on_setup_activate), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);



  entry = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);

  snprintf(menbuf,53, "%s...",_("Connect"));
  entry =  gtk_image_menu_item_new_with_label (menbuf);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_YES, GTK_ICON_SIZE_MENU) ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (on_connect_activate), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);
  if (tray_connected) {gtk_widget_set_sensitive(entry,0);}

  entry =  gtk_image_menu_item_new_with_label (_("Disconnect"));
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_NO, GTK_ICON_SIZE_MENU) ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (on_disconnect_activate), NULL);
  gtk_widget_set_sensitive(entry,tray_connected);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);

  entry = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);

  entry =  gtk_image_menu_item_new_with_label (_("Quit"));
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_QUIT, GTK_ICON_SIZE_MENU) ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (on_quit_activate), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);


  entry = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);

  entry =  gtk_menu_item_new_with_label (tray_mailcount);
  gtk_widget_set_sensitive(entry,tray_connected);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);

  entry =  gtk_menu_item_new_with_label (tray_budcount);
  gtk_widget_set_sensitive(entry,tray_connected);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);


  gtk_widget_show_all (menu);
  gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, event->button, event->time);
}

static void systray_clicked (GtkWidget *button, GdkEventButton *event, void *data)
{
  switch (event->button) {
  case 1: {
	GtkWidget *w;
	w = (GtkWidget *)gtk_window_list_toplevels ()->data;
	if (GTK_WIDGET_VISIBLE (w)) {
	  gtk_widget_hide(w);
	} else {
	gtk_widget_show_all (w);
	}
	break;
  }
  case 2:
	break;
  case 3:
	systray_menu (event);
	break;
  }
}

static void systray_update_icon ()
{
  if (! systray) {return;}

  else {
  char **picon=NULL;
  switch (tray_current_status) {
  case TRAY_ICON_OFFLINE:
	picon=(char **)pixmap_pm_tux;
	break;
  case TRAY_ICON_ONLINE_AVAILABLE:
	picon=(char **)pixmap_buddy_online;
	break;
  case TRAY_ICON_INVISIBLE:
	picon=(char **)pixmap_buddy_offline;
	break;
  case TRAY_ICON_IDLE:
	picon=(char **)pixmap_buddy_idle;
	break;
  case TRAY_ICON_BUSY:
	picon=(char **)pixmap_buddy_away;
	break;
  default:
	picon=(char **)pixmap_pm_tux;
  }

	if (tray_have_mail) {picon=(char **)pixmap_mail;}	
	if (imbuf) {g_object_unref(imbuf);}
	imbuf=gdk_pixbuf_new_from_xpm_data((const char**)picon);
        gtk_image_set_from_pixbuf (GTK_IMAGE(trayimage),imbuf);

    } /* end else */
}

static void systray_embedded (GtkWidget *widget, void *data) { }

static void systray_remove_callbacks ()
{
  int ig=0;
  while (g_source_remove_by_user_data (&systray)) {
	ig++;
  }
}

static void systray_destroyed (GtkWidget *widget, void *data)
{
  systray_remove_callbacks ();
  g_object_unref (G_OBJECT (systray));
  systray = NULL;
}


gboolean trayicon_destroy (void)
{
  if (systray) {
	gtk_widget_destroy (GTK_WIDGET (systray));
   }   
  return FALSE;
}


void update_tray_tooltips()  {	
	char tray_tooltip[160];
  	if (!enable_trayicon) {return ;}
	if (!systray) {return;}
	snprintf(tray_tooltip, 158, "Gyach-E: %s\n%s\n%s",
		tray_connected? _("Online"): _("Offline"), 
		tray_mailcount, tray_budcount
			);

	set_tooltip(GTK_WIDGET (systray), tray_tooltip);
	if (!trayimage) {return;}
	set_tooltip(GTK_WIDGET (trayimage), tray_tooltip);
}


gboolean trayicon_create (void)
{
  
  if (!enable_trayicon) {return FALSE;}

  else {
	GtkWidget *box;

  if (systray) {
	/* printf ("Systray: Already created!\n");   */
	return FALSE;
  }

  systray = tray_icon_new ("GyachETrayIcon");
  box = gtk_event_box_new ();
  trayimage = gtk_image_new ();

  g_signal_connect (G_OBJECT (systray), "embedded", G_CALLBACK (systray_embedded), NULL);
  g_signal_connect (G_OBJECT (systray), "destroy", G_CALLBACK (systray_destroyed), NULL);
  g_signal_connect (G_OBJECT (box), "button-press-event", G_CALLBACK (systray_clicked), NULL);

  gtk_container_add (GTK_CONTAINER (box), trayimage);
  gtk_container_add (GTK_CONTAINER (systray), box);
  gtk_widget_show_all (GTK_WIDGET (systray));



  g_object_ref (G_OBJECT (systray));
  systray_update_icon ();

  if (strlen(tray_budcount)<2) {
	snprintf(tray_budcount,53, " %s : %d      %s : %d", 
		_("Friends"), 0, _("Online"), 0
			);
			   						}

  if (strlen(tray_mailcount)<2) {
	snprintf(tray_mailcount,30, " %s :  %d", _("E-mail Messages"), 0	);
			   						}
	
	update_tray_tooltips();

	} /* end else */
  return FALSE;
}


/* Turn mail alerts off after 90 seconds, and go back to whatever our
    regular tray icon was */

int tray_mail_alert_off(guint data) {
	tray_have_mail=0; 
	systray_update_icon ();
	update_tray_tooltips();
	return 0;
}

void tray_mail_alert_timeout() {
    gtk_timeout_add( 75000, (void *)tray_mail_alert_off, NULL );
}

void trayicon_mail_alert() { 
   tray_have_mail=1; 
   systray_update_icon ();
   update_tray_tooltips();
   tray_mail_alert_timeout(); 
}

void trayicon_mail_count(char *mailcount) {
   snprintf(tray_mailcount,30, " %s :  %s", _("E-mail Messages"),mailcount);
   if (menu) { gtk_widget_destroy (menu); menu=NULL;}  /* we need to rebuild menu */
   update_tray_tooltips();
}

void trayicon_buddy_count(int buddycount, int buddyonline) { 
	snprintf(tray_budcount,53, " %s : %d      %s : %d", 
		_("Friends"), buddycount, _("Online"), buddyonline
			);
	if (menu) { gtk_widget_destroy (menu); menu=NULL;}  /* we need to rebuild menu */
	update_tray_tooltips();	
}

void trayicon_set_connected(int connected) {
	tray_connected=connected; 

	/* We only need to rebuild the menu when our connection status changes...
	    since the sensitivity of some menu widgets changes based on whether 
            we are connected, otherwise, we can re-use an existing menu */

	if (menu) { gtk_widget_destroy (menu); menu=NULL;}  /* we need to rebuild menu */
	if (!connected) {
		trayicon_set_status(TRAY_ICON_OFFLINE);
		snprintf(tray_budcount,53, " %s : %d      %s : %d", _("Friends"), 0, _("Online"), 0);
		snprintf(tray_mailcount,30, " %s :  %d", _("E-mail Messages"), 0	);
					  }
	update_tray_tooltips();
}

void trayicon_set_status(int tray_status) {
	if (tray_current_status==tray_status) {return;}
	tray_current_status= tray_status;
	systray_update_icon ();
}



