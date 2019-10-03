/*****************************************************************************
 * tuxvironments.c
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
 * This is BASIC code for TUXVironments, a very basic representation of IMVironments
 * for GYach Enhanced, right now this just handles IMvironments sent from Windows/Mac
 * users by showing a static background image in the PM window: No fancy animations, 
 * interaction, or ability to play games with the other user. Most of the interactive 
 * IMvironments sent from the 'official' client use Java/Flash/JS/DHTML, and proprietary code 
 * stored in an 'imv' component: If somebody wishes to try to reverse engineer it
 * knock yourself out, for now this module handles IMvironments with nice, but 
 * static background images.
 *****************************************************************************/


#include <ctype.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <expat.h>
#include <malloc.h>

#include "animations.h"
#include "yahoochat.h"
#include "profname.h"
#include "plugins.h"


int enable_tuxvironments=1;
GList *tuxvironment_list = NULL;
extern char *_(char *some);  /* added PhrozenSmoke: locale support */ 
extern GtkWidget *get_pixmapped_button(char *button_label, const gchar *stock_id);
extern void set_tooltip(GtkWidget *somewid, char *somechar);
extern void focus_pm_entry(GtkWidget *pmwin);
extern GtkWidget *get_pm_icon(int which_icon, char *err_str) ;
char *current_tuxvironment=NULL;
static	GdkPixmap *pmback = NULL;
char tuxy_tmp[64];
int tux_silent_redraw=0;

int set_pmwindow_background (GtkWidget *pmwidget, GtkWidget *pmwindow, char *filename) {

	GdkWindow *pmwin;
	int width, height;
	GdkPixbuf *pixbuf;
	GdkPixbuf *spixbuf;

	//g_warning("trans-1\n");
	pmwin = gtk_text_view_get_window( GTK_TEXT_VIEW(pmwidget), GTK_TEXT_WINDOW_TEXT );
	//g_warning("trans-1aa\n");
	gdk_window_get_size( pmwin, &width, &height);
	//g_warning("trans-1a\n");
	if (pmback) {	gdk_pixmap_unref( pmback ); pmback=NULL; }
	//g_warning("trans-1b\n");
	pixbuf = gdk_pixbuf_new_from_file( filename, NULL );
	if (!pixbuf) {
		char imvbuf[320];
		snprintf(imvbuf,318, "  %s%s* %s: '%s' *\n%s%s", YAHOO_STYLE_ITALICON, "\033[#FFB8AFm", _("TUXVironment is not available"), filename, YAHOO_STYLE_ITALICOFF, YAHOO_COLOR_BLACK);
		if (!tux_silent_redraw) {append_to_textbox_color( pmwindow,pmwidget, imvbuf );}
		return 0;
				}
	spixbuf = gdk_pixbuf_scale_simple( pixbuf, width, height,
					GDK_INTERP_BILINEAR );
	//g_warning("trans-2a\n");
	gdk_pixbuf_render_pixmap_and_mask (spixbuf, &pmback, NULL, 0);
	//g_warning("trans-2b\n");
	gdk_window_set_back_pixmap( pmwin, pmback, FALSE );
	//g_warning("trans-3a\n");
	gtk_widget_queue_draw( GTK_WIDGET(pmwidget));
	//g_warning("trans-3\n");
	if (pixbuf) {g_object_unref(pixbuf);}
	//g_warning("trans-4\n");
	if (spixbuf) {g_object_unref(spixbuf);}
	//g_warning("trans-5\n");
	return 1;
}


char *find_tuxvironment( char *imv ) {
	GList *this_tux;

	if (!tuxvironment_list) {return NULL;}
	if (current_tuxvironment) {free(current_tuxvironment); current_tuxvironment=NULL;}
	this_tux = tuxvironment_list;
	while( this_tux ) {
		if ( ! strncasecmp( this_tux->data, imv , strlen(this_tux->data))) {			
			current_tuxvironment=strdup(this_tux->data);
			return current_tuxvironment;
		}
		this_tux = g_list_next( this_tux );
	}

	return current_tuxvironment;
}


int tuvs_callback_enc(void *enc_data, const XML_Char *name, XML_Encoding *info) {return 0;}
void tuvs_callback_cdata(void *user_data, const XML_Char *data, int len) {return;}
void tuvs_callback_end(void *user_data, const XML_Char *name) {	return; }

