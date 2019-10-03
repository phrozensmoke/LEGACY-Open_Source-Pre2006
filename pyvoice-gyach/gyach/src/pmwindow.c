/*****************************************************************************
 * pmwindow.c
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

/*
	exported by PhrozenSmoke from interface.c, to reduce monstrous size of 
	interface.c (can cause 'out of memory' errors while compiling)...
	all main PM Window UI creation functions
*/


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"

#include "images.h"
#include "util.h"
#include <time.h>
#include <malloc.h>

#include "gyach.h"
#include "gyach_int.h"
#include "plugin_api.h"
#include "plugins.h"
#include "profname.h"

extern GtkWidget *yab_entry_list;
extern int use_buddy_list_font;
GList *encryption_list = NULL;
char *last_file_selected=NULL;
GdkColor tchat_online_color;
extern gboolean tuxviron_redraw_notify_event (GtkWidget *widget,
		GdkEvent *event, gpointer user_data);
extern GtkWidget *create_list_dialog(char *wintitle, char *header, char *ldefault, GList *comblist, int editable, int icon);


static char pmwinbuf[320];

static char * down_xpm[] = {
"12 13 33 1",
" 	c None",
".	c #030303",
"+	c #C0C0C0",
"@	c #9C9C9C",
"#	c #171717",
"$	c #2E2E2E",
"%	c #C7C7C7",
"&	c #707070",
"*	c #D1D1D1",
"=	c #E4E4E4",
"-	c #4A4A4A",
";	c #EAEAEA",
">	c #5E5E5E",
",	c #EEEEEE",
"'	c #BABABA",
")	c #595959",
"!	c #F2F2F2",
"~	c #8E8E8E",
"{	c #292929",
"]	c #363636",
"^	c #ACACAC",
"/	c #DEDEDE",
"(	c #3A3A3A",
"_	c #4F4F4F",
":	c #A3A3A3",
"<	c #FAFAFA",
"[	c #676767",
"}	c #7E7E7E",
"|	c #434343",
"1	c #222222",
"2	c #0A0A0A",
"3	c #FEFEFE",
"4	c #1E1E1E",
"            ",
"  ]___)___)#",
".$&@@~::@@:$",
".|+@>{1';;,-",
"._%^@}#[+<3_",
"._%*=%(1>'3-",
".|+,!%&#.|3-",
".-+,3%#..(3-",
".|+,3;^]2|3-",
".-+,333@||3-",
".|';<<<3@&3_",
" |:*=/====3-",
"  (_))_)))>4"};




/* PhrozenSmoke:  added some features here to save PMs to file */

int pm_capture_to_file( char *filename, GtkTextBuffer *mybuffer, GtkTextView *widget, GtkWindow  *mywindow ) {
	FILE *fp;
	int  length;
	GtkTextIter start;
	GtkTextIter end;
	gchar *ptr;
	gchar *capb2loc;	
	char *lastfilename=NULL;
	FILE *lastfile=NULL;
	time_t time_llnow= time(NULL);

	/* Close possible old logs first */
	lastfilename=gtk_object_get_data(GTK_OBJECT(widget),"logfilename");
	lastfile=gtk_object_get_data(GTK_OBJECT(widget),"logfile");
	if (lastfile != NULL) {
		if (lastfilename != NULL) {
			snprintf(pmwinbuf,318, "  %s%s* %s : '%s' *\n%s%s", YAHOO_STYLE_ITALICON, "\033[#220099m", _("Stopped logging PM to file"), lastfilename, YAHOO_STYLE_ITALICOFF, YAHOO_COLOR_BLACK);
			append_to_textbox_color( GTK_WIDGET(mywindow) ,GTK_WIDGET(widget), pmwinbuf );
		}

		fprintf( lastfile, "\n" );
		fflush(lastfile);
		fclose(lastfile);
		lastfile=NULL;
		gtk_object_set_data(GTK_OBJECT(widget),"logfile", lastfile);
	}


	if (!filename) {return 0;}
	if (lastfilename != NULL) {free(lastfilename);}
	gtk_object_set_data(GTK_OBJECT(widget),"logfilename", strdup(filename));

	fp = fopen( filename, "ab" );

	if ( ! fp )  {
		show_ok_dialog(_("Error opening file for saving."));
		return( 0 );
				}

	length = gtk_text_buffer_get_char_count( mybuffer );

			fprintf( fp, "\nGyach Enhanced, version %s\nCopyright (c) 2003-2006, Erica Andrews\n%s\nLicense: GNU General Public License\n\nINSTANT MESSAGE SESSION LOG\n", VERSION, GYACH_URL );
			fprintf( fp, "%s", ctime(&time_llnow ) );
			fprintf( fp, "\n_____________________\n\n" );
			fflush( fp );

	gtk_text_buffer_get_iter_at_offset( mybuffer, &start, 0 );
	gtk_text_buffer_get_iter_at_offset(mybuffer, &end, length );
	ptr = gtk_text_buffer_get_text(mybuffer, &start, &end, 0 );
	capb2loc=_b2loc(ptr);
	fwrite( capb2loc, 1, strlen(capb2loc), fp );
	g_free( ptr );
	fflush(fp);

	gtk_object_set_data(GTK_OBJECT(widget),"logfile", fp);
	snprintf(pmwinbuf,318, "  %s%s* %s : '%s' *\n%s%s", YAHOO_STYLE_ITALICON, "\033[#FFB8AFm", _("Logging PM to file"), filename, YAHOO_STYLE_ITALICOFF, YAHOO_COLOR_BLACK);
	append_to_textbox_color( GTK_WIDGET(mywindow ),GTK_WIDGET(widget), pmwinbuf );

	return( 0 );
}

void on_pmsavefile_ok_button_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkTextBuffer *mybuff;
	GtkWidget *tmp_widget;
	GtkWidget *tmp_widget2;
	GtkWidget *mywindow;
	char *myfile;

	      tmp_widget =gtk_object_get_data(GTK_OBJECT(button),"mywindow");
	      if (!tmp_widget) {return;}
	      tmp_widget2=gtk_object_get_data(GTK_OBJECT(button),"pmstext");
	      if (!tmp_widget2) {return;}
	      mybuff=gtk_object_get_data(GTK_OBJECT(tmp_widget2),"textbuffer");
	      if (!mybuff) {return;}
	      mywindow=gtk_object_get_data(GTK_OBJECT(tmp_widget2),"pmwindow");
	      if (!mywindow) {return;}
	       myfile = strdup(gtk_file_selection_get_filename( GTK_FILE_SELECTION( tmp_widget )));
	     if (strlen(myfile)<1) {free(myfile); return;}
	     pm_capture_to_file( myfile, mybuff, GTK_TEXT_VIEW(tmp_widget2), GTK_WINDOW(mywindow));
	     gtk_widget_destroy( tmp_widget);
	     free(myfile);
}

void preselected_filename(char *filename) {
	if (last_file_selected) {free(last_file_selected); last_file_selected=NULL;}
	if (filename) {last_file_selected=strdup(filename);}
}

