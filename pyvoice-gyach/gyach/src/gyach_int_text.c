/*****************************************************************************
 * gyach_int_text.c
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
 * Copyright (C) 2000-2002 Chris Pinkham
 * cpinkham@corp.infi.net, cpinkham@bc2va.org
 * http://www4.infi.net/~cpinkham/gyach/
 * 
 * Released under the terms of the GPL.
 * *NO WARRANTY*
 *
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#ifdef GYACH_INT_TEXT

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#ifdef USE_GDK_PIXBUF
#include <gdk-pixbuf/gdk-pixbuf.h>
#endif

#ifdef USE_PTHREAD_CREATE
#include <pthread.h>
#endif

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "util.h"

#include "gyach.h"
#include "gyach_int.h"

#include "animations.h"

extern gchar *_b2loc(char *some);
extern int show_emoticons; 


GtkTextView *chat_text;
GtkTextBuffer *text_buffer;
GtkTextBuffer *text_buffer22;
GtkTextMark *end_mark;
char chat_font[112] = "fixed 12";
GtkWidget *top_text;
GtkWidget *vpaned;
char *underlined = NULL;
int trans_bg = 0;
int trans_shade_r = 248;
int trans_shade_g = 251;
int trans_shade_b = 254;
GdkColor chat_color;
int max_lines = 0;
char *custom_scale_text=NULL;
char *last_custom_scale_text=NULL;

/* support functiond decls */
gboolean on_chat_text_clicked (GtkWidget *widget, GdkEventButton *event, gpointer user_data);
#ifdef TRANS_BG
gboolean update_transparent_background (GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
gboolean redraw_notify_event (GtkWidget *widget, GdkEvent *event, gpointer user_data);
#endif

/* standard gyach interface function decls */
int ct_build() {
	GtkWidget *hbox;
	GtkTextIter end;
	GtkWidget *sw;
	GtkWidget *frame;

	hbox = lookup_widget( chat_window, "hbox2" );

	chat_text = GTK_TEXT_VIEW(gtk_text_view_new());
	text_buffer = gtk_text_view_get_buffer( chat_text );
	text_buffer22=text_buffer;

	gtk_widget_ref( GTK_WIDGET(chat_text) );
	gtk_object_set_data( GTK_OBJECT(hbox), "chat_text",
		chat_text);

	vpaned = gtk_vpaned_new();
	gtk_container_set_border_width( GTK_CONTAINER(vpaned), 0 );
	gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET(vpaned), TRUE, TRUE, 0);
	top_text = gtk_text_view_new_with_buffer( text_buffer );
	gtk_widget_ref( GTK_WIDGET(top_text) );

	sw = gtk_scrolled_window_new( NULL, NULL );
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN );
	gtk_paned_add1( GTK_PANED (vpaned), frame );
	gtk_container_add (GTK_CONTAINER (frame), sw );
	gtk_container_add( GTK_CONTAINER (sw), top_text );
        gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_IN);
  gtk_text_view_set_left_margin (GTK_TEXT_VIEW( top_text ), 5);
  gtk_text_view_set_right_margin (GTK_TEXT_VIEW( top_text ), 5);

	gtk_paned_set_position( GTK_PANED( vpaned ), 0 );

	sw = gtk_scrolled_window_new( NULL, NULL );
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN );
	gtk_paned_add2( GTK_PANED (vpaned), frame );
	gtk_container_add (GTK_CONTAINER (frame), sw );
	gtk_container_add( GTK_CONTAINER (sw), GTK_WIDGET( chat_text ));
        gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_IN);
  gtk_text_view_set_left_margin (GTK_TEXT_VIEW( chat_text ), 5);
  gtk_text_view_set_right_margin (GTK_TEXT_VIEW( chat_text ), 5);

	gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW( top_text ), GTK_WRAP_WORD );

	gtk_widget_show_all( vpaned );

	gtk_signal_connect (GTK_OBJECT (top_text), "button_press_event",
		GTK_SIGNAL_FUNC (on_chat_text_clicked), NULL);

	gtk_signal_connect_after (GTK_OBJECT (top_text), "motion_notify_event",
		GTK_SIGNAL_FUNC (chat_motion_notify), NULL);

	GTK_WIDGET_UNSET_FLAGS(GTK_WIDGET(top_text), GTK_CAN_FOCUS);

	gtk_widget_set_events( GTK_WIDGET(top_text),
		GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK );

	gtk_widget_set_usize( GTK_WIDGET(chat_text), 480, -2);
	gtk_widget_show( GTK_WIDGET(chat_text) );

	gtk_signal_connect (GTK_OBJECT (chat_text), "button_press_event",
		GTK_SIGNAL_FUNC (on_chat_text_clicked), NULL);

	gtk_signal_connect_after (GTK_OBJECT (chat_text), "motion_notify_event",
		GTK_SIGNAL_FUNC (chat_motion_notify), NULL);

	GTK_WIDGET_UNSET_FLAGS(GTK_WIDGET(chat_text), GTK_CAN_FOCUS);

	gtk_widget_set_events( GTK_WIDGET(chat_text),
		GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK 
#ifdef TRANS_BG
		| GDK_EXPOSURE_MASK
#endif
		);

