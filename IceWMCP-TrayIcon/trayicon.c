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
 * Copyright (C) 2004 Erica Andrews 
 * (PhrozenSmoke ['at'] yahoo.com)
 * http://icesoundmanager.sourceforge.net
 *
 * Released under the terms of the GPL.
 * *NO WARRANTY*
 *
 * IceWMCP: Support for FreeDesktop (IceWM/Gnome/KDE) system tray icons
 *****************************************************************************/


#include <sys/types.h>
#include <unistd.h>
#include <libintl.h>
#include <locale.h>
#include <string.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <gtk/gtkplug.h>
#include <gdk/gdkx.h>


#include "trayicons.h"


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
static GdkPixbuf *load_pix(int which_pix);
static TrayIcon *systray = NULL;
static GtkWidget *trayimage = NULL;
static gchar *current_command=NULL;
static char command_buf[40]="";
static   char menbuf[55]="";
static  GdkPixbuf *imbuf=NULL;
static  char **picon=NULL;
static GtkWidget *menu = NULL;
GtkWidget *submenu = NULL;
static GtkWidget *dvbox = NULL;
static GtkWidget *dlabel = NULL;	
static GtkWidget *dbutton = NULL;
static GtkWidget *about_window=NULL;
static char mymsg[355]="";

static char **icewmcp_tray_icon = icewmcp_tray_icon_xpm;
static char **icewmcp_energystar =icewmcp_energystar_xpm;
static char **icewmcp_gtkpccard =icewmcp_gtkpccard_xpm;
static char **icewmcp_iceme =icewmcp_iceme_xpm;
static char **icewmcp_icepref_td =icewmcp_icepref_td_xpm;
static char **icewmcp_icepref =icewmcp_icepref_xpm;
static char **icewmcp_iconselection =icewmcp_iconselection_xpm;
static char **icewmcp_ism =icewmcp_ism_xpm;
static char **icewmcp_keyboard =icewmcp_keyboard_xpm;
static char **icewmcp_mouse =icewmcp_mouse_xpm;
static char **icewmcp_phrozenclock =icewmcp_phrozenclock_xpm;
static char **icewmcp_pyspool =icewmcp_pyspool_xpm;
static char **icewmcp_hwsystem =icewmcp_system_xpm;
static char **icewmcp_wallpaper =icewmcp_wallpaper_xpm;
static char **icewmcp_winoptions =icewmcp_winoptions_xpm;

static char *TRAY_ICON_VERSION="0.1";


char *_(char *some) 
{
	return gettext(some);
}


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

  display = gdk_display;

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
  xdisplay = gdk_display;

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
  char buffer[256];
  GdkWindow *root_window;

  g_return_val_if_fail (xscreen != NULL, NULL);

  icon = g_object_new (TYPE_TRAY_ICON, NULL);
  gtk_window_set_title (GTK_WINDOW (icon), name);

  gtk_plug_construct (GTK_PLUG (icon), 0);

  gtk_widget_realize (GTK_WIDGET (icon));
  g_snprintf (buffer, sizeof (buffer), "_NET_SYSTEM_TRAY_S%d", XScreenNumberOfScreen (xscreen));
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
  icon = tray_icon_new_for_xscreen (DefaultScreenOfDisplay (gdk_display), name);
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
	xdisplay = gdk_display;
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



int icewmcp_system( char *command ) {
	int pid;
	int status;

	if ( ! command ) {
		return( 1 );
	}

	pid = fork();
	if ( pid == -1 ) {
		return( -1 );
	} else if ( pid == 0 ) {
		char *argv[4];
		
		argv[0] = "sh";
		argv[1] = "-c";
		snprintf(command_buf,38,"\"%s\" &", command);
		argv[2] = command_buf;
		argv[3] = 0;

		execv( "/bin/sh", argv );
		exit( 127 );
	} else {
		while( 1 ) {
			if ( waitpid( pid, &status, 0 ) == -1 ) {
				return( -1 );
			} else {
				return( status );
			}
		}
	}
}