void on_pmsavefile(GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *okbutton;
	GtkWidget *tmp_widget2;
	char *filename=NULL;
	tmp_widget2=gtk_object_get_data(GTK_OBJECT(button),"pmstext");
	if (!tmp_widget2) {return;}

	if (gtk_object_get_data(GTK_OBJECT(tmp_widget2),"logfile") != NULL) {
		/* log file open, so close it and return */
		GtkTextBuffer *mybuff;
		GtkWidget *mywindow;
	     	mybuff=gtk_object_get_data(GTK_OBJECT(tmp_widget2),"textbuffer");
	      	if (!mybuff) {return;}
	     	mywindow=gtk_object_get_data(GTK_OBJECT(button),"pmwindow");
	      	if (!mywindow) {return;}
		pm_capture_to_file( NULL, mybuff, GTK_TEXT_VIEW(tmp_widget2), GTK_WINDOW(mywindow));
		return;
	}

	filename=gtk_object_get_data(GTK_OBJECT(tmp_widget2),"logfilename");
		
	if (filename != NULL) {
		preselected_filename(filename);
		}
	else {
		char fldate[24];
		time_t timell_now;
		struct tm *tmll_now;
		char *who=NULL;
		who=gtk_object_get_data(GTK_OBJECT(button),"who");
		timell_now = time( NULL );
		tmll_now = localtime( &timell_now );
		if (! strftime(fldate, 22, "_%b-%d-%Y",tmll_now ) ) {sprintf(fldate,"%s","");}
		snprintf( pmwinbuf, 254, "%s/gyachE-IM-%s%s.txt", GYACH_CFG_DIR, who?who:"default", fldate );
		preselected_filename(pmwinbuf);
	}	

	okbutton=create_fileselection2();
	gtk_object_set_data (GTK_OBJECT (okbutton), "pmstext", tmp_widget2);

  gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_pmsavefile_ok_button_clicked ),
                      NULL);
}



/* END PhrozenSmoke:  added some features here to save PMs to file */



GtkWidget *get_pm_icon(int which_icon, char *err_str) 
{
	GtkWidget *imimage=NULL;
	GdkPixbuf *imbuf=NULL;
	char **picon=NULL;

	if (which_icon<1)  {return GTK_WIDGET(gtk_label_new(err_str));}
	if (which_icon>17)  {return GTK_WIDGET(gtk_label_new(err_str));}

		if (which_icon==1) {picon=(char **)pixmap_pm_buzz;}
		if (which_icon==2) {picon=(char **)pixmap_pm_close;}
		if (which_icon==3) {picon=(char **)pixmap_pm_erase;}
		if (which_icon==4) {picon=(char **)pixmap_pm_file;}
		if (which_icon==5) {picon=(char **)pixmap_pm_ignore;}
		if (which_icon==6) {picon=(char **)pixmap_pm_join;}
		if (which_icon==7) {picon=(char **)pixmap_pm_profile;}
		if (which_icon==8) {picon=(char **)pixmap_pm_send;}
		if (which_icon==9) {picon=(char **)pixmap_pm_voice;}
		if (which_icon==10) {picon=(char **)pixmap_pm_save;}
		if (which_icon==11) {picon=(char **)pixmap_pm_tux;}
		if (which_icon==12) {picon=(char **)pixmap_mail;}
		if (which_icon==13) {picon=(char **)pixmap_colors;}
		if (which_icon==14) {picon=(char **)pixmap_pm_encrypt;}
		if (which_icon==15) {picon=(char **)pixmap_no_photo;}
		if (which_icon==16) {picon=(char **)pixmap_pm_audible;}
		if (which_icon==17) {picon=(char **)down_xpm;}

  		imbuf=gdk_pixbuf_new_from_xpm_data((const char**)picon);
  		imimage=gtk_image_new_from_pixbuf(imbuf);
		if (imimage) {g_object_unref( imbuf ); return imimage;}

	return GTK_WIDGET(gtk_label_new(err_str));
}




void activate_encryption(GtkWidget *button, gpointer user_data) {
	PM_SESSION *pms;
	GtkWidget *tuxwin;
	GtkWidget *tentry;
	char *stuff=NULL;

	tuxwin=gtk_object_get_data(GTK_OBJECT(button),"window");
	pms=gtk_object_get_data(GTK_OBJECT(button),"session");
	tentry=gtk_object_get_data(GTK_OBJECT(button),"entry");
	if (!pms) { if (tuxwin) {gtk_widget_destroy(tuxwin);} return;}
	if (!tentry) { if (tuxwin) {gtk_widget_destroy(tuxwin);} return;}

	stuff=strdup(gtk_entry_get_text(GTK_ENTRY(tentry)));

	if (!strcmp(stuff,_("[NONE]"))) {  /* turn encryption off */
		if (pms->encrypted_myway) {
			snprintf(pmwinbuf,300, "  %s%s* %s *\n%s%s", YAHOO_STYLE_ITALICON, "\033[#FFB8AFm", _("Encrypted session stopped."),  YAHOO_STYLE_ITALICOFF, YAHOO_COLOR_BLACK);
			append_to_textbox_color( pms->pm_window,pms->pm_text, pmwinbuf );
	set_current_pm_profile_name(retrieve_profname_str(GTK_WIDGET(retrieve_profname_entry(pms->pm_window)))  );
			if (! pms->sms) {acknowledge_encryption(pms, 0);}
			reset_current_pm_profile_name();
			pms->encrypted_myway=0;
			pms->my_gpg_passphrase=-1;
			pms->their_gpg_passphrase=-1;
			free(stuff);
			if (tuxwin) {gtk_widget_destroy(tuxwin); focus_pm_entry(pms->pm_window); }
			return;
			}

		 								   } else {		
			if ( (! enable_encryption) ||  (pms->sms)) {
				pms->encrypted_myway=0;
				pms->encryption_type=0;
				pms->my_gpg_passphrase=-1;
				pms->their_gpg_passphrase=-1;
				snprintf(pmwinbuf,300, "  %s%s* %s *\n%s%s", YAHOO_STYLE_ITALICON, "\033[#FFB8AFm", _("Encryption is not currently enabled."),  YAHOO_STYLE_ITALICOFF, YAHOO_COLOR_BLACK);
				append_to_textbox_color( pms->pm_window,pms->pm_text, pmwinbuf );
				free(stuff);
				if (tuxwin) {gtk_widget_destroy(tuxwin); focus_pm_entry(pms->pm_window); }
				return;
									  			  } else {
				int enctype=0;
				enctype=get_encryption_from_description(stuff);

				int apptest=0;

				if (pms->encryption_type != enctype) {
					snprintf(pmwinbuf,300, "  %s%s* %s '%s' *\n%s%s", YAHOO_STYLE_ITALICON, "\033[#FFB8AFm", _("Requesting encrypted session..."), stuff, YAHOO_STYLE_ITALICOFF, YAHOO_COLOR_BLACK);	
					append_to_textbox_color( pms->pm_window,pms->pm_text, pmwinbuf );
	set_current_pm_profile_name(retrieve_profname_str(GTK_WIDGET(retrieve_profname_entry(pms->pm_window)))  );
					acknowledge_encryption(pms, enctype);

						/* It's important that we lock our profile identity in place
							once we start using encryption stuff, otherwise 
							encryption/decryption can fail for some algorithms */					
					gtk_widget_set_sensitive(GTK_WIDGET(				gtk_object_get_data(GTK_OBJECT(pms->pm_window),"profwidget" )), FALSE);

					reset_current_pm_profile_name();
					pms->encrypted_myway=1;
					pms->encryption_type=enctype;
					pms->my_gpg_passphrase=-1;
					pms->their_gpg_passphrase=-1;

					/* for testing */
					if (apptest) {
						/* *********  */  /* 
						char *appencstr=gyache_encrypt_message ("Shamis123Gone", "I feel you thought. Hmm, this is just another encryption test String. Testing for problems such as truncation, etc.: This is the Gyach Enhanced Encryption test string. I dont wanna grow up, I'm a toys r us kid. - PGP MESSAGE",
 pms->encryption_type);
						printf("encryption success:  %s\n", appencstr); fflush(stdout);
						printf("PM Decrypted: %s\n",
							gyache_decrypt_message ("Shamis123Gone", appencstr, pms->encryption_type)	);
						fflush(stdout);

					if (capture_fp) {
						fprintf(capture_fp, "DEcrypt Msg [%s] : %s\n\n",stuff,  gyache_decrypt_message ("Shamis123Gone", appencstr, pms->encryption_type));
						fflush(capture_fp);
					}
					*/  /*  *********** */ 
										}

																			} else {
					snprintf(pmwinbuf,300, "  %s%s* %s '%s' *\n%s%s", YAHOO_STYLE_ITALICON, "\033[#FFB8AFm", _("The session is already using this encryption."), stuff, YAHOO_STYLE_ITALICOFF, YAHOO_COLOR_BLACK);
					append_to_textbox_color( pms->pm_window,pms->pm_text, pmwinbuf );
																					  }
				if (tuxwin) {gtk_widget_destroy(tuxwin); focus_pm_entry(pms->pm_window); }
				free(stuff);
				return;
															}
													}

	free(stuff);
	if (tuxwin) {gtk_widget_destroy(tuxwin); focus_pm_entry(pms->pm_window); }
}