#	ifdef TRANS_BG
	gtk_signal_connect_after (GTK_OBJECT (chat_text), "expose_event",
		GTK_SIGNAL_FUNC (update_transparent_background), NULL );
	gtk_signal_connect_after (GTK_OBJECT (chat_text), "configure_event",
		GTK_SIGNAL_FUNC (redraw_notify_event), NULL );
	gtk_signal_connect(GTK_OBJECT(chat_text->vadjustment), "value_changed",
		GTK_SIGNAL_FUNC (redraw_notify_event), NULL);
#	endif

	gtk_text_view_set_wrap_mode( chat_text, GTK_WRAP_WORD );
	ct_set_font( "fixed 12" );
	ct_set_color( 0, 0, 0 );  /* This creates necessary tags */

	gtk_text_buffer_get_end_iter( text_buffer, &end );
	end_mark = gtk_text_buffer_create_mark( text_buffer, "end_mark", &end, 0 );

	return( 0 );
}

GtkWidget *ct_widget() {
	return( GTK_WIDGET(chat_text));
}

int ct_scroll_at_bottom() {
	GtkAdjustment *adj;
	/* if we are split and they have adjusted the split then always scroll */
	/* the bottom window to the bottom */
	if ( gtk_paned_get_position( GTK_PANED( vpaned ))) { return( 1 ); }

	adj = chat_text->vadjustment;

	 /* printf("targ: %f\n", (float) ( adj->upper - adj->lower - adj->page_size )); fflush(stdout); 
		printf("adj: %f\n", (float) adj->value ); fflush(stdout);  */ 

	if ( adj->value == ( adj->upper - adj->lower - adj->page_size )) {	return( 1 ); }
	/* not right at the bottom, but very close ... */ 
	if ( adj->value > ( adj->upper - adj->lower - adj->page_size -33 )) {	return( 1 ); }	
	return( 0 );
}

int ct_scroll_to_bottom() {
	GtkTextIter iter;

	gtk_text_buffer_get_iter_at_mark( text_buffer, &iter, end_mark );
	gtk_text_buffer_place_cursor( text_buffer, &iter );
	gtk_text_view_scroll_to_mark( chat_text, end_mark, 0, FALSE, 0, 0 );
	/*  */   gtk_text_view_scroll_mark_onscreen( chat_text, end_mark); 
	return( 0 );
}

int remove_extra_lines() {
	int text_length;
	GtkTextIter start;
	GtkTextIter end;

	text_length = gtk_text_buffer_get_char_count( text_buffer );
	if ( max_lines ) {
		ct_freeze();
		if ( text_length > ( max_lines * 1024 )) {
			int rmtext=max_lines * 1024;
			gtk_text_buffer_get_iter_at_offset( text_buffer, &start, 0 );
			gtk_text_buffer_get_iter_at_offset( text_buffer, &end, (text_length-rmtext)+512 );
			gtk_text_buffer_delete( text_buffer, &start, &end );

		}
		ct_thaw();
	}

	return( 0 );
}

