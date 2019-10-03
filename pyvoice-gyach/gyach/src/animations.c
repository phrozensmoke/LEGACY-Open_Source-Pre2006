/*****************************************************************************
 * animations.c
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
 * VERY preliminary code for handling animated smileys
 *****************************************************************************/
#include "animations.h"

int enable_animations=1;

int max_sim_animations=2;
int running_animations=0;

extern int show_emoticons; 
extern GtkWidget *top_text;
extern char *custom_scale_text;


int stop_animation(GtkLabel *mylab) {
	GdkPixbuf *buf = NULL;
	GdkPixbufAnimation *anim=NULL;
	GtkImage *myimage=NULL;
	myimage=gtk_object_get_data(GTK_OBJECT(mylab), "image");
	anim=gtk_object_get_data(GTK_OBJECT(mylab), "anim");

	if (myimage && anim) {
		buf=gdk_pixbuf_animation_get_static_image(anim);
		if (buf)  {
			gtk_image_set_from_pixbuf(myimage, buf);
			running_animations--;
			if (running_animations<0) {running_animations=0;}
			g_object_unref( buf );
					}
		g_object_unref(anim);
								}
	gtk_widget_destroy(GTK_WIDGET(mylab));
	return 0;
}



void ct_insert_animation( char *filename, GtkWidget *textbox) {
	GdkPixbufAnimation *anim = NULL;
	GdkPixbuf *buf = NULL;
	GtkTextIter iter;
	GtkTextBuffer *t_buffer;
	int text_length;
	GtkTextView *pmst=NULL;
	GtkWidget *myimage=NULL;
	GtkTextChildAnchor *tanch=NULL;
	/* need a small gtk object to carry around data, seems easier, will be destroyed */
	GtkWidget *mylab=gtk_label_new("");
	int allow_anim=enable_animations;
	int scalepix=0;

	if (running_animations>=max_sim_animations) {allow_anim=0;}
	if (! show_emoticons) {return;}

		//printf("debug-a\n"); fflush(stdout);

	anim = gdk_pixbuf_animation_new_from_file(filename,NULL);

	if (anim) {
		if (gdk_pixbuf_animation_is_static_image(anim) || (!allow_anim)) {
			buf=gdk_pixbuf_animation_get_static_image(anim);
																									}

		     } else {
		/* printf("no animation, trying to load regular\n"); fflush(stdout); */
		buf = gdk_pixbuf_new_from_file(filename,NULL);
		if (!buf) 		{
			/* printf("couldnt load at all!\n"); fflush(stdout);  */
			return;
						}

				}

		//printf("debug-b\n"); fflush(stdout);

	if (textbox != NULL) { /* something other than main chat window */
		t_buffer= gtk_object_get_data(GTK_OBJECT(textbox),"textbuffer");
		pmst=GTK_TEXT_VIEW(textbox);
								 } else {
		t_buffer=text_buffer22; /* default to chat window itself */
											}

	if (t_buffer==NULL) { t_buffer=text_buffer22;  }
	

	text_length = gtk_text_buffer_get_char_count(t_buffer);
	gtk_text_buffer_get_iter_at_offset(t_buffer, &iter, text_length );

		   if ((anim) && (! gdk_pixbuf_animation_is_static_image(anim))
			&& allow_anim) {

			if ( capture_fp ) {	
				time_t time_llnow = time(NULL);
				fprintf(capture_fp,"\n\n[%s]  ANIMATION RENDERED, File: %s\n\n", ctime(&time_llnow), filename);
				fflush( capture_fp );
				}

			tanch=gtk_text_buffer_create_child_anchor (t_buffer, &iter);
			myimage=gtk_image_new_from_animation(anim);

			if (pmst) {
				gtk_text_view_add_child_at_anchor(pmst, myimage, tanch);
						} else  {
				gtk_text_view_add_child_at_anchor(chat_text, myimage, tanch);
				   				  }
			running_animations++;
			gtk_widget_show_all(myimage);
						 		} else  {
				/* static gifs and all other images, also when animation is disabled */
				/* try to scale to a reasonable size, if necessary */
				int awidth=0;
				int aheight=0;
				int anim_mxsz=40;
				awidth = gdk_pixbuf_get_width( buf );
				aheight = gdk_pixbuf_get_height( buf );

				if (custom_scale_text) {
					if (strcmp(custom_scale_text, "1.0")) {
						anim_mxsz=(int) (anim_mxsz*atof(custom_scale_text));
						if (anim_mxsz>55) {anim_mxsz=55;}
						awidth=(int) (awidth*atof(custom_scale_text));
						aheight=(int) (aheight*atof(custom_scale_text));
						scalepix=1;
						/* printf("scale:  %d  %d   %d\n", anim_mxsz, awidth, aheight);
						fflush(stdout); */ 
					}
				}

					if ((aheight>anim_mxsz) || scalepix ) {
					GdkPixbuf *spixbuf;
					if (aheight>anim_mxsz) {
						awidth = ( 1.0 * awidth / aheight ) * anim_mxsz;
						aheight = anim_mxsz;
					}
					spixbuf = gdk_pixbuf_scale_simple( buf, awidth, aheight,
					GDK_INTERP_BILINEAR );
					if (spixbuf) {
						gtk_text_buffer_insert_pixbuf( t_buffer, &iter, spixbuf ); 
						g_object_unref( spixbuf );
						g_object_unref( buf );
						return;
					}
																}

			gtk_text_buffer_insert_pixbuf( t_buffer, &iter, buf ); 
			g_object_unref( buf );
			return;
							    			}


	 if (anim && allow_anim) {
		if (! gdk_pixbuf_animation_is_static_image(anim)) {
			gtk_object_set_data(GTK_OBJECT(mylab),"image",myimage);
			gtk_object_set_data(GTK_OBJECT(mylab),"anim",anim);	
			/* auto-stop all animations after 10 seconds */
			g_timeout_add( 10000, (void *)stop_animation, mylab );
											  }
									
		    			      }

}