void create_encryption_list() {
	encryption_list = g_list_append( encryption_list, _("[NONE]"));

	if (encryption_type_available(ENC_TYPE_GPGME)) {
		encryption_list = g_list_append( encryption_list, ENC_STR_GPGME);
	}

	if (encryption_type_available(ENC_TYPE_BF_INTERNAL)) {
		encryption_list = g_list_append( encryption_list, ENC_STR_BF_INTERNAL);
	}

	if (encryption_type_available(ENC_TYPE_MCRYPT_BF) ) {

		encryption_list = g_list_append( encryption_list, ENC_STR_MCRYPT_BF);
		encryption_list = g_list_append( encryption_list, ENC_STR_MCRYPT_CAST128);
		encryption_list = g_list_append( encryption_list, ENC_STR_MCRYPT_CAST256); 
		encryption_list = g_list_append( encryption_list, ENC_STR_MCRYPT_GOST);
		encryption_list = g_list_append( encryption_list, ENC_STR_MCRYPT_LOKI97);
		encryption_list = g_list_append( encryption_list, ENC_STR_MCRYPT_ARCFOUR);
		encryption_list = g_list_append( encryption_list, ENC_STR_MCRYPT_RC6);
		encryption_list = g_list_append( encryption_list, ENC_STR_MCRYPT_RIJNDAEL);
		encryption_list = g_list_append( encryption_list, ENC_STR_MCRYPT_SAFERPLUS);
		encryption_list = g_list_append( encryption_list, ENC_STR_MCRYPT_SERPENT);
		encryption_list = g_list_append( encryption_list, ENC_STR_MCRYPT_TD);
		encryption_list = g_list_append( encryption_list, ENC_STR_MCRYPT_TF);
		encryption_list = g_list_append( encryption_list, ENC_STR_MCRYPT_XTEA);


		/* Loki-97 and Cast-256 may have truncation issues, think its corrected */
	}
}


void create_encryption_window(PM_SESSION *pms) {
        GtkWidget *okbutton;
  	 if (!pms) {return;}
	if (!encryption_list) {create_encryption_list();}
	if (!encryption_list) {return;}

	okbutton=create_list_dialog(_("Encryption"), _("Select An Encryption Method:"), NULL, encryption_list, FALSE, 14);

  gtk_object_set_data (GTK_OBJECT (okbutton), "session",pms);
  gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC (activate_encryption),
                      NULL);
}


void open_encryption_selection(GtkWidget *button, gpointer user_data) {
	PM_SESSION *pms;
	pms=gtk_object_get_data(GTK_OBJECT(button),"session");
	if (!pms) {return;}
	create_encryption_window(pms);
}


void style_text_entry(GtkWidget *pms_entry) {
	PangoFontDescription *font_desc=NULL;
	int r=0,b=0,g=0;
	if (!strcmp(use_color,"custom")) {
		r=custom_color_red; g=custom_color_green; b=custom_color_blue;
	}
	if (!strcmp(use_color,"blue")) {r = 0; g = 0; b = 0xd9;}
	if (!strcmp(use_color,"cyan")) {r = 0; g = 0xb8; b = 0xc8;}
	if (!strcmp(use_color,"gray")) {r = 0x9a; g = 0xa0; b = 0xb3;}
	if (!strcmp(use_color,"green")) {r = 0x4f; g = 0xbc ; b = 0x6f;}
	if (!strcmp(use_color,"pink")) {r = 0xff; g = 0xaf; b = 0xaf;}
	if (!strcmp(use_color,"purple")) {r = 0xb2; g = 0; b = 0xba;}
	if (!strcmp(use_color,"yellow")) {r = 0xdd; g = 0xd9 ; b = 0x27;} 
	if (!strcmp(use_color,"orange")) {r = 0xe6; g = 0xb4; b = 0x2c;}
	if (!strcmp(use_color,"red")) {r = 0xc8 ; g = 0x14; b = 0x47;}

	tchat_online_color.red   = r * ( 65535 / 255 );
	tchat_online_color.green = g * ( 65535 / 255 );
	tchat_online_color.blue  = b * ( 65535 / 255 );
	gtk_widget_modify_text  (pms_entry,GTK_STATE_NORMAL,&tchat_online_color);
	font_desc = pango_font_description_from_string( font_name?font_name:"Sans 12");
	if ( font_desc ) {
		gtk_widget_modify_font  (pms_entry,font_desc);
		pango_font_description_free(font_desc);
	}
}

void set_text_entry_styles() {
	GtkWidget *tmp_wid=NULL;
	GList *this_session = pm_list;
	PM_SESSION *pm_sess;

	tmp_wid=gtk_object_get_data( GTK_OBJECT(chat_window), "chat_entry");	
	if (tmp_wid) {style_text_entry(tmp_wid);}

	while( this_session ) {   /* any open PM sessions */
		tmp_wid=NULL;
		pm_sess = (PM_SESSION *)this_session->data;
		tmp_wid=gtk_object_get_data( GTK_OBJECT(pm_sess->pm_window), "pms_entry");	
		if (tmp_wid) {style_text_entry(tmp_wid);}
		this_session = g_list_next( this_session );
	}
	if (use_buddy_list_font) {
		PangoFontDescription *font_desc=NULL;
		font_desc = pango_font_description_from_string( display_font_name?display_font_name:"Sans 12" );
		if ( font_desc ) {
			if (bd_tree) {gtk_widget_modify_font  (bd_tree,font_desc);}
			if (chat_users) {gtk_widget_modify_font  (GTK_WIDGET(chat_users),font_desc);}
			if (yab_entry_list) {gtk_widget_modify_font  (yab_entry_list,font_desc);}
			pango_font_description_free(font_desc);
		}
	}
}


/*  exported from interface.c - PhrozenSmoke
    PM Windows in separate module to prevent MONSTROUS interface.c
*/