void tray_cb1 (GtkMenuItem *menuitem, gpointer  user_data) {
	if (current_command) {g_free(current_command);}
	current_command=g_strdup(user_data);
	icewmcp_system( current_command);
	g_free(current_command);
	current_command=g_strdup("");
}

void on_quit_activate (GtkMenuItem *menuitem, gpointer  user_data) {
	gtk_main_quit();
}


gboolean on_close_ok_dialog  (GtkWidget  *widget,   gpointer   user_data)
{
	if (about_window)  {
		gtk_widget_destroy(about_window);
						}
	return( TRUE );
}

gboolean on_close_ok_dialogw   (GtkWidget  *widget,   GdkEvent *event,  gpointer user_data)
{
	return on_close_ok_dialog(widget,user_data);
}

void show_ok_about_box(GtkMenuItem *menuitem, gpointer  user_data)  {

  about_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_object_set_data (GTK_OBJECT (about_window), "mywindow", about_window);
  gtk_window_set_title (GTK_WINDOW (about_window), _("About"));
  gtk_window_set_policy (GTK_WINDOW (about_window), TRUE, TRUE, FALSE);
  gtk_window_set_position (GTK_WINDOW (about_window), GTK_WIN_POS_MOUSE);
  gtk_window_set_modal (GTK_WINDOW (about_window), TRUE);
  dvbox = gtk_vbox_new (FALSE, 2);
  gtk_container_add (GTK_CONTAINER (about_window), dvbox); 
  gtk_container_set_border_width (GTK_CONTAINER (dvbox), 6);
  dlabel=gtk_label_new("");

  snprintf(mymsg,353,"IceWMCP TrayIcon v. %s\n\n%s\n\nCopyright (C) 2004 Erica Andrews\nPhrozenSmoke ['at'] yahoo.com\nhttp://icesoundmanager.sourceforge.net\n\nLicense: GNU General Public License", TRAY_ICON_VERSION, _("A simple tray icon for icewmtray and other FreeDesktop.org-compliant system trays."));

  gtk_label_set_text(GTK_LABEL(dlabel),mymsg); 
  gtk_label_set_line_wrap(GTK_LABEL(dlabel),1);
  dbutton=gtk_button_new_with_label(_("OK"));
  gtk_object_set_data (GTK_OBJECT (dbutton), "mywindow", about_window);
  gtk_box_pack_start (GTK_BOX (dvbox), dlabel, TRUE, TRUE, 4);
  gtk_box_pack_start (GTK_BOX (dvbox), dbutton, FALSE, FALSE, 8);
  gtk_signal_connect (GTK_OBJECT (dbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_close_ok_dialogw), NULL);
  gtk_signal_connect (GTK_OBJECT (about_window), "delete_event",
                      GTK_SIGNAL_FUNC (on_close_ok_dialogw), NULL);
about_window=about_window;
gtk_widget_show_all(about_window);
}




static void systray_menu (GdkEventButton *event)
{
  GtkWidget *entry;

  if (menu) {
  	gtk_widget_show_all (menu);
  	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, event->button, event->time);
	return;
	/* Speed up showing of menu by using existing menu
	   instead of destroy and rebuilding, PhrozenSmoke */
	/* gtk_widget_destroy (menu); */
  }

  menu = gtk_menu_new ();

  snprintf(menbuf,53, "%s...",_("IceWMCP Tray Icon"));
  entry =  gtk_image_menu_item_new_with_label (menbuf);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_HOME, GTK_ICON_SIZE_MENU) ));
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);

  submenu= gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (entry), submenu);

  snprintf(menbuf,53, "%s...",_("About IceWMCP Tray Icon"));
  entry =  gtk_image_menu_item_new_with_label (menbuf);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
  GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_MENU) ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (show_ok_about_box), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry =  gtk_image_menu_item_new_with_label (_("Close IceWMCP Tray Icon"));
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DIALOG_ERROR, GTK_ICON_SIZE_MENU) ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (on_quit_activate), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);


  entry = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);

  snprintf(menbuf,53, "%s...",_("Tools"));
  entry =  gtk_image_menu_item_new_with_label (menbuf);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_SELECT_COLOR, GTK_ICON_SIZE_MENU) ));
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);

  submenu= gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (entry), submenu);

  entry =  gtk_image_menu_item_new_with_label (_("IceWM Theme Designer"));
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
  	GTK_WIDGET(gtk_image_new_from_pixbuf (load_pix(4))   ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb1), "icepref-td");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry =  gtk_image_menu_item_new_with_label (_("Icon Browser"));
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
  	GTK_WIDGET(gtk_image_new_from_pixbuf (load_pix(6))   ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb1), "IceWMCP-Icons");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

	/* printf ("pyfind: %d ", HAVE_PLUGIN_PYFIND);
	fflush(stdout); */