#ifdef TRANS_BG
/**************************************************************************/
/* from xchat */

#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

static GdkPixmap *
shade_pixmap_gdk (GtkTextView *text, Pixmap p, int x, int y, int w, int h)
{
	GdkPixmap *pp, *tmp, *shaded_pixmap;
	GdkPixbuf *pixbuf;
	GdkColormap *cmap;
	GdkGC *tgc;
	unsigned char *buf;
	unsigned char *pbuf;
	int width, height, depth;
	int rowstride;
	int pbwidth;
	int pbheight;
	int i, j;
	int offset;
	int r, g, b, a;

//g_warning("shade-1\n");
	pp = gdk_pixmap_foreign_new (p);
//g_warning("shade-2\n");
	gdk_drawable_get_size (pp, &width, &height);
//g_warning("shade-3\n");
	cmap = gtk_widget_get_colormap (GTK_WIDGET (text));
//g_warning("shade-4\n");
	depth = gdk_drawable_get_depth (pp);
//g_warning("shade-5\n");

	if (width < x + w || height < y + h || x < 0 || y < 0)
	{
		tgc = gdk_gc_new (pp);
		tmp = gdk_pixmap_new (pp, w, h, depth);
		gdk_gc_set_tile (tgc, pp);
		gdk_gc_set_fill (tgc, GDK_TILED);
		gdk_gc_set_ts_origin (tgc, -x, -y);
		gdk_draw_rectangle (tmp, tgc, TRUE, 0, 0, w, h);
		gdk_gc_destroy (tgc);

		pixbuf = gdk_pixbuf_get_from_drawable (NULL, tmp, cmap,
															0, 0, 0, 0, w, h);
		gdk_pixmap_unref (tmp);
	} else
	{
		pixbuf = gdk_pixbuf_get_from_drawable (NULL, pp, cmap,
															x, y, 0, 0, w, h);
	}
	g_free (pp);

	if (!pixbuf)
		return NULL;

	buf = gdk_pixbuf_get_pixels (pixbuf);
	rowstride = gdk_pixbuf_get_rowstride (pixbuf);
	pbwidth = gdk_pixbuf_get_width (pixbuf);
	pbheight = gdk_pixbuf_get_height (pixbuf);

//g_warning("shade-8\n");

	a = 128;	/* alpha */
	r = trans_shade_r;
	g = trans_shade_g;
	b = trans_shade_b;

	if (gdk_pixbuf_get_has_alpha (pixbuf))
		offset = 4;
	else
		offset = 3;

	for (i=0;i<pbheight;i++)
	{
		pbuf = buf;
		for (j=0;j<pbwidth;j++)
		{
			pbuf[0] = ((pbuf[0] * r) >> 8);
			pbuf[1] = ((pbuf[1] * g) >> 8);
			pbuf[2] = ((pbuf[2] * b) >> 8);
			pbuf+=offset;
		}
		buf+=rowstride;
	}

	/* reuse the same pixmap to save a few cycles */
#ifdef RECYCLE
	if (xtext->recycle)
	{
		shaded_pixmap = xtext->pixmap;
		gdk_pixbuf_render_to_drawable (pixbuf, shaded_pixmap, xtext->fgc, 0, 0,
												 0, 0, w, h, GDK_RGB_DITHER_NORMAL, 0, 0);
	} else
#endif
	{
		gdk_pixbuf_render_pixmap_and_mask (pixbuf, &shaded_pixmap, NULL, 0);
	}
	g_object_unref (pixbuf);

	return shaded_pixmap;
}

static Window desktop_window = None;