void tuvs_callback_start(void *user_data, const XML_Char *name, const XML_Char **attrs) {
	if (strncmp((char*)name,"tuxv",4)==0) {
		if (!attrs) {return;}
		if (! *attrs) {return;}
		else {
			XML_Char **cptr=(XML_Char **) attrs;
			while (cptr && *cptr) {
				if (strncmp(cptr[0],"iname",5)==0) {
					tuxvironment_list = g_list_append( tuxvironment_list, strdup(cptr[1]));
				}
				cptr += 2;
				}
		}
	}
}

int load_xml_tuxvironments() {
	char filename[192];
	int fd;
	int bytes;
	XML_Parser p;
	void *buff;
	snprintf(filename,190,"%s/tuxvironments/gytuxvironments.xml",  PACKAGE_DATA_DIR);
	fd = open( filename, O_RDONLY, 0600 );
	if ( fd == -1 ) {return( 0 );	}
	p = XML_ParserCreate(NULL);      /* XML_ParserCreate( "iso-8859-1"); */
	XML_SetElementHandler(p, tuvs_callback_start, tuvs_callback_end);
	XML_SetCharacterDataHandler(p, tuvs_callback_cdata);
	XML_SetUnknownEncodingHandler(p, tuvs_callback_enc, NULL);
	XML_SetUserData(p, "");
		buff = XML_GetBuffer(p, 6144);		
		if (buff == NULL) {	return -1;}
		bytes = read( fd, (char *)buff, 6100 );
		close( fd ); 
		if (! XML_ParseBuffer(p, bytes, bytes == 0)) { 	return 1;	}
	XML_ParserFree(p);	
	return 1;
}

void create_tuxvironment_list() {
	tuxvironment_list = g_list_append( tuxvironment_list, _("[NONE]"));
	load_xml_tuxvironments();
}

void set_tuxvironment (GtkWidget *pmwidget, GtkWidget *pmwindow, char *tuxviron) {
	char imfile[192]="";
	char imvbuf[192]="";
	char *tttmp;
	char *found_tux=NULL;
	if (!enable_tuxvironments) {
			snprintf(imvbuf,190, "  %s%s* %s *\n%s%s", YAHOO_STYLE_ITALICON, "\033[#FFB8AFm", _("TUXVironments are not currently enabled"),  YAHOO_STYLE_ITALICOFF, YAHOO_COLOR_BLACK);
			append_to_textbox_color( pmwindow,pmwidget, imvbuf );
			return;
									  }
	if (!tuxviron) {return;}

	if (!tuxvironment_list) {create_tuxvironment_list();}
	if (!tuxvironment_list) {return;}

	tttmp = strchr(tuxviron, ';');
	if(tttmp) {*tttmp = '\0';}
	if (strlen(tuxviron)<1) {return;}

	if (!strcmp(tuxviron, "TUXVIRONMENT_OFF" )) {found_tux="empty";}
	else {found_tux=find_tuxvironment(tuxviron);}

	if (!found_tux) {		
		//g_warning("tuxviron not supported");
		snprintf(imvbuf,190, "  %s%s* %s: '%s' *\n%s%s", YAHOO_STYLE_ITALICON, "\033[#FFB8AFm", _("TUXVironment is not supported"), tuxviron, YAHOO_STYLE_ITALICOFF, YAHOO_COLOR_BLACK);
		if (!tux_silent_redraw) {append_to_textbox_color( pmwindow,pmwidget, imvbuf );}
		return; 
					  }
	snprintf(imfile,190, "%s/tuxvironments/%s.jpg",PACKAGE_DATA_DIR,found_tux);
	if (set_pmwindow_background (pmwidget, pmwindow, imfile)) {

	  if (strcmp(tuxviron, "TUXVIRONMENT_OFF" )) {
		snprintf(imvbuf,190, "  %s%s* %s: '%s' *\n%s%s", YAHOO_STYLE_ITALICON, "\033[#D8BEFFm", _("TUXVironment loaded"), found_tux, YAHOO_STYLE_ITALICOFF, YAHOO_COLOR_BLACK);
		if (!tux_silent_redraw) {append_to_textbox_color( pmwindow,pmwidget, imvbuf );}
																	}
																						  }

}

