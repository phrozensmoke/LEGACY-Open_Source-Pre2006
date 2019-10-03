/* Mainly a wrapper for launching the external web cam 
    viewer gyache-webcam, but handles the necessary 
    Ymsg negotiations inside Gyach */

/*****************************************************************************
 * webcam.c
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
 * VERY preliminary code for handling webcams - much of this code is 
 * borrowed from the Ayttm/libyahoo2 projects
 *****************************************************************************/

#include "webcam.h"
#include "sounds.h"
#include "friends.h"
#include "profname.h"

extern int enable_webcam_features;
extern char *_(char *some);  /* added PhrozenSmoke: locale support */ 

char *lastcamwho =NULL;

extern GtkWidget *show_confirm_dialog_config(char *mymsg, char *okbuttonlabel, char *cancelbuttonlabel, int with_callback);
extern void show_ok_dialog(char *mymsg);  /* added: PhrozenSmoke */

char *webcam_viewer_app=NULL;
char *webcam_device=NULL;

int launch_my_cam=0;

void yahoo_webcam_get_feed(char *who)
{
	if (!who) {return;}
	if (lastcamwho) {free(lastcamwho); lastcamwho=NULL; launch_my_cam=0;}
	if (!enable_webcam_features) {
		show_ok_dialog(_("Sorry, You have the webcam feature disabled."));	
		return;
												}
	lastcamwho=strdup(who);

	if (!strcasecmp(lastcamwho, get_default_profile_name())) {launch_my_cam=1;}

	/* This is REALLY stupid, Yahoo will send us a webcam key with NO name 
	    attached, so all we can do is save the last requested name and hope 
	    they match up!  */

	ymsg_get_webcam(ymsg_sess, lastcamwho);
 }

char *make_webcam_friends( ) {
	GList *this_friend;
	char *frmsg=malloc(10240);
	if (!frmsg) {return strdup("");}
	sprintf( frmsg, "%s", "");
	this_friend = friend_list;
	while( this_friend ) {
		if (strlen(frmsg)>10155) {break;} /* avoid buffer overflow */
		if ( this_friend != friend_list ) {	strcat( frmsg, "," );}
		strcat( frmsg, this_friend->data );
		this_friend = g_list_next( this_friend );
	}
	return frmsg;
}


void yahoo_process_webcam_key(char *key)
{	
	if (!enable_webcam_features) {return;}
	if (!key) {return;}
	if (lastcamwho) {

		if (launch_my_cam) {  /* launch the webcam uploader */
			struct stat sbuf;
			char *launcher=NULL;
			char *myfriends=NULL;
		
			launcher=malloc(256);
			if (!launcher) {return ;}

			snprintf(launcher,254, "%s/webcam/gyache-upload", PACKAGE_DATA_DIR); 

			if ( stat( launcher, &sbuf ))  {
				char wmsg[304];
				snprintf(wmsg, 302, "%s:\n\n%s", _("The Gyach Enhanced external webcam broadcaster could not be found"), 	launcher);
				show_ok_dialog(wmsg);
				free(lastcamwho); lastcamwho=NULL;
				launch_my_cam=0;
				free(launcher); launcher=NULL;
				return; 
														}
		
			/* TODO: add friends list as last argument */
			if (!webcam_device) {webcam_device=strdup("/dev/video0");}

			myfriends=make_webcam_friends( );
			free(launcher); launcher=NULL;
			launcher=malloc(10642);
			if (!launcher) { free(myfriends); return ;}

			snprintf(launcher,10640, "%s/webcam/gyache-upload \"%s\" \"%s\" \"%s\"  \"%s\" &",
			PACKAGE_DATA_DIR, get_default_profile_name(), key, webcam_device , myfriends
			); 

			my_system(launcher);
			free(lastcamwho); lastcamwho=NULL;
			free(myfriends); myfriends=NULL;
			free(launcher); launcher=NULL;
			launch_my_cam=0;
			return ;
		}

		else {
		/* run EXTERNAL VIEWER HERE */
		struct stat sbuf;
		char launcher[384];
		int launchertype=0;

		if (! webcam_viewer_app) {webcam_viewer_app=strdup("Gyach-E Webcam");}
		if (strstr(webcam_viewer_app, "pY WebCam")) {launchertype=1;}

		/* check to see if the external viewer is there */

			/* C-based cam viewer */
		if (! launchertype) { 
		snprintf(launcher,250, "%s/webcam/gyache-webcam", PACKAGE_DATA_DIR); 
									} else {   /* Python-based viewer */
		snprintf(launcher,250, "%s/webcam/pywebcam.py", PACKAGE_DATA_DIR); 
											  }

		if ( stat( launcher, &sbuf ))  {
			char wmsg[304];
			snprintf(wmsg, 302, "%s:\n\n%s", _("The Gyach Enhanced external webcam viewer could not be found"), 	launcher);
			show_ok_dialog(wmsg);
			return; 
													}


		if (! launchertype) {  /* C-based viewer */
		snprintf(launcher,382, "%s/webcam/gyache-webcam \"%s\" \"%s\" \"%s\" &",
		PACKAGE_DATA_DIR, lastcamwho, get_default_profile_name(), key
		); 
									} else { /* Python-based viewer */
		snprintf(launcher,382, "python %s/webcam/pywebcam.py \"%s\" \"%s\" \"%s\" &",
		PACKAGE_DATA_DIR, lastcamwho, get_default_profile_name(), key
		); 
											 }

		my_system(launcher);
		free(lastcamwho); lastcamwho=NULL;
			} /* end else */
					  }
}