/* stolen from zvt, which was stolen from Eterm */
static Pixmap get_pixmap_prop ()
{
	Atom type;
	int format;
	unsigned long length, after;
	unsigned char *data;
	Pixmap pix = None;
	static Atom prop = None;

	desktop_window = GDK_ROOT_WINDOW ();

	if (prop == None)
		prop = XInternAtom (GDK_DISPLAY (), "_XROOTPMAP_ID", True);
	if (prop == None)
		return None;

	XGetWindowProperty (GDK_DISPLAY (), desktop_window, prop, 0L, 1L, False,
							  AnyPropertyType, &type, &format, &length, &after,
							  &data);
	if (data)
	{
		if (type == XA_PIXMAP)
			pix = *((Pixmap *) data);

		XFree (data);
	}

	return pix;
}

/**************************************************************************/


gboolean redraw_notify_event (GtkWidget *widget,
		GdkEvent *event, gpointer user_data) {
	if ( trans_bg )
		update_transparent_background( NULL, NULL, "update" );
	return( FALSE );
}

static GdkPixmap *st_pixmap_text = NULL;
static GdkPixmap *st_pixmap_text_top = NULL;

gboolean update_transparent_background (GtkWidget *widget,
		GdkEventExpose *event, gpointer user_data) {
	GdkWindow *gwin;
	GdkWindow *gwin_top;
	Pixmap rootpix;
	int x, y;
	static int orig_x = -1;
	static int orig_y = -1;
	static int orig_w = -1;
	static int orig_h = -1;
	int width, height;
	int redraw = 0;
	int tx, ty, twidth, theight;
	static int orig_pane_pos = -1;
	int pane_pos;

	if ( ! trans_bg )
		return( FALSE );

	//g_warning("trans-1\n");

	gwin = gtk_text_view_get_window( chat_text, GTK_TEXT_WINDOW_TEXT );
	gdk_window_get_deskrelative_origin( gwin, &x, &y);
	gdk_window_get_size( gwin, &width, &height);
	gwin_top = gtk_text_view_get_window( GTK_TEXT_VIEW(top_text), GTK_TEXT_WINDOW_TEXT );

	pane_pos = gtk_paned_get_position( GTK_PANED( vpaned ));

	//g_warning("trans-1a\n");

	if (( x != orig_x ) ||
		( y != orig_y ) ||
		( width != orig_w ) ||
		( height != orig_h ) ||
		( pane_pos != orig_pane_pos ) ||
		( ! st_pixmap_text ) ||
		( user_data )) {
			//g_warning("trans-1aa\n");
		rootpix = get_pixmap_prop();
			//g_warning("trans-1ab\n");
		if ( st_pixmap_text )
			gdk_pixmap_unref( st_pixmap_text );
		//g_warning("trans-1ac: %d %d %d %d\n", x, y, width, height);
		//if (rootpix==None)  {			g_warning("rootpix null\n");}
		st_pixmap_text = shade_pixmap_gdk( chat_text, rootpix, x, y, width, height);
			//g_warning("trans-1ad\n");
		redraw = 1;

	//g_warning("trans-1b\n");

		if ( pane_pos ) {
			gdk_window_get_deskrelative_origin( gwin_top, &tx, &ty);
			gdk_window_get_size( gwin_top, &twidth, &theight);
			if ( st_pixmap_text_top )
				gdk_pixmap_unref( st_pixmap_text_top );
			st_pixmap_text_top = shade_pixmap_gdk( GTK_TEXT_VIEW(top_text), rootpix,
				tx, ty, twidth, theight);

	//g_warning("trans-1c\n");
		}
	}

	//g_warning("trans-2\n");

	if (( x != orig_x ) ||
		( y != orig_y ) ||
		( width != orig_w ) ||
		( height != orig_h ) ||
		( pane_pos != orig_pane_pos ) ||
		( user_data )) {
		if ( pane_pos ) {
			gdk_window_set_back_pixmap( gwin_top, st_pixmap_text_top, FALSE );
			gtk_widget_queue_draw( top_text );
		}

		gdk_window_set_back_pixmap( gwin, st_pixmap_text, FALSE );
		if (( event ) &&
			( ! redraw )) {
			gtk_widget_queue_draw_area( GTK_WIDGET(chat_text),
				event->area.x, event->area.y,
				event->area.width, event->area.height );
		} else {
			gtk_widget_queue_draw( GTK_WIDGET(chat_text));
		}
	}

	//g_warning("trans-3\n");

	orig_x = x;
	orig_y = y;
	orig_w = width;
	orig_h = height;
	orig_pane_pos = pane_pos;

	return( FALSE );
}
#endif /* TRANS_BG */