gboolean tuxviron_redraw_notify_event (GtkWidget *widget,
		GdkEvent *event, gpointer user_data) {
	char *tuxy=NULL;
	GtkWidget *ptext=NULL;
	GtkWidget *pwin=NULL;
	pwin=gtk_object_get_data(GTK_OBJECT(widget), "pmwindow");
	if (!pwin) { return( FALSE );}
	ptext=gtk_object_get_data(GTK_OBJECT(pwin), "pms_text");
	if (!ptext) {return( FALSE );}
	tuxy=gtk_object_get_data(GTK_OBJECT(pwin), "imv");
	if (!tuxy) {return( FALSE );}
	if (!strcmp(tuxy, ";0")) {return(FALSE);}
	if (tuxy) {
		char *tcolon=NULL;
		snprintf(tuxy_tmp, 62, "%s",tuxy );
		tcolon=strchr(tuxy_tmp,';');
		if (tcolon) {*tcolon='\0';}
		tux_silent_redraw=1;
		set_tuxvironment (ptext, pwin, tuxy_tmp);
		/* printf("redrawn\n"); fflush(stdout); */
		tux_silent_redraw=0;
	}
	return( FALSE );
}


void deactivate_tuxvironment(PM_SESSION *pms) {
	char *current_tux=NULL;
	char imfile[192]="";

	if (!pms) {return;}


	current_tux=gtk_object_get_data (GTK_OBJECT (pms->pm_window), "imv");
	if (current_tux) {
		/* already deactivated */
		/* if (!strcmp(current_tux,";0")) {return;}  */
		free(current_tux);
						}
	set_current_pm_profile_name(retrieve_profname_str(GTK_WIDGET(retrieve_profname_entry(pms->pm_window)))  );
	if (! pms->sms) {ymsg_imvironment(ymsg_sess, pms->pm_user, ";0");}
	reset_current_pm_profile_name();
	gtk_object_set_data (GTK_OBJECT (pms->pm_window), "imv",strdup(";0"));
	snprintf(imfile,190, "%s/tuxvironments/empty.jpg",PACKAGE_DATA_DIR);
	set_pmwindow_background (pms->pm_text, pms->pm_window, imfile);
}

void on_close_tuxviron_win_cb(GtkWidget *button, gpointer user_data) {
	GtkWidget *tuxwin;
	tuxwin=gtk_object_get_data(GTK_OBJECT(button),"window");
	if (tuxwin) {gtk_widget_destroy(tuxwin); }
}

void activate_tuxvironment(GtkWidget *button, gpointer user_data) {
	PM_SESSION *pms;
	GtkWidget *tuxwin;
	GtkWidget *tentry;
	char *stuff=NULL;
	char *tsel=NULL;

	tuxwin=gtk_object_get_data(GTK_OBJECT(button),"window");
	pms=gtk_object_get_data(GTK_OBJECT(button),"session");
	tentry=gtk_object_get_data(GTK_OBJECT(button),"entry");
	if (!pms) { if (tuxwin) {gtk_widget_destroy(tuxwin);} return;}
	if (!tentry) { if (tuxwin) {gtk_widget_destroy(tuxwin);} return;}

	stuff=strdup(gtk_entry_get_text(GTK_ENTRY(tentry)));

	if (!strcmp(stuff,_("[NONE]"))) {  /* turn them off */
		deactivate_tuxvironment(pms);

		 								   } else {		
		tsel=find_tuxvironment( stuff );
		if (tsel) {

			if ((!enable_tuxvironments) || (pms->sms) ) {
				char imvbuf[160]="";
				snprintf(imvbuf,158, "  %s%s* %s *\n%s%s", YAHOO_STYLE_ITALICON, "\033[#FFB8AFm", _("TUXVironments are not currently enabled"),  YAHOO_STYLE_ITALICOFF, YAHOO_COLOR_BLACK);
				append_to_textbox_color( pms->pm_window,pms->pm_text, imvbuf );
									  			  } else {
			char imv[24]="";
			int is_yphoto=0;
			char *current_tux=NULL;
			snprintf(imv,23,"%s",tsel);
			set_tuxvironment (pms->pm_text, pms->pm_window, imv);
			snprintf(imv,23,"%s;0",stuff);
			if (!strcmp(stuff,"photos")) {
				is_yphoto=1;
			}
			current_tux=gtk_object_get_data (GTK_OBJECT (pms->pm_window), "imv");
			if (current_tux) {free(current_tux);}
			gtk_object_set_data (GTK_OBJECT (pms->pm_window), "imv",strdup(imv));
	set_current_pm_profile_name(retrieve_profname_str(GTK_WIDGET(retrieve_profname_entry(pms->pm_window)))  );
			if (! is_yphoto) {ymsg_imvironment(ymsg_sess, pms->pm_user, imv);}
			reset_current_pm_profile_name();
			if (is_yphoto) {plugins_on_yphoto_session_start(pms);}
															}
					}
													}

	free(stuff);
	if (tuxwin) {gtk_widget_destroy(tuxwin); focus_pm_entry(pms->pm_window);}
}