GtkWidget *get_display_image_bar(GtkWidget *widg) {
	GtkWidget *topbox, *vbox1, *hbox1, *hbox2, *hbox3;
	GtkWidget  *menubar1, *menu_head, *photo_menu, *photo_item;
	GtkWidget *bimage_me, *bimage_them, *bimage_butt;

	topbox = gtk_vbox_new (FALSE, 2);
	vbox1 = gtk_vbox_new (FALSE, 2);
	hbox1 = gtk_hbox_new (FALSE, 2);
	hbox2 = gtk_hbox_new (FALSE, 2);
	hbox3 = gtk_hbox_new (FALSE, 2);

  gtk_box_pack_start (GTK_BOX (topbox), hbox1, FALSE, FALSE, 3);
  gtk_box_pack_start (GTK_BOX (topbox), gtk_label_new(""), TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (topbox), vbox1, FALSE, FALSE, 0);

  bimage_me=GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_NEW, GTK_ICON_SIZE_DND) );
  bimage_butt=gtk_button_new();
  set_tooltip(bimage_butt,_("Edit My Display Image..."));
  gtk_signal_connect (GTK_OBJECT (bimage_butt), "clicked",
                      GTK_SIGNAL_FUNC (on_bimage_edit_my_photo),
                      NULL);
  gtk_button_set_relief(GTK_BUTTON(bimage_butt), GTK_RELIEF_NONE);
  gtk_container_add(GTK_CONTAINER(bimage_butt), bimage_me);

  bimage_them=GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_NEW, GTK_ICON_SIZE_DND) );
  gtk_box_pack_start (GTK_BOX (vbox1), gtk_hseparator_new(), FALSE, FALSE, 3);
  gtk_box_pack_start (GTK_BOX (vbox1), bimage_them, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox2, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox1), gtk_hseparator_new(), FALSE, FALSE, 3);
  gtk_box_pack_start (GTK_BOX (vbox1), bimage_butt, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox3, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (topbox), gtk_label_new(" "), TRUE, TRUE, 0);

  gtk_box_pack_start (GTK_BOX (hbox1), gtk_label_new(""), TRUE, TRUE, 0);
  menubar1 = gtk_menu_bar_new ();
  gtk_box_pack_start (GTK_BOX (hbox1), menubar1, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox1), gtk_label_new(""), TRUE, TRUE, 0);
  menu_head = gtk_image_menu_item_new ();
  gtk_container_add (GTK_CONTAINER (menubar1), menu_head);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_head), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_ZOOM_IN, GTK_ICON_SIZE_MENU) ));
  photo_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_head), photo_menu);

  photo_item=gtk_menu_item_new_with_label(_("Show"));
  gtk_container_add (GTK_CONTAINER (photo_menu), photo_item);
  gtk_signal_connect (GTK_OBJECT (photo_item), "activate",
                      GTK_SIGNAL_FUNC (on_bimage_show_cb),
                      vbox1);

  photo_item=gtk_menu_item_new_with_label(_("Hide"));
  gtk_container_add (GTK_CONTAINER (photo_menu), photo_item);
  gtk_signal_connect (GTK_OBJECT (photo_item), "activate",
                      GTK_SIGNAL_FUNC (on_bimage_hide_cb),
                      vbox1);

  gtk_container_add (GTK_CONTAINER (photo_menu), gtk_menu_item_new());

  photo_item=gtk_menu_item_new_with_label(_("Small"));
  gtk_container_add (GTK_CONTAINER (photo_menu), photo_item);
  gtk_signal_connect (GTK_OBJECT (photo_item), "activate",
                      GTK_SIGNAL_FUNC (on_bimage_update_size1),
                      widg);
  photo_item=gtk_menu_item_new_with_label(_("Medium"));
  gtk_container_add (GTK_CONTAINER (photo_menu), photo_item);
  gtk_signal_connect (GTK_OBJECT (photo_item), "activate",
                      GTK_SIGNAL_FUNC (on_bimage_update_size2),
                      widg);
  photo_item=gtk_menu_item_new_with_label(_("Large"));
  gtk_container_add (GTK_CONTAINER (photo_menu), photo_item);
  gtk_signal_connect (GTK_OBJECT (photo_item), "activate",
                      GTK_SIGNAL_FUNC (on_bimage_update_size3),
                      widg);



  gtk_box_pack_start (GTK_BOX (hbox2), gtk_label_new(""), TRUE, TRUE, 0);
  menubar1 = gtk_menu_bar_new ();
  gtk_box_pack_start (GTK_BOX (hbox2), menubar1, FALSE, FALSE, 0);
  menu_head = gtk_image_menu_item_new ();
  gtk_container_add (GTK_CONTAINER (menubar1), menu_head);
  gtk_container_set_border_width (GTK_CONTAINER (menubar1), 0);
  gtk_container_set_border_width (GTK_CONTAINER (menu_head), 0);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_head), 
	GTK_WIDGET(get_pm_icon(17,"") ));
  photo_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_head), photo_menu);

  photo_item=gtk_menu_item_new_with_label(_("View full-size avatar"));
  gtk_container_add (GTK_CONTAINER (photo_menu), photo_item);
  gtk_signal_connect (GTK_OBJECT (photo_item), "activate",
                      GTK_SIGNAL_FUNC (on_bimage_view_avatar),
                      widg);
  photo_item=gtk_menu_item_new_with_label(_("View avatar animation"));
  gtk_container_add (GTK_CONTAINER (photo_menu), photo_item);
  gtk_signal_connect (GTK_OBJECT (photo_item), "activate",
                      GTK_SIGNAL_FUNC (on_bimage_view_animation),
                      widg);


  gtk_box_pack_start (GTK_BOX (hbox3), gtk_label_new(""), TRUE, TRUE, 0);
  menubar1 = gtk_menu_bar_new ();
  gtk_box_pack_start (GTK_BOX (hbox3), menubar1, FALSE, FALSE, 0);
  menu_head = gtk_image_menu_item_new ();
  gtk_container_add (GTK_CONTAINER (menubar1), menu_head);
  gtk_container_set_border_width (GTK_CONTAINER (menubar1), 0);
  gtk_container_set_border_width (GTK_CONTAINER (menu_head), 0);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_head), 
	GTK_WIDGET(get_pm_icon(17,"") ));
  photo_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_head), photo_menu);

  photo_item=gtk_menu_item_new_with_label(_("Share my image"));
  gtk_container_add (GTK_CONTAINER (photo_menu), photo_item);
  gtk_signal_connect (GTK_OBJECT (photo_item), "activate",
                      GTK_SIGNAL_FUNC (on_bimage_share),
                      widg);
  photo_item=gtk_menu_item_new_with_label(_("Don't share my image"));
  gtk_container_add (GTK_CONTAINER (photo_menu), photo_item);
  gtk_signal_connect (GTK_OBJECT (photo_item), "activate",
                      GTK_SIGNAL_FUNC (on_bimage_share_off),
                      widg);

	gtk_object_set_data(GTK_OBJECT(topbox), "panel",vbox1 );
	gtk_object_set_data(GTK_OBJECT(topbox), "bimage_me",bimage_me );
	gtk_object_set_data(GTK_OBJECT(topbox), "bimage_them",bimage_them );
	gtk_object_set_data(GTK_OBJECT(topbox), "bimage_butt",bimage_butt );
	gtk_widget_show_all(topbox);
	return topbox;
}