int ct_append( char *str, int len ) {
	char tmp[128];
	GtkTextIter iter;

	if (( ! len ) ||
		( ! str )) {
		return( 0 );
	}  

	/* sprintf( tmp, "fg_%05d%05d%05d",
		chat_color.red, chat_color.green, chat_color.blue ); */

	  /* create hex string 'ffccff', etc. */
	snprintf( tmp, 126, "%02X%02X%02X|%s|%s", chat_color.red / ( 65535 / 255 ), chat_color.green / ( 65535 / 255 ), chat_color.blue / ( 65535 / 255 ),
	chat_font, underlined?"u1":"u0"	);  /* create hex string 'ffccff', etc. */

	gtk_text_buffer_get_end_iter( text_buffer, &iter );

	/* NOTE: "underlined" must be last in this because it is sometimes set */
	/*       to NULL */
	gtk_text_buffer_insert_with_tags_by_name( text_buffer, &iter, str, len,
		tmp, NULL );

	if ( underlined ) {
		free( underlined );
		underlined = NULL;
	}

#ifdef TRANS_BG
	if ( trans_bg )
		update_transparent_background( NULL, NULL, "update" );
#endif

	remove_extra_lines();

	return( 0 );
}

int ct_append_fixed( char *str, int len ) {
	char orig_font[96];
	char fixed_font[16];
	int pitch;
	char *ptr;

	ptr = chat_font + strlen( chat_font ) - 1;
	while(( ptr > chat_font ) && ( *ptr != ' ' )) {
		ptr--;
	}
	ptr++;
	pitch = atoi( ptr );

	strncpy( orig_font, chat_font, 93 );
	snprintf( fixed_font, 15, "fixed %d", 12 );
	ct_set_font( fixed_font );
	ct_set_color( 0, 0, 0 );
	ct_append( str, len );
	ct_set_font( orig_font );
	return( 0 );
}

int ct_freeze() {
	/* FIX ME */
	/*
	GdkWindow *gwin;
	gwin = gtk_text_view_get_window( chat_text, GTK_TEXT_WINDOW_TEXT );
	gdk_window_freeze_updates( gwin );
	*/
	return( 0 );
}

int ct_thaw() {
	/* FIX ME */

	/*
	GdkWindow *gwin;
	gwin = gtk_text_view_get_window( chat_text, GTK_TEXT_WINDOW_TEXT );
	gdk_window_thaw_updates( gwin );
	*/

	return( 0 );
}


int ct_clear() {
	GtkTextIter start;
	GtkTextIter end;
	int text_length;

	ct_freeze();
	text_length = gtk_text_buffer_get_char_count( text_buffer );
	gtk_text_buffer_get_iter_at_offset( text_buffer, &start, 0 );
	gtk_text_buffer_get_iter_at_offset( text_buffer, &end, text_length );
	gtk_text_buffer_delete( text_buffer, &start, &end );

	gtk_text_buffer_get_end_iter( text_buffer, &end );
	end_mark = gtk_text_buffer_create_mark( text_buffer, "end_mark", &end, 0 );
	ct_thaw();

	return( 0 );
}

GdkColor *ct_get_color() {
	return( &chat_color );
}

char *ct_get_font() {
	return( chat_font );
}


void tag_scale_update(GtkTextTag *texttag, gpointer user_data) {
	if (!texttag) {return;}
	g_object_set(texttag,"scale", (double) atof(custom_scale_text), NULL);
}