GtkWidget *create_list_dialog(char *wintitle, char *header, char *ldefault, GList *comblist, int editable, int icon) {
	GtkWidget *tuxwin;
	GtkWidget *vbox_main;
        GtkWidget *okbutton;
	GtkWidget *tentry;
	GtkWidget *tcombo, *bbox_main, *closeb;

  tuxwin = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_wmclass (GTK_WINDOW (tuxwin), "gyachEDialog", "GyachE"); 
  gtk_window_set_title (GTK_WINDOW (tuxwin), wintitle);
  gtk_window_set_position (GTK_WINDOW (tuxwin), GTK_WIN_POS_MOUSE);
  gtk_window_set_modal (GTK_WINDOW (tuxwin), TRUE);

  vbox_main = gtk_vbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (vbox_main), 6);

  gtk_container_add (GTK_CONTAINER (tuxwin), vbox_main);

  gtk_box_pack_start (GTK_BOX (vbox_main), get_pm_icon(icon,""), FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (vbox_main), gtk_label_new(header), FALSE, FALSE, 5);

  tcombo=gtk_combo_new();
  tentry=GTK_COMBO(tcombo)->entry;
  gtk_editable_set_editable(GTK_EDITABLE(tentry),editable );
  gtk_entry_set_text(GTK_ENTRY(tentry),ldefault?ldefault:_("[NONE]"));

  gtk_box_pack_start (GTK_BOX (vbox_main), tcombo, FALSE, FALSE, 4);
  if (comblist) {gtk_combo_set_popdown_strings( GTK_COMBO(tcombo), comblist );}

  bbox_main = gtk_hbox_new (FALSE, 0);

  gtk_box_pack_start (GTK_BOX (vbox_main), gtk_label_new(" "), TRUE, TRUE, 3);
  gtk_box_pack_start (GTK_BOX (vbox_main), bbox_main, FALSE, FALSE, 6);

  okbutton=  get_pixmapped_button(_("OK"), GTK_STOCK_APPLY);
  set_tooltip(okbutton,_("OK"));
  gtk_box_pack_start (GTK_BOX (bbox_main), okbutton, FALSE, FALSE, 2);

  gtk_box_pack_start (GTK_BOX (bbox_main), gtk_label_new("   "), TRUE, TRUE, 2);

  closeb=get_pixmapped_button(_("Cancel"), GTK_STOCK_CLOSE);
  set_tooltip(closeb,_("Cancel"));
  gtk_box_pack_start (GTK_BOX (bbox_main), closeb, FALSE, FALSE, 2);
  gtk_object_set_data (GTK_OBJECT (closeb), "window",tuxwin);
  gtk_signal_connect (GTK_OBJECT (closeb), "clicked",
                      GTK_SIGNAL_FUNC (on_close_tuxviron_win_cb),
                      NULL);

  gtk_object_set_data (GTK_OBJECT (okbutton), "window",tuxwin);
  gtk_object_set_data (GTK_OBJECT (okbutton), "entry",tentry);
  gtk_widget_show_all(tuxwin);
  return okbutton;
}


void create_tuxvironment_window(PM_SESSION *pms) {
	GtkWidget *okbutton;
  	 if (!pms) {return;}
	if (!tuxvironment_list) {create_tuxvironment_list();}
	if (!tuxvironment_list) {return;}

	okbutton=create_list_dialog(_("TUXVironment"), _("Select a TUXVironment:"), NULL, tuxvironment_list, FALSE, 11);

  gtk_object_set_data (GTK_OBJECT (okbutton), "session",pms);
  gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC (activate_tuxvironment),
                      NULL);
}


void open_tuxvironment_selection(GtkWidget *button, gpointer user_data) {
	PM_SESSION *pms;
	pms=gtk_object_get_data(GTK_OBJECT(button),"session");
	if (!pms) {return;}
	create_tuxvironment_window(pms);
}


