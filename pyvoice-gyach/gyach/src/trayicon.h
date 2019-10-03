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

#define TRAY_ICON_OFFLINE 0
#define TRAY_ICON_ONLINE_AVAILABLE 1
#define TRAY_ICON_INVISIBLE 2
#define TRAY_ICON_IDLE 3
#define TRAY_ICON_BUSY 4

extern int enable_trayicon;

extern char *_(char *some);  /* added PhrozenSmoke: locale support */
extern gboolean trayicon_create (void);
extern gboolean trayicon_destroy (void);

extern void trayicon_mail_alert();
extern void trayicon_mail_count(char *mailcount);
extern void trayicon_buddy_count(int buddycount, int buddyonline);
extern void trayicon_set_connected(int connected);
extern void trayicon_set_status(int tray_status);