void check_tag_table_validity(GtkTextTagTable *tag_table) {
		if (!custom_scale_text) {custom_scale_text=strdup("1.0");}
		if ( ( atof(custom_scale_text) <0.25)   ||  ( atof(custom_scale_text) >2.5)   ) {
			free(custom_scale_text); 
			custom_scale_text=strdup("1.0");
		}
		if (last_custom_scale_text) {
			if (strcmp(last_custom_scale_text,custom_scale_text )) {
				free(last_custom_scale_text);
				last_custom_scale_text=strdup(custom_scale_text);
				gtk_text_tag_table_foreach(tag_table, tag_scale_update, NULL);
			}
		}  else {last_custom_scale_text=strdup(custom_scale_text);}
}

int ct_set_color( int r, int g, int b ) {
	char tmp[128];
	GtkTextTagTable *tag_table;
#ifdef TRANS_BG
	if (( trans_bg ) &&
		( r == 0 ) &&
		( g == 0 ) &&
		( b == 0 )) {
		r = g = b = 255;
	} else
#endif
	/* if person sends white, change to black since background is white */
	if (( r == 0xff ) &&
		( g == 0xff ) &&
		( b == 0xff )) {
		r = g = b = 0xa0;
	}

	chat_color.red   = r * ( 65535 / 255 );
	chat_color.green = g * ( 65535 / 255 );
	chat_color.blue  = b * ( 65535 / 255 );

	snprintf( tmp, 126, "%02X%02X%02X|%s|%s", r,g,b,
	chat_font, underlined?"u1":"u0"	);  /* create hex string 'ffccff', etc. */

	tag_table = gtk_text_buffer_get_tag_table( text_buffer );
	check_tag_table_validity(tag_table);
	if ( ! gtk_text_tag_table_lookup( tag_table, tmp )) {
		GtkTextTag *ttag;	
		ttag=gtk_text_buffer_create_tag( text_buffer, tmp, "foreground-gdk",
			&chat_color, NULL);
		if (underlined) {g_object_set(ttag,"underline",PANGO_UNDERLINE_SINGLE, NULL);}
		g_object_set(ttag,"font",chat_font, NULL);
		g_object_set(ttag,"editable",FALSE, NULL);

			/* Below, this tag 'encourages' use of iso-8859-1 charsets, suitable 
			for  English and most Euro languages, avoids most Gtk-related 
			warnings about trying to load the wrong charset for the font, so assume
			English */
		g_object_set(ttag,"language","en_US.ISO8859-1", NULL);
		/* if (strcmp(custom_scale_text,"1.0")) { */  
			 g_object_set(ttag,"scale", (double) (atof(custom_scale_text)), NULL);
		/* }   */
	}

	/* printf("Size:  %d\n", gtk_text_tag_table_get_size(tag_table)); fflush(stdout); */ 

	return( 0 );
}

void ct_set_underline( int on ) {
	if ( on ) {
		if ( ! underlined ) {
			underlined = strdup( "underlined" );
		}
	} else {
		if ( underlined ) {
			free( underlined );
			underlined = NULL;
		}
	}
}

int ct_set_font( char *new_font ) {
	snprintf( chat_font,110, "%s", new_font );
	return( 0 );
}

int ct_set_max_lines( int lines ) {
	if ( lines >= 0 )
		max_lines = lines;
	else
		max_lines = 0;

	return( 0 );
}