if ( HAVE_PLUGIN_PYFIND) {
  entry = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  snprintf(menbuf,53, "%s...",_("Find Files"));
  entry =  gtk_image_menu_item_new_with_label (menbuf);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_ZOOM_IN, GTK_ICON_SIZE_MENU) ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb1), "IceWMCP-PyFind2");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);
  /* REMOVE BELOW when PyFind2 Plugin is available */
  gtk_widget_set_sensitive(entry,0);
								}


  entry = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);

  snprintf(menbuf,53, "%s :",_("System Configuration"));
  entry =  gtk_menu_item_new_with_label (menbuf);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);

  entry =  gtk_image_menu_item_new_with_label (_("Date & Time"));
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
  	GTK_WIDGET(gtk_image_new_from_pixbuf (load_pix(10))   ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb1), "PhrozenClock");
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);

  entry =  gtk_image_menu_item_new_with_label (_("EnergyStar"));
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
  	GTK_WIDGET(gtk_image_new_from_pixbuf (load_pix(1))   ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb1), "IceWMCP-EnergyStar");
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);

  entry =  gtk_image_menu_item_new_with_label (_("PC Cards"));
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
  	GTK_WIDGET(gtk_image_new_from_pixbuf (load_pix(2))   ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb1), "GtkPCCard");
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);

  entry =  gtk_image_menu_item_new_with_label (_("Printer Queue"));
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
  	GTK_WIDGET(gtk_image_new_from_pixbuf (load_pix(11))   ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb1), "IceWMCP-PySpool");
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);

	/* printf ("system: %d ", HAVE_PLUGIN_SYSTEM);
	fflush(stdout);  */

