/*****************************************************************************
 * gytreeview.h
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
 * Wrappers for GtkTreeView stuff
 *****************************************************************************/

#ifndef _GYTREEVIEW_H_
#define _GYTREEVIEW_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <gtk/gtk.h>

#define GYTV_FIRST_COL 0
#define GYTV_TYPE_LIST 1
#define GYTV_TYPE_TREE 2


/* For single-columned list and trees that may or may not have Pixbuf objects...
	Some Setup lists, Ignore list, Conference lists, fader list, Favorites, 
	Create Room/Room list, the first is specifically for room lists  */

#define GYLIST_TYPE_ROOM 0
enum
{
  GYROOM_PIX_TOGGLE = 0,
  GYROOM_PIX,
  GYROOM_COLOR,
  GYROOM_COL1, 
  GYROOM_DATA,   /* ROOM_ENTRY pointer */
  GYROOM_NUMCOLS
} ;

#define GYLIST_TYPE_SINGLE 1
enum
{
  GYSINGLE_PIX_TOGGLE = 0,
  GYSINGLE_PIX,
  GYSINGLE_COLOR,
  GYSINGLE_COL1, 
  GYSINGLE_TOP,   /* Top level or not */
  GYSINGLE_NUMCOLS
} ;


/* Double-columned lists such as the Buddy list, which may or may not have Pixbufs */

#define GYLIST_TYPE_DOUBLE 2
enum
{
  GYDOUBLE_AVA_TOGGLE = 0,
  GYDOUBLE_AVA,  /* Avatar pixbuf */
  GYDOUBLE_PIX_TOGGLE,
  GYDOUBLE_PIX,
  GYDOUBLE_COLOR,
  GYDOUBLE_LABEL,  /* display name */
  GYDOUBLE_COL1,  /* raw 'name' value */
  GYDOUBLE_COL2,   /* buddy status */
  GYDOUBLE_TOP,  /* info about toplevel stuff */

  GYDOUBLE_STEALTH_TOGGLE,
  GYDOUBLE_STEALTH_PIX,

  GYDOUBLE_SMS_TOGGLE,
  GYDOUBLE_SMS_PIX,

  GYDOUBLE_CHAT_TOGGLE,
  GYDOUBLE_CHAT_PIX,

  GYDOUBLE_WEBCAM_TOGGLE,
  GYDOUBLE_WEBCAM_PIX,

  GYDOUBLE_GAMES_TOGGLE,
  GYDOUBLE_GAMES_PIX,
  GYDOUBLE_GAMES_COL,

  GYDOUBLE_LC_TOGGLE,  /* launchcast */
  GYDOUBLE_LC_PIX,
  GYDOUBLE_LC_COL,

  GYDOUBLE_NUMCOLS
} ;

/* Triple-columned list (right now only Alias list), no Pixbuf */

#define GYLIST_TYPE_TRIPLE 3
enum
{
  GYTRIPLE_COL1 = 0,
  GYTRIPLE_COL2, 
  GYTRIPLE_COL3, 
  GYTRIPLE_NUMCOLS
} ;

/* 4-columned list (right now only Chat user list), un-embedded Pixbuf always on */

#define GYLIST_TYPE_QUAD 4
enum
{
  GYQUAD_PIX_TOGGLE = 0,
  GYQUAD_PIX,
  GYQUAD_COLOR,
  GYQUAD_COL1, 
  GYQUAD_COL2, 
  GYQUAD_COL3, 
  GYQUAD_NUMCOLS
} ;


gint sort_iter_compare_func (GtkTreeModel *model,
                          GtkTreeIter  *a,
                          GtkTreeIter  *b,
                          gpointer      userdata);
void set_basic_treeview_sorting(GtkWidget *widgy, int gylist_type);
void unfreeze_treeview(GtkWidget *view, GtkTreeModel *model);
GtkTreeModel *freeze_treeview(GtkWidget *view);
void gy_treeview_adjust_columns(GtkWidget *view);
GtkTreeView *create_gy_treeview(int view_type, int gylist_type, int headers_visible, 
	int with_pixbuf,  char **col_headers);
void gy_empty_model(GtkTreeModel *tree, int gylist_type);



#endif /* #ifndef _GYTREEVIEW_H_ */