int ct_capture_to_file( char *filename ) {
	FILE *fp;
	int  length;
	GtkTextIter start;
	GtkTextIter end;
	gchar *ptr;
	gchar *capb2loc;
	time_t time_llnow= time(NULL);

	if (!filename) {return 0;}
	fp = fopen( filename, "ab" );

	if ( ! fp )  {
		show_ok_dialog(_("Error opening file for saving."));
		return( 0 );

				}

	length = gtk_text_buffer_get_char_count( text_buffer );

			fprintf( fp, "\nGyach Enhanced, version %s\nCopyright (c) 2003-2006, Erica Andrews\n%s\nLicense: GNU General Public License\n\nCHAT/CONFERENCE SESSION LOG\n", VERSION, GYACH_URL );
			fprintf( fp, "%s", ctime(&time_llnow ) );
			fprintf( fp, "\n_____________________\n\n" );
			fflush( fp );

	gtk_text_buffer_get_iter_at_offset( text_buffer, &start, 0 );
	gtk_text_buffer_get_iter_at_offset( text_buffer, &end, length );
	ptr = gtk_text_buffer_get_text( text_buffer, &start, &end, 0 );
	capb2loc=_b2loc(ptr);
	fwrite( capb2loc, 1, strlen(capb2loc), fp );
	g_free( ptr );

	fprintf( fp, "\n" );
	fclose( fp );

	return( 0 );
}

int ct_can_do_pixmaps( void ) {
	return( 1 );
}


/* changed here: PhrozenSmoke, to allow for smileys in PM windows */

int ct_append_pixmap( char *filename, GtkWidget *textbox ) {
	static char smtmpfilename[256];
	GdkPixbuf *pixbuf;
	GdkPixbuf *spixbuf;
	int	width;
	int height;
	int text_length;
	GtkTextIter iter;
	GtkTextBuffer *t_buffer;
	int is_scentral_smiley=0;
	int scalepix=0;
	int anim_mxsz=40;
	int local_image=0;

	if (filename==NULL) {return 0;}

	if (! show_emoticons) {
		if (strstr(filename,"/smileys/"))  {return 0;}
									}  

/* added: PhrozenSmoke, special handling of animated gifs to allow animated smileys */
	strncpy(smtmpfilename, filename, 254);	

	/* Added support for the growingly common SmileyCentral smileys, 
	   downloadable from the web */

	if (!strncasecmp("scnt://", filename, 7) ) {
		char *smweb_pic="";
		smweb_pic=download_image(filename);
		if (!smweb_pic) {return 0;}
		if (!strcmp(smweb_pic,"")) { return 0; }
		strncpy(smtmpfilename, smweb_pic, 254);
		is_scentral_smiley=1;
	}

	if (!is_scentral_smiley) {
		if ( (! strstr(filename,"/avatars/")) && 
		(! strstr(filename,"/smileys/")) && 
		(! strstr(filename,"/pixmaps/")) ) {
			/* viewing a local image on hard drive */ 
			anim_mxsz=300;
			local_image=1;
			scalepix=1;
		}
	}

	if (strstr(filename,".gif") && enable_animations && (! local_image))  {		
		ct_insert_animation(smtmpfilename,textbox);
		if (is_scentral_smiley) {unlink(smtmpfilename);}
#ifdef TRANS_BG
		if ( trans_bg ) {update_transparent_background( NULL, NULL, "update" );}
#endif
		return 0; 
					     													} 

	if (textbox != NULL) { /* something other than main chat window */
		t_buffer= gtk_object_get_data(GTK_OBJECT(textbox),"textbuffer");
								 } else {
		t_buffer=text_buffer; /* default to chat window itself */
											}

	if (t_buffer==NULL) { t_buffer=text_buffer;  }

	text_length = gtk_text_buffer_get_char_count(t_buffer);
	gtk_text_buffer_get_iter_at_offset(t_buffer, &iter, text_length );

	pixbuf = gdk_pixbuf_new_from_file( smtmpfilename, NULL );
	if (pixbuf) { /* added: Phrozen Smoke */

		width = gdk_pixbuf_get_width( pixbuf );
		height = gdk_pixbuf_get_height( pixbuf );


		/* added, scale avatars down to something halfway 
			reasonable: PhrozenSmoke */

				if ( strstr(filename,"/avatars/"))  {
						if ( width < height ) {
							width = ( 1.0 * width / height ) * 30;
							height = 30;
						} else {
							height = ( 1.0 * height / width ) * 30;
							width = 30;
						}
																}

				if (custom_scale_text && (! local_image) ) {
					if (strcmp(custom_scale_text, "1.0")) {
						anim_mxsz=(int) (anim_mxsz*atof(custom_scale_text));
						if (anim_mxsz>55) {anim_mxsz=55;}
						width=(int) (width*atof(custom_scale_text));
						height=(int) (height*atof(custom_scale_text));
						scalepix=1;
						/* printf("uscale:  %d  %d   %d\n", anim_mxsz, width, height);
						fflush(stdout); */ 
					}
				}

				if ( is_scentral_smiley || scalepix)  {
					if (height>anim_mxsz ) {
							width = ( 1.0 * width / height ) * anim_mxsz;
							height = anim_mxsz;
														}
					if (local_image) {
						if (width>475 ) { width=475;}
					}
													}

		spixbuf = gdk_pixbuf_scale_simple( pixbuf, width, height,
		GDK_INTERP_BILINEAR );

		if (spixbuf) { /* added: Phrozen Smoke */
			gtk_text_buffer_insert_pixbuf( t_buffer, &iter, spixbuf );
			g_object_unref( spixbuf );
						   }
		g_object_unref( pixbuf );
			   }

		if (is_scentral_smiley) {unlink(smtmpfilename);}
	
#ifdef TRANS_BG
	if ( trans_bg )
		update_transparent_background( NULL, NULL, "update" );
#endif

	return( 0 );
}