if (HAVE_PLUGIN_SYSTEM) {
  entry =  gtk_image_menu_item_new_with_label (_("System & Hardware"));
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
  	GTK_WIDGET(gtk_image_new_from_pixbuf (load_pix(12))   ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb1), "IceWMCP-System");
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);
								}

  entry = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);


  snprintf(menbuf,53, "%s :",_("IceWM Preferences"));
  entry =  gtk_menu_item_new_with_label (menbuf);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);

  snprintf(menbuf,53, "%s...",_("Configuration"));
  entry =  gtk_image_menu_item_new_with_label (menbuf);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_PREFERENCES, GTK_ICON_SIZE_MENU) ));
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);

  submenu= gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (entry), submenu);

  entry = gtk_image_menu_item_new_with_label ("IcePref2");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
  	GTK_WIDGET(gtk_image_new_from_pixbuf (load_pix(5))   ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb1), "icepref");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  entry = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);


  snprintf(menbuf,53, "%s... ",_("Address Bar"));
  entry = gtk_image_menu_item_new_with_label (menbuf);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
  	GTK_WIDGET(gtk_image_new_from_pixbuf (load_pix(5))   ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb1), "\"icepref module=Address Bar\"");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  snprintf(menbuf,53, "%s... ",_("Desktop"));
  entry = gtk_image_menu_item_new_with_label (menbuf);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
  	GTK_WIDGET(gtk_image_new_from_pixbuf (load_pix(5))   ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb1), "\"icepref module=Desktop\"");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  snprintf(menbuf,53, "%s... ",_("Dialogs"));
  entry = gtk_image_menu_item_new_with_label (menbuf);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
  	GTK_WIDGET(gtk_image_new_from_pixbuf (load_pix(5))   ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb1), "\"icepref module=Dialogs\"");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  snprintf(menbuf,53, "%s... ",_("Key Bindings"));
  entry = gtk_image_menu_item_new_with_label (menbuf);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
  	GTK_WIDGET(gtk_image_new_from_pixbuf (load_pix(5))   ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb1), "\"icepref module=Key Bindings\"");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  snprintf(menbuf,53, "%s... ",_("Menus"));
  entry = gtk_image_menu_item_new_with_label (menbuf);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
  	GTK_WIDGET(gtk_image_new_from_pixbuf (load_pix(5))   ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb1), "\"icepref module=Menus\"");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  snprintf(menbuf,53, "%s... ",_("Miscellaneous"));
  entry = gtk_image_menu_item_new_with_label (menbuf);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
  	GTK_WIDGET(gtk_image_new_from_pixbuf (load_pix(5))   ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb1), "\"icepref module=Miscellaneous\"");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  snprintf(menbuf,53, "%s... ",_("Mouse"));
  entry = gtk_image_menu_item_new_with_label (menbuf);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
  	GTK_WIDGET(gtk_image_new_from_pixbuf (load_pix(5))   ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb1), "\"icepref module=Mouse\"");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  snprintf(menbuf,53, "%s... ",_("Quick Switch"));
  entry = gtk_image_menu_item_new_with_label (menbuf);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
  	GTK_WIDGET(gtk_image_new_from_pixbuf (load_pix(5))   ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb1), "\"icepref module=Quick Switch\"");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  snprintf(menbuf,53, "%s... ",_("Scroll Bars"));
  entry = gtk_image_menu_item_new_with_label (menbuf);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
  	GTK_WIDGET(gtk_image_new_from_pixbuf (load_pix(5))   ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb1), "\"icepref module=Scroll Bars\"");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  snprintf(menbuf,53, "%s... ",_("Task Bar"));
  entry = gtk_image_menu_item_new_with_label (menbuf);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
  	GTK_WIDGET(gtk_image_new_from_pixbuf (load_pix(5))   ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb1), "\"icepref module=Task Bar\"");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  snprintf(menbuf,53, "%s... ",_("Task Bar Applets"));
  entry = gtk_image_menu_item_new_with_label (menbuf);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
  	GTK_WIDGET(gtk_image_new_from_pixbuf (load_pix(5))   ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb1), "\"icepref module=Task Bar Applets\"");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  snprintf(menbuf,53, "%s... ",_("Theme"));
  entry = gtk_image_menu_item_new_with_label (menbuf);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
  	GTK_WIDGET(gtk_image_new_from_pixbuf (load_pix(5))   ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb1), "\"icepref module=Themes\"");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  snprintf(menbuf,53, "%s... ",_("Title Bars"));
  entry = gtk_image_menu_item_new_with_label (menbuf);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
  	GTK_WIDGET(gtk_image_new_from_pixbuf (load_pix(5))   ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb1), "\"icepref module=Title Bars\"");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  snprintf(menbuf,53, "%s... ",_("Tool Tips"));
  entry = gtk_image_menu_item_new_with_label (menbuf);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
  	GTK_WIDGET(gtk_image_new_from_pixbuf (load_pix(5))   ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb1), "\"icepref module=Tool Tips\"");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  snprintf(menbuf,53, "%s... ",_("Windows"));
  entry = gtk_image_menu_item_new_with_label (menbuf);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
  	GTK_WIDGET(gtk_image_new_from_pixbuf (load_pix(5))   ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb1), "\"icepref module=Windows\"");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);

  snprintf(menbuf,53, "%s... ",_("Workspaces"));
  entry = gtk_image_menu_item_new_with_label (menbuf);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
  	GTK_WIDGET(gtk_image_new_from_pixbuf (load_pix(5))   ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb1), "\"icepref module=Workspaces\"");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), entry);



  entry =  gtk_image_menu_item_new_with_label (_("Desktop"));
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
  	GTK_WIDGET(gtk_image_new_from_pixbuf (load_pix(13))   ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb1), "IceWMCP-Wallpaper");
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);

  entry =  gtk_image_menu_item_new_with_label (_("Menus"));
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
  	GTK_WIDGET(gtk_image_new_from_pixbuf (load_pix(3))   ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb1), "iceme");
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);

  entry =  gtk_image_menu_item_new_with_label (_("Mouse"));
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
  	GTK_WIDGET(gtk_image_new_from_pixbuf (load_pix(9))   ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb1), "IceWMCP-Mouse");
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);

  entry =  gtk_image_menu_item_new_with_label (_("Keyboard"));
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
  	GTK_WIDGET(gtk_image_new_from_pixbuf (load_pix(8))   ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb1), "IceWMCP-Keyboard");
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);

  entry =  gtk_image_menu_item_new_with_label (_("Sound Events"));
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
  	GTK_WIDGET(gtk_image_new_from_pixbuf (load_pix(7))   ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb1), "IceSoundManager");
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);

  entry =  gtk_image_menu_item_new_with_label (_("Window Options"));
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
  	GTK_WIDGET(gtk_image_new_from_pixbuf (load_pix(14))   ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb1), "IceWMCP-WinOptions");
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);


  entry = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);

  snprintf(menbuf,53, "%s...",_("IceWM Control Panel"));
  entry =  gtk_image_menu_item_new_with_label (menbuf);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(entry), 
  GTK_WIDGET(gtk_image_new_from_pixbuf (load_pix(0))   ));
  g_signal_connect (G_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (tray_cb1), "IceWMCP");
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);

  entry = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);

  entry =  gtk_menu_item_new_with_label (_("IceWMCP Tray"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);

  gtk_widget_show_all (menu);
  gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, event->button, event->time);
}