void gyache_yahoo_webcam_invite_callback(char *who, int result)
{
	if (!enable_webcam_features) {return;}
	if(result) {
		ymsg_webcam_invite_accept( ymsg_sess, who);
		yahoo_webcam_get_feed(who);
			} else { ymsg_webcam_invite_reject( ymsg_sess, who);}
}

void on_webcam_invite_accept   (GtkWidget       *button,  gpointer         user_data)
{	
	GtkWidget *tmp_widget;
	char *who=NULL;
	who = gtk_object_get_data( GTK_OBJECT(button), "who" );
	if (who) {gyache_yahoo_webcam_invite_callback(who,1); 	free(who);}
	tmp_widget=gtk_object_get_data(GTK_OBJECT(button), "mywindow");
	if (tmp_widget) {gtk_widget_destroy(tmp_widget);}
}

void on_webcam_invite_reject   (GtkWidget       *button,  gpointer         user_data)
{
	GtkWidget *tmp_widget;
	char *who=NULL;
	who = gtk_object_get_data( GTK_OBJECT(button), "who" );
	if (who) {gyache_yahoo_webcam_invite_callback(who,0); 	free(who);}
	tmp_widget=gtk_object_get_data(GTK_OBJECT(button), "mywindow");
	if (tmp_widget) {gtk_widget_destroy(tmp_widget);}
}


void yahoo_webcam_invite_msg(char *who)
{
	char buff[256];
	GtkWidget *okbutton=NULL;
	GtkWidget *cbutton=NULL;

	if (!enable_webcam_features) {
		/* we aren't allowing webcam, so auto-reject invites */
		/* packet_handler should catch this first, but just in case */
		ymsg_webcam_invite_reject( ymsg_sess, who);
		return;
												}

	snprintf(buff, 254, _("The yahoo user '%s' has invited you to view their webcam. Do you want to accept?"), who);

				okbutton=show_confirm_dialog_config(buff,"Yes","No",0);
				if (!okbutton) {ymsg_webcam_invite_reject( ymsg_sess, who); return;}
  				gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_webcam_invite_accept), NULL);
				gtk_object_set_data (GTK_OBJECT (okbutton), "who", strdup(who));

				cbutton=gtk_object_get_data( GTK_OBJECT(okbutton), "cancel" );
				if (cbutton) {
  					gtk_signal_connect (GTK_OBJECT (cbutton), "clicked",
                      		GTK_SIGNAL_FUNC (on_webcam_invite_reject), NULL);
				gtk_object_set_data (GTK_OBJECT (cbutton), "who", strdup(who));
						 }

				play_sound_event(SOUND_EVENT_OTHER);
}



/* This should be handled in GyachE, not in the external cam viewer:
    A viewer sent a Ymsg packet accepting/rejecting
   a cam invite we sent them  - this is already handled by packet_handler.c 
   appropriately */

/*  
void yahoo_webcam_invite_reply(char *from, int accept)
{
}
*/ 