GtkWidget *create_pm_session (void)
{
  GtkWidget *pm_session;
  GtkWidget *vbox13;
  GtkWidget *scrolledwindow8;
  GtkWidget *pms_text;
  GtkWidget *hbox14;
  GtkWidget *pms_entry;
  GtkWidget *pms_send;
  GtkWidget *pms_clear;
  GtkWidget *hbox13;
  GtkWidget *hbox_main;
  GtkWidget *pms_profile;
  GtkWidget *pms_close;
  GtkWidget *pms_ignore;
  GtkWidget *buzz_user;
  GtkWidget *voice_chat;
  GtkWidget *send_user;
  GtkWidget *goto_user;
  GtkWidget *tuxviron;
  GtkWidget *encbutton;
   GtkWidget *save_user;
    GtkWidget *pms_status;
  GtkWidget *smile_button;
  GtkWidget *profwidget, *topbox;
  GtkWidget *pm_toolbar;
  GtkWidget *pm_aud, *ptlabel;
  GtkWidget *pmst_menu, *webcam_menu, *inv_conf, *startcam, *yphoto;
  GtkWidget *sendcam, *menubar1, *menu_head, *start_conf, *yab, *fadd1, *fadd2; 
   GtkWidget *hpaned1=NULL;

  
  pm_session = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_wmclass (GTK_WINDOW (pm_session), "gyachEPM", "GyachE");
  gtk_object_set_data (GTK_OBJECT (pm_session), "pm_session", pm_session);
  gtk_window_set_title (GTK_WINDOW (pm_session), _("Yahoo! Private Message"));
  gtk_window_set_default_size (GTK_WINDOW (pm_session), 512, 365);

  hbox_main = gtk_hbox_new (FALSE, 0);
     gtk_object_set_data(GTK_OBJECT(pm_session),"hbox_main" ,hbox_main );
  vbox13 = gtk_vbox_new (FALSE, 0);

  topbox = gtk_hbox_new (FALSE, 0);
  if (pmb_toolbar1) {gtk_box_pack_start (GTK_BOX (vbox13), topbox, FALSE, FALSE, 2);}
  profwidget=get_profile_name_component(1);
  gtk_box_pack_start (GTK_BOX (topbox), profwidget, TRUE, TRUE, 0);
  gtk_object_set_data(GTK_OBJECT(pm_session),PROFNAMESTR , retrieve_profname_entry (profwidget) );
  gtk_object_set_data(GTK_OBJECT(pm_session),"profwidget" ,profwidget );
  gtk_object_set_data(GTK_OBJECT(pm_session),"profcombo" ,gtk_object_get_data(GTK_OBJECT(profwidget), "profcombo") );
 
 ptlabel=gtk_object_get_data(GTK_OBJECT(profwidget), "label");
 if (ptlabel) {gtk_label_set_text(GTK_LABEL(ptlabel), _("From:") );}


  gtk_box_pack_start (GTK_BOX (topbox), gtk_label_new(" "), FALSE, FALSE, 0);
  pm_toolbar= gtk_toolbar_new ();
  gtk_toolbar_set_orientation( GTK_TOOLBAR(pm_toolbar), GTK_ORIENTATION_HORIZONTAL );
  gtk_toolbar_set_style( GTK_TOOLBAR(pm_toolbar), GTK_TOOLBAR_ICONS );
  gtk_object_set_data(GTK_OBJECT(pm_session),"with_fader_button" ,pm_toolbar );
  gtk_object_set_data(GTK_OBJECT(pm_session),"chat_toolbar" ,pm_toolbar );
  gtk_box_pack_start (GTK_BOX (topbox), pm_toolbar, FALSE, FALSE, 3);
  gtk_widget_realize(pm_session);
  gtk_container_set_border_width (GTK_CONTAINER (pm_toolbar), 1);
  gtk_widget_show(pm_toolbar);
  build_toolbar( pm_session);

  hbox13 = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox13), 1);
  gtk_container_set_border_width (GTK_CONTAINER (hbox_main), 2);
   if (pmb_toolbar2) {gtk_box_pack_start (GTK_BOX (vbox13), hbox13, FALSE, FALSE, 0); }
   gtk_box_pack_start (GTK_BOX (hbox_main), vbox13, TRUE, TRUE, 1);


  if (pm_window_style==1) {
  	hpaned1 = gtk_vpaned_new ();
	 gtk_box_pack_start (GTK_BOX (vbox13), hpaned1, TRUE, TRUE, 0);
  	 
  }

  scrolledwindow8 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_ref (scrolledwindow8);
  gtk_object_set_data_full (GTK_OBJECT (pm_session), "scrolledwindow8", scrolledwindow8,
                            (GtkDestroyNotify) gtk_widget_unref);

  if (pm_window_style==1) {
  	gtk_paned_pack1 (GTK_PANED (hpaned1), scrolledwindow8, TRUE, TRUE);
  } else {
  gtk_box_pack_start (GTK_BOX (vbox13), scrolledwindow8, TRUE, TRUE, 0);
            }
  gtk_widget_show (scrolledwindow8);
  gtk_container_set_border_width (GTK_CONTAINER (scrolledwindow8), 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow8), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow8), GTK_SHADOW_ETCHED_OUT);

  pms_text = gtk_text_view_new ();
  gtk_text_view_set_left_margin (GTK_TEXT_VIEW( pms_text ), 5);
  gtk_text_view_set_right_margin (GTK_TEXT_VIEW( pms_text ), 5);


  gtk_widget_ref (pms_text);
gtk_object_set_data(GTK_OBJECT (pm_session), "pms_text", pms_text); 
  gtk_container_add (GTK_CONTAINER (scrolledwindow8), pms_text);
  gtk_widget_show (pms_text);


/* hold on to text buffer for smileys : PhrozenSmoke */

gtk_object_set_data (GTK_OBJECT (pms_text), "textbuffer", 
		 gtk_text_view_get_buffer( GTK_TEXT_VIEW(pms_text)));