gboolean
on_chat_text_clicked                   (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
	int x, y;
	gint buf_x, buf_y;
	gchar *ptr;

	if (event->button == 1 && event->type == GDK_BUTTON_PRESS) {
		x = event->x;
		y = event->y;

		gtk_text_view_window_to_buffer_coords( GTK_TEXT_VIEW(widget),
			GTK_TEXT_WINDOW_TEXT, x, y, &buf_x, &buf_y );
		ptr = get_word_at_pos( GTK_TEXT_VIEW(widget), buf_x, buf_y );
		if ( check_clicked_text( ptr )) {
			return( TRUE );
		}
	}

	return FALSE;
}

void ct_set_indent( int indent_value ) {
	/* negative number indents successive lines, positive indents the first */
	gtk_text_view_set_indent( GTK_TEXT_VIEW(top_text), 0 - indent_value );
	gtk_text_view_set_indent( chat_text, 0 - indent_value );
}

#ifdef TRANS_BG
void ct_set_transparent( int trans ) {
	char tellbuf[256];

	if (( trans_bg ) &&
		( ! trans )) {
		/* reset background to blank white for now */
		/* Bug fix: PhrozenSmoke:...uh let's set the background to white, not black */
		ct_set_font( "fixed 12" );
		ct_set_color( 0, 0, 0 );
		ct_set_transparent_shade( 255, 255, 255);
		snprintf( tellbuf, 255, 
			"\n************************************************************\nTransparency support is experimental in Gyach Enhanced.\nTo fully turn off transparency, you must exit and restart\nGyach Enhanced!\n************************************************************\n"
			);
		ct_append( tellbuf, strlen( tellbuf ));
		ct_scroll_to_bottom();
	}

	trans_bg = trans;
	if ( trans_bg ) {
		ct_update_bg();
	}
}

void ct_set_transparent_shade( int r, int g, int b ) {
	trans_shade_r = r ;
	trans_shade_g = g ;
	trans_shade_b = b ;

	/* Bug fix: removed - PhrozenSmoke, why did original author have this here?  ...
	it was throwing the colors off and screwing everything up */
	/* trans_shade_r = r + 128;
	trans_shade_g = g + 128;
	trans_shade_b = b + 128; */

	if ( trans_bg ) {
		gdk_pixmap_unref( st_pixmap_text );
		st_pixmap_text = NULL;
		ct_update_bg();
	}
}

void ct_update_bg() {
	if ( trans_bg )
		update_transparent_background( NULL, NULL, "update" );
}
#endif

#endif  /* GYACH_INT_TEXT */