static void systray_clicked (GtkWidget *button, GdkEventButton *event, void *data)
{
  switch (event->button) {
  case 1:
	systray_menu (event);
	break;
  case 2:
	break;
  case 3:
	systray_menu (event);
	break;
  }
}

static GdkPixbuf *load_pix(int which_pix) {
	switch (which_pix) {
  		case 0:
			picon=(char **)icewmcp_tray_icon;
			break;
  		case 1:
			picon=(char **)icewmcp_energystar;
			break;
  		case 2:
			picon=(char **)icewmcp_gtkpccard;
			break;
  		case 3:
			picon=(char **)icewmcp_iceme;
			break;
  		case 4:
			picon=(char **)icewmcp_icepref_td;
			break;
  		case 5:
			picon=(char **)icewmcp_icepref;
			break;
  		case 6:
			picon=(char **)icewmcp_iconselection;
			break;
  		case 7:
			picon=(char **)icewmcp_ism;
			break;
  		case 8:
			picon=(char **)icewmcp_keyboard;
			break;
  		case 9:
			picon=(char **)icewmcp_mouse;
			break;
  		case 10:
			picon=(char **)icewmcp_phrozenclock;
			break;
  		case 11:
			picon=(char **)icewmcp_pyspool;
			break;
  		case 12:
			picon=(char **)icewmcp_hwsystem;
			break;
  		case 13:
			picon=(char **)icewmcp_wallpaper;
			break;
  		case 14:
			picon=(char **)icewmcp_winoptions;
			break;
		default:
			picon=(char **)icewmcp_tray_icon;
	}
	if (imbuf) {gdk_pixbuf_unref(imbuf);}
	imbuf=gdk_pixbuf_new_from_xpm_data((const char**)picon);
	return imbuf;
}

static void systray_update_icon ()
{
  if (! systray) {return;}
  else {
        gtk_image_set_from_pixbuf (GTK_IMAGE(trayimage),load_pix(0));
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

gboolean trayicon_create (void)
{  
   GtkWidget *box;

  if (systray) {
	/* printf ("Systray: Already created!\n");   */
	return FALSE;
  }

  systray = tray_icon_new ("IceWMCPTrayIcon");
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
  return FALSE;
}

int main( int argc, char **argv ) {
	setlocale(LC_ALL, "");
	bindtextdomain ("icewmcp-trayicon", LOCALEDIR);  
	bind_textdomain_codeset("icewmcp-trayicon", "UTF-8");
	textdomain ("icewmcp-trayicon");
	gtk_set_locale();
	gtk_init( &argc, &argv );
	trayicon_create();
	gtk_main();
	return( 0 );
}