gtk_object_set_data(GTK_OBJECT (pms_text), "pmwindow", pm_session);
gtk_object_set_data(GTK_OBJECT (GTK_TEXT_VIEW(pms_text)->vadjustment), "pmwindow", pm_session);

	/* Needed for TuxVironment redrawing events, do not remove */
	gtk_signal_connect(GTK_OBJECT(GTK_TEXT_VIEW(pms_text)->vadjustment), "value_changed", GTK_SIGNAL_FUNC (tuxviron_redraw_notify_event),NULL);
	gtk_signal_connect(GTK_OBJECT(pms_text), "configure_event",
		GTK_SIGNAL_FUNC (tuxviron_redraw_notify_event), NULL);


  hbox14 = gtk_hbox_new (FALSE, 0);

  if (pm_window_style==1) {
  	 gtk_paned_pack2 (GTK_PANED (hpaned1), hbox14, FALSE, TRUE);
  } else {
	gtk_box_pack_start (GTK_BOX (vbox13), hbox14, FALSE, TRUE, 0);
	    }
  
  gtk_container_set_border_width (GTK_CONTAINER (hbox14), 1);

  /* smiley button */
  smile_button=get_smiley_button("smicon",0);
  	gtk_signal_connect (GTK_OBJECT (smile_button), "clicked",
                      GTK_SIGNAL_FUNC (show_smiley_window_pm), NULL);	
  set_tooltip(smile_button,_("Smileys"));
  gtk_button_set_relief(GTK_BUTTON(smile_button), GTK_RELIEF_NONE);

	if (pmb_smileys) {
  gtk_box_pack_start (GTK_BOX (hbox14), smile_button, FALSE, FALSE, 1);
	}


  pm_aud=gtk_button_new();
  gtk_container_add(GTK_CONTAINER(pm_aud), get_pm_icon(16,_(" Audibles ")) );
  gtk_container_set_border_width(GTK_CONTAINER(pm_aud), 1);
  gtk_button_set_relief(GTK_BUTTON(pm_aud), GTK_RELIEF_NONE);
  set_tooltip(pm_aud,_(" Audibles "));

	if (pmb_audibles && emulate_ymsg6 && enable_audibles) {
  gtk_box_pack_start (GTK_BOX (hbox14), pm_aud, FALSE, FALSE, 1);
	}


   if (pm_window_style==1) {
   GtkTextBuffer *tbe=NULL;
   GtkTextIter	end_iter;
  scrolledwindow8 = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow8), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow8), GTK_SHADOW_IN);
  pms_entry = gtk_text_view_new ();
  gtk_text_view_set_left_margin (GTK_TEXT_VIEW(pms_entry ), 5);
  gtk_text_view_set_right_margin (GTK_TEXT_VIEW( pms_entry), 5);
  gtk_widget_ref (pms_entry);
  gtk_object_set_data_full (GTK_OBJECT (pm_session), "pms_entry", pms_entry,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_container_add (GTK_CONTAINER (scrolledwindow8), pms_entry);
  gtk_box_pack_start (GTK_BOX (hbox14), scrolledwindow8, TRUE, TRUE, 0);

	tbe=gtk_text_view_get_buffer(GTK_TEXT_VIEW(pms_entry));
	gtk_object_set_data (GTK_OBJECT (pms_entry), "textbuffer", tbe);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(pms_entry), GTK_WRAP_WORD );
	gtk_text_buffer_get_end_iter( tbe, &end_iter );
	gtk_text_buffer_create_mark( tbe, "end_mark", &end_iter, 0 );

   } else {

  pms_entry = gtk_entry_new ();
  gtk_entry_set_max_length(GTK_ENTRY(pms_entry),425);
  gtk_widget_ref (pms_entry);
  gtk_object_set_data (GTK_OBJECT (pms_entry), "is_gtk_entry", "1");
  gtk_object_set_data_full (GTK_OBJECT (pm_session), "pms_entry", pms_entry,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_box_pack_start (GTK_BOX (hbox14), pms_entry, TRUE, TRUE, 0);

          }


  gtk_object_set_data(GTK_OBJECT(pms_entry),PROFNAMESTR , retrieve_profname_entry (profwidget) );
  	gtk_object_set_data(GTK_OBJECT (smile_button), "entry", pms_entry);
  	gtk_object_set_data(GTK_OBJECT (smile_button), "pmwindow", pm_session);
	style_text_entry(pms_entry);

  pms_send=gtk_button_new();
  gtk_container_add(GTK_CONTAINER(pms_send), get_pm_icon(8,_(" Send ")) );
  gtk_container_set_border_width(GTK_CONTAINER(pms_send), 1);
  gtk_button_set_relief(GTK_BUTTON(pms_send), GTK_RELIEF_NONE);
  set_tooltip(pms_send,_(" Send "));

	if (pmb_send_erase) {
  gtk_box_pack_start (GTK_BOX (hbox14), pms_send, FALSE, FALSE, 1);
	}

  pms_clear=gtk_button_new();
  gtk_container_add(GTK_CONTAINER(pms_clear), get_pm_icon(3,_(" Clear ")) );
  gtk_container_set_border_width(GTK_CONTAINER(pms_clear), 1);
  gtk_button_set_relief(GTK_BUTTON(pms_clear), GTK_RELIEF_NONE);
  set_tooltip(pms_clear,_(" Clear "));

	if (pmb_send_erase) {
  gtk_box_pack_start (GTK_BOX (hbox14), pms_clear, FALSE, FALSE, 1);
	}


  pms_profile=gtk_button_new();
  gtk_container_add(GTK_CONTAINER(pms_profile), get_pm_icon(7,_(" Profile ")) );
  gtk_container_set_border_width(GTK_CONTAINER(pms_profile), 0);
    gtk_button_set_relief(GTK_BUTTON(pms_profile), GTK_RELIEF_NONE);
  set_tooltip(pms_profile,_(" Profile "));
  gtk_widget_ref (pms_profile);
  gtk_object_set_data_full (GTK_OBJECT (pm_session), "pms_profile", pms_profile,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_box_pack_start (GTK_BOX (hbox13), pms_profile, FALSE, FALSE, 0);

   /* added: PhrozenSmoke - button for 'Buzz' feature */
  buzz_user=gtk_button_new();
  gtk_container_add(GTK_CONTAINER(buzz_user), get_pm_icon(1,_(" Buzz ")) );
  gtk_container_set_border_width(GTK_CONTAINER(buzz_user), 0);
    gtk_button_set_relief(GTK_BUTTON(buzz_user), GTK_RELIEF_NONE);
  set_tooltip(buzz_user,_(" Buzz "));
  gtk_box_pack_start (GTK_BOX (hbox13), buzz_user, FALSE, FALSE, 0);

   /* added: PhrozenSmoke - button for 'voice chat' feature */
  voice_chat=gtk_button_new();
  gtk_container_add(GTK_CONTAINER(voice_chat), get_pm_icon(9,_(" Voice Chat ")) );
  gtk_container_set_border_width(GTK_CONTAINER(voice_chat), 0);
    gtk_button_set_relief(GTK_BUTTON(voice_chat), GTK_RELIEF_NONE);
  set_tooltip(voice_chat,_(" Voice Chat "));
  gtk_box_pack_start (GTK_BOX (hbox13), voice_chat, FALSE, FALSE, 0);

  tuxviron=gtk_button_new();
  gtk_container_add(GTK_CONTAINER(tuxviron), get_pm_icon(11,_(" TUXVironment... ")) );
  gtk_container_set_border_width(GTK_CONTAINER(tuxviron), 0);
    gtk_button_set_relief(GTK_BUTTON(tuxviron), GTK_RELIEF_NONE);
  set_tooltip(tuxviron,_(" TUXVironment... "));
  gtk_box_pack_start (GTK_BOX (hbox13), tuxviron, FALSE, FALSE, 0);

  encbutton=gtk_button_new();
  gtk_container_add(GTK_CONTAINER(encbutton), get_pm_icon(14,_(" Encryption... ")) );
  gtk_container_set_border_width(GTK_CONTAINER(encbutton), 0);
    gtk_button_set_relief(GTK_BUTTON(encbutton), GTK_RELIEF_NONE);
  set_tooltip(encbutton,_(" Encryption... "));
  gtk_box_pack_start (GTK_BOX (hbox13), encbutton, FALSE, FALSE, 0);

   /* added: PhrozenSmoke - button for 'Send File..' feature */
  send_user=gtk_button_new();
  gtk_container_add(GTK_CONTAINER(send_user), get_pm_icon(4,_(" Send File... ")) );
  gtk_container_set_border_width(GTK_CONTAINER(send_user), 0);
    gtk_button_set_relief(GTK_BUTTON(send_user), GTK_RELIEF_NONE);
  set_tooltip(send_user,_(" Send File... "));
  gtk_box_pack_start (GTK_BOX (hbox13), send_user, FALSE, FALSE, 0);

   /* added: PhrozenSmoke - button for 'Join in chat..' feature */
  goto_user=gtk_button_new();
  gtk_container_add(GTK_CONTAINER(goto_user), get_pm_icon(6,_(" Join In Chat ")) );
    gtk_button_set_relief(GTK_BUTTON(goto_user), GTK_RELIEF_NONE);
  gtk_container_set_border_width(GTK_CONTAINER(goto_user), 0);
  set_tooltip(goto_user,_(" Join In Chat "));
  gtk_box_pack_start (GTK_BOX (hbox13), goto_user, FALSE, FALSE, 0);

  save_user=gtk_button_new();
  gtk_container_add(GTK_CONTAINER(save_user), get_pm_icon(10,_(" Save ")) );
  gtk_container_set_border_width(GTK_CONTAINER(save_user), 0);
  gtk_button_set_relief(GTK_BUTTON(save_user), GTK_RELIEF_NONE);
  set_tooltip(save_user,_(" Save "));
  gtk_box_pack_start (GTK_BOX (hbox13), save_user, FALSE, FALSE, 0);
  gtk_object_set_data (GTK_OBJECT (save_user), "pmstext", pms_text);
gtk_object_set_data(GTK_OBJECT (save_user), "pmwindow", pm_session);


	/* Sub-menu */

  menubar1 = gtk_menu_bar_new ();
 gtk_box_pack_start (GTK_BOX (hbox13), menubar1, FALSE, FALSE, 0);
  menu_head = gtk_image_menu_item_new ();
  gtk_container_add (GTK_CONTAINER (menubar1), menu_head);
  gtk_container_set_border_width (GTK_CONTAINER (menubar1), 0);
  gtk_container_set_border_width (GTK_CONTAINER (menu_head), 0);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_head), 
	GTK_WIDGET(get_pm_icon(17,"") ));
  pmst_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_head), pmst_menu);

  webcam_menu = gtk_image_menu_item_new_with_label (_(" View Webcam... "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(webcam_menu), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_ZOOM_100, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (pmst_menu), webcam_menu);
  gtk_object_set_data (GTK_OBJECT (pm_session), "pms_menu2", webcam_menu);

  startcam = gtk_image_menu_item_new_with_label (_(" Start My Webcam... "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(startcam), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_YES, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (pmst_menu), startcam);

  sendcam = gtk_image_menu_item_new_with_label (_(" Send Webcam Invitation... "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(sendcam), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DND, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (pmst_menu), sendcam);
  gtk_object_set_data (GTK_OBJECT (pm_session), "pms_menu1", sendcam);


  /* separator */
  gtk_container_add (GTK_CONTAINER (pmst_menu), gtk_menu_item_new ());

  inv_conf = gtk_image_menu_item_new_with_label (_("Invite User to Room or Conference..."));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(inv_conf ), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_CONVERT, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (pmst_menu),inv_conf);

  start_conf = gtk_image_menu_item_new_with_label (_("Start A Conference"));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(start_conf ), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DND, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (pmst_menu),start_conf);

  /* separator */
  gtk_container_add (GTK_CONTAINER (pmst_menu), gtk_menu_item_new ());

  yab = gtk_image_menu_item_new_with_label (_("Edit Contact Information..."));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(yab ), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_FIND_AND_REPLACE, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (pmst_menu),yab);
  gtk_object_set_data (GTK_OBJECT (pm_session), "yab", yab);

  /* separator */
  gtk_container_add (GTK_CONTAINER (pmst_menu), gtk_menu_item_new ());

  yphoto = gtk_image_menu_item_new_with_label (_("Share a photo album..."));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(yphoto ), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_SELECT_COLOR, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (pmst_menu),yphoto);
  gtk_object_set_data (GTK_OBJECT (pm_session), "yphoto", yphoto);


  /* separator */
  gtk_container_add (GTK_CONTAINER (pmst_menu), gtk_menu_item_new ());

  fadd1 = gtk_image_menu_item_new_with_label (_(" Add To Friends "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(fadd1 ), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_ADD, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (pmst_menu),fadd1);
  gtk_object_set_data (GTK_OBJECT (pm_session), "fadd1", fadd1);

  fadd2 = gtk_image_menu_item_new_with_label (_(" Add As Temporary Friend "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(fadd2 ), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_ADD, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (pmst_menu),fadd2);
  gtk_object_set_data (GTK_OBJECT (pm_session), "fadd2", fadd2);


  gtk_signal_connect (GTK_OBJECT (fadd1), "activate",
                      GTK_SIGNAL_FUNC (on_pms_add_friend),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (fadd2), "activate",
                      GTK_SIGNAL_FUNC (on_pms_add_friend),
                      fadd2);
  gtk_signal_connect (GTK_OBJECT (yab), "activate",
                      GTK_SIGNAL_FUNC (on_pms_get_yab),
                      NULL);
 gtk_signal_connect (GTK_OBJECT (yphoto), "activate",
                      GTK_SIGNAL_FUNC (on_pms_yphoto_start),
                      NULL);

  gtk_signal_connect (GTK_OBJECT (start_conf), "activate",
                      GTK_SIGNAL_FUNC (on_start_conference),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (webcam_menu), "activate",
                      GTK_SIGNAL_FUNC (on_pms_viewcam),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (inv_conf), "activate",
                      GTK_SIGNAL_FUNC (on_invite_to_conf),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (startcam), "activate",
                      GTK_SIGNAL_FUNC (on_startmycam),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (sendcam), "activate",
                      GTK_SIGNAL_FUNC (on_pms_sendcaminvite),
                      NULL);




  gtk_box_pack_start (GTK_BOX (hbox13), gtk_label_new("    "), FALSE, FALSE, 0);

  pms_ignore=gtk_button_new();
  gtk_container_add(GTK_CONTAINER(pms_ignore), get_pm_icon(5,_(" Ignore ")) );
  gtk_container_set_border_width(GTK_CONTAINER(pms_ignore), 0);
  gtk_button_set_relief(GTK_BUTTON(pms_ignore), GTK_RELIEF_NONE);
  set_tooltip(pms_ignore,_(" Ignore "));
  gtk_box_pack_start (GTK_BOX (hbox13), pms_ignore, FALSE, FALSE, 0);

  pms_status=gtk_label_new("                       ");  /* holds typing status */
  gtk_box_pack_start (GTK_BOX (hbox13), pms_status, TRUE, TRUE, 1);


  pms_close=gtk_button_new();
  gtk_container_add(GTK_CONTAINER(pms_close), get_pm_icon(2,_(" Close ")) );
  gtk_container_set_border_width(GTK_CONTAINER(pms_close), 0);
  set_tooltip(pms_close,_(" Close "));
  gtk_box_pack_start (GTK_BOX (hbox13), pms_close, FALSE, FALSE, 0);
  gtk_button_set_relief(GTK_BUTTON(pms_close), GTK_RELIEF_NONE);

  gtk_object_set_data (GTK_OBJECT (pm_session), "pms_status", pms_status);
  gtk_object_set_data (GTK_OBJECT (pm_session), "pms_send", pms_send);
  gtk_object_set_data (GTK_OBJECT (pm_session), "pms_clear", pms_clear);
  gtk_object_set_data (GTK_OBJECT (pm_session), "pms_profile", pms_profile);
  gtk_object_set_data (GTK_OBJECT (pm_session), "buzz_user", buzz_user);
  gtk_object_set_data (GTK_OBJECT (pm_session), "voice_chat", voice_chat);
  gtk_object_set_data (GTK_OBJECT (pm_session), "tuxviron", tuxviron);
  gtk_object_set_data (GTK_OBJECT (pm_session), "encbutton", encbutton);
  gtk_object_set_data (GTK_OBJECT (pm_session), "send_user", send_user);
  gtk_object_set_data (GTK_OBJECT (pm_session), "goto_user", goto_user);
  gtk_object_set_data (GTK_OBJECT (pm_session), "save_user", save_user);
  gtk_object_set_data (GTK_OBJECT (pm_session), "pms_ignore", pms_ignore);
  gtk_object_set_data (GTK_OBJECT (pm_session), "pms_close", pms_close);
  gtk_object_set_data (GTK_OBJECT (pm_session), "pm_aud", pm_aud);

  gtk_signal_connect (GTK_OBJECT (pm_session), "delete_event",
                      GTK_SIGNAL_FUNC (on_pm_session_delete_event),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (pms_text), "button_press_event",
                      GTK_SIGNAL_FUNC (on_pms_text_button_press_event),
                      NULL);

	 if (pm_window_style != 1) {
  gtk_signal_connect (GTK_OBJECT (pms_entry), "activate",
                      GTK_SIGNAL_FUNC (on_pms_entry_activate),
                      NULL);
	}
  gtk_signal_connect (GTK_OBJECT (pms_entry), "key_press_event",
                      GTK_SIGNAL_FUNC (on_pms_entry_key_press_event),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (pms_send), "clicked",
                      GTK_SIGNAL_FUNC (on_pms_send_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (pms_clear), "clicked",
                      GTK_SIGNAL_FUNC (on_pms_clear_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (pms_profile), "clicked",
                      GTK_SIGNAL_FUNC (on_pms_profile_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (send_user), "clicked",
                      GTK_SIGNAL_FUNC (on_pms_sendfile_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (goto_user), "clicked",
                      GTK_SIGNAL_FUNC (on_pms_goto_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (pms_close), "clicked",
                      GTK_SIGNAL_FUNC (on_pms_close_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (pms_ignore), "clicked",
                      GTK_SIGNAL_FUNC (on_pms_ignore_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (buzz_user), "clicked",
                      GTK_SIGNAL_FUNC (on_pms_buzz_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (voice_chat), "clicked",
                      GTK_SIGNAL_FUNC (on_activate_voice_conf),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (save_user), "clicked",
                      GTK_SIGNAL_FUNC (on_pmsavefile),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (tuxviron), "clicked",
                      GTK_SIGNAL_FUNC (open_tuxvironment_selection),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (encbutton), "clicked",
                      GTK_SIGNAL_FUNC (open_encryption_selection),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (pm_aud), "clicked",
                      GTK_SIGNAL_FUNC (on_open_audible_window),
                      NULL);


  if (show_bimages) { 
	GtkWidget *vbox_extra, *bim_box;
	vbox_extra=gtk_hbox_new (FALSE, 0);
	bim_box=get_display_image_bar(pms_entry);
	gtk_container_set_border_width (GTK_CONTAINER (bim_box), 3);
	gtk_box_pack_start (GTK_BOX (vbox_extra), hbox_main, TRUE, TRUE, 1);
	gtk_box_pack_start (GTK_BOX (vbox_extra), bim_box, FALSE, FALSE, 1);
  	gtk_container_add (GTK_CONTAINER (pm_session), vbox_extra);
  	gtk_object_set_data(GTK_OBJECT(pm_session),"panel" ,gtk_object_get_data(GTK_OBJECT(bim_box), "panel") );
  	gtk_object_set_data(GTK_OBJECT(pm_session),"bimage_me" ,gtk_object_get_data(GTK_OBJECT(bim_box), "bimage_me") );
  	gtk_object_set_data(GTK_OBJECT(pm_session),"bimage_them" ,gtk_object_get_data(GTK_OBJECT(bim_box), "bimage_them") );
 	gtk_object_set_data(GTK_OBJECT(gtk_object_get_data(GTK_OBJECT(bim_box), "bimage_butt")),"textfocus" , pms_entry);

  } else {
	gtk_container_add (GTK_CONTAINER (pm_session), hbox_main);
	   }


  /* This was sloppy: original author unregistered PM session on clicking button "Close"
       but not when we clicked the window's X ??? - was causing seg-fault: phrozensmoke  */
  gtk_signal_connect (GTK_OBJECT (pm_session), "delete_event",
                      GTK_SIGNAL_FUNC (on_pms_close_clicked),
                      NULL);

  gtk_widget_show_all (hbox_main);
  return pm_session;
}

GtkWidget* create_pm_window (void)
{
  GtkWidget *pm_window;
  GtkWidget *vbox4;
  GtkWidget *hbox4;
  GtkWidget *pm_user_label;
  GtkWidget *pm_user;
  GtkWidget *pm_entry;
  GtkWidget *hbox5;
  GtkWidget *pm_send;
  GtkWidget *pm_cancel;
  GtkWidget *profwidget;

  pm_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_wmclass (GTK_WINDOW (pm_window), "gyachEDialog", "GyachE"); 
  gtk_object_set_data (GTK_OBJECT (pm_window), "pm_window", pm_window);
  /* gtk_widget_set_usize (pm_window, 310, 80); */
  gtk_window_set_modal(GTK_WINDOW(pm_window), TRUE);  /* needs to be true : PhrozenSmoke */
  gtk_window_set_title (GTK_WINDOW (pm_window), _("Yahoo! Instant Message"));
  gtk_window_set_default_size (GTK_WINDOW (pm_window), 390, -1);

  vbox4 = gtk_vbox_new (FALSE, 5);
  gtk_container_add (GTK_CONTAINER (pm_window), vbox4);
  gtk_container_set_border_width (GTK_CONTAINER (vbox4), 5);

  	gtk_box_pack_start (GTK_BOX (vbox4), GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_NEW, GTK_ICON_SIZE_DND)), FALSE, FALSE , 3);
  gtk_widget_show_all (vbox4);

  profwidget=get_profile_name_component(1);
  gtk_box_pack_start (GTK_BOX (vbox4), profwidget, FALSE, FALSE, 2);

  hbox4 = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox4), hbox4, FALSE, FALSE, 0);
  gtk_widget_show (hbox4);

  pm_user_label = gtk_label_new (_("Username: "));
  gtk_widget_ref (pm_user_label);
  gtk_object_set_data_full (GTK_OBJECT (pm_window), "pm_user_label", pm_user_label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_box_pack_start (GTK_BOX (hbox4), pm_user_label, FALSE, FALSE, 0);
  gtk_widget_show (pm_user_label);
  gtk_misc_set_padding (GTK_MISC (pm_user_label), 5, 0);

  pm_user = gtk_entry_new ();
  gtk_entry_set_max_length(GTK_ENTRY(pm_user),64);
  gtk_widget_ref (pm_user);
  gtk_object_set_data_full (GTK_OBJECT (pm_window), "pm_user", pm_user,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_box_pack_start (GTK_BOX (hbox4), pm_user, TRUE, TRUE, 0);
  gtk_widget_show (pm_user);

  pm_entry = gtk_entry_new ();
  style_text_entry(pm_entry);
  gtk_entry_set_max_length(GTK_ENTRY(pm_entry),425);
  gtk_widget_ref (pm_entry);
  gtk_object_set_data_full (GTK_OBJECT (pm_window), "pm_entry", pm_entry,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_box_pack_start (GTK_BOX (vbox4), pm_entry, FALSE, FALSE, 0);
  gtk_widget_show (pm_entry);

  gtk_object_set_data(GTK_OBJECT(pm_entry),PROFNAMESTR , retrieve_profname_entry (profwidget) );

  hbox5 = gtk_hbox_new (TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox4), hbox5, FALSE, FALSE, 0);
  gtk_widget_show (hbox5);

  pm_send = get_pixmapped_button(_(" Send "), GTK_STOCK_APPLY);
  set_tooltip(pm_send,_(" Send "));
  gtk_widget_ref (pm_send);
  gtk_object_set_data_full (GTK_OBJECT (pm_window), "pm_send", pm_send,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_box_pack_start (GTK_BOX (hbox5), pm_send, FALSE, FALSE, 0);
  gtk_widget_show (pm_send);

  pm_cancel = get_pixmapped_button(_(" Cancel "), GTK_STOCK_CANCEL);
  set_tooltip(pm_cancel ,_(" Cancel "));
  gtk_widget_ref (pm_cancel);
  gtk_object_set_data_full (GTK_OBJECT (pm_window), "pm_cancel", pm_cancel,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_box_pack_start (GTK_BOX (hbox5), pm_cancel, FALSE, FALSE, 0);
  gtk_widget_show (pm_cancel);

  gtk_signal_connect_after (GTK_OBJECT (pm_window), "delete_event",
                            GTK_SIGNAL_FUNC (on_pm_window_destroy_event),
                            NULL);
  gtk_signal_connect (GTK_OBJECT (pm_entry), "activate",
                      GTK_SIGNAL_FUNC (on_pm_entry_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (pm_send), "clicked",
                      GTK_SIGNAL_FUNC (on_pm_send_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (pm_cancel), "clicked",
                      GTK_SIGNAL_FUNC (on_pm_cancel_clicked),
                      NULL);

  return pm_window;
}


/* added: PhrozenSmoke - an easier method for opening 'blank' PM windows
     or windows specifying a screen name */

GtkWidget *blank_pm_window (char *some_sn)
{
	GtkWidget *tmp_widget;
	GtkWidget *pm_window_widget;

	pm_window_widget = create_pm_window();
	gtk_widget_show(pm_window_widget);

	if (some_sn != NULL) {
		if (strlen(some_sn)>0 && strlen(some_sn)<39) {
			tmp_widget = lookup_widget( pm_window_widget, "pm_user" );
			gtk_entry_set_text( GTK_ENTRY(tmp_widget), some_sn);
											  										}
								   }
return pm_window_widget;

}
