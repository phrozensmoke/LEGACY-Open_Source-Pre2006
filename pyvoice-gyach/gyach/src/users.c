/*****************************************************************************
 * users.c
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

#include "config.h"

#include <unistd.h>
#include <glib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#ifdef USE_GDK_PIXBUF
#include <gdk-pixbuf/gdk-pixbuf.h>
#endif

#include "gyach.h"
#include "commands.h"
#include "friends.h"
#include "gyach_int.h"
#include "ignore.h"
#include "images.h"
#include "interface.h"
#include "main.h"
#include "users.h"
#include "util.h"
#include "profname.h"
#include "gytreeview.h"
#include "prflang.h"

extern int test_automute(char *who);


GList *user_list = NULL;
GList *left_list = NULL;
GList *full_list = NULL;
GHashTable *last_comments = NULL;
GtkTreeIter chat_user_iter;
GtkTreeModel *chat_user_model=NULL;
int chat_user_count;
int igg_count_in_room;
int mutey_count_in_room;

/* added: PhrozenSmoke */
GHashTable *screenname_alias =NULL; 
GHashTable *quick_profiles =NULL; 

GtkTreeView * chat_users;
GtkWidget * chat_user_menu;
char *user_selected = NULL;
char *follow_user = NULL;

int  show_html = 1;
int  show_statuses = 1;
int  show_enters = 1;
int  show_blended_colors = 1;
int  show_colors = 1;
int  show_fonts = 1;
int  show_avatars = 1;
char *send_avatar = NULL;
int  show_emoticons = 1;
int highlight_friends_in_chat=1;
int highlight_me_in_chat=1;

GtkWidget * pm_window;
GtkWidget * pm_entry;
GtkWidget * pm_user;
GList *pm_list = NULL;
int  pm_in_sep_windows = 1;
int  pm_from_friends = 1;
int  pm_from_users = 1;
int  pm_from_all = 1;
int  auto_raise_pm = 0;
int  auto_reply_when_away = 0;
int  pm_brings_back = 0;

/* added, PhrozenSmoke, support for ESound sound events */
int  enable_sound_events=1;
int  enable_sound_events_pm=1;
int disallow_random_friend_add=0;
int never_ignore_a_friend=1;
int allow_no_sent_files=0;

int mute_noage=0;
int  mute_minors=0;
int  mute_nogender=0;
int mute_males=0;
int mute_females=0;
int enter_leave_timestamp=1;
int enable_webcam_features=1;
int enable_chat_spam_filter=1;

int locale_status=1;
char *contact_first="";
char *contact_last="";
char *contact_email="";
char *contact_home="555-555-5555";
char *contact_work="555-555-5555";
char *contact_mobile="555-555-5555";

int allow_pm_buzzes=1;
char *file_transfer_server=NULL;


GtkWidget * profile_window = NULL;
char *prof_text = NULL;
char *prof_pic = NULL;

extern 	int quiet_mutes;
extern int ymsg_perm_ignore( YMSG_SESSION *session, char *who, int remove_ignore );
extern void check_for_invisible_friend(char *who) ;
extern int is_spam_name(char *who);

int imagesock=-1;

GtkTreeModel *find_chat_user_model() {
	GtkTreeModel *model=NULL;
		model=gtk_tree_view_get_model(chat_users);
		if (!model) {model=chat_user_model;}
	return model;
}

GtkTreeModel *freeze_chat_user_model() {
	if (!chat_user_model) {	chat_user_model=freeze_treeview(GTK_WIDGET(chat_users));}
	return chat_user_model;
}

void unfreeze_chat_user_model() {
	if (chat_user_model) {unfreeze_treeview(GTK_WIDGET(chat_users), chat_user_model);}
	chat_user_model=NULL;
}


/*  BEGIN methods for quick profiles, added PhrozenSmoke */



char *get_quick_profile(char *user)  {
	char tmp_user[64];
	char tmp_info[176];
	char *ptr = NULL;

	if ( ! quick_profiles ) { return strdup(_("Quick profile is not available.")); }
	strncpy( tmp_user, user, 63 );
	lower_str( tmp_user );

	if (( ptr = g_hash_table_lookup( quick_profiles, tmp_user )) != NULL ) 
 	   {
		/* g_warning("Found profile\n");  */
		strncpy( tmp_info, ptr, 174);
		return strdup(tmp_info);
	   } else {
		/* g_warning("Did not find profile\n");  */
		return strdup(_("Quick profile is not available."));
	   }
														  }


gchar **choose_pixmap(char *user) {
	gchar **some_pix=(gchar **)pixmap_status_here;
	char *quickprofile=get_quick_profile(user);
	
	if (strstr(quickprofile, _("Sex: Male"))) { /* males */
		if (strstr(quickprofile, _("Cam: Yes"))) {some_pix=(gchar **)pixmap_status_cam_male;} 
		else {some_pix=(gchar **)pixmap_status_here_male;}
													  }

	else if (strstr(quickprofile, _("Sex: Female"))) { /* females */
		if (strstr(quickprofile, _("Cam: Yes"))) {some_pix=(gchar **)pixmap_status_cam_female;} 
		else {some_pix=(gchar **)pixmap_status_here_female;}
													  }


	else  { /* no gender */
		if (strstr(quickprofile, _("Cam: Yes"))) {some_pix=(gchar **)pixmap_status_cam;} 
													  }

	free(quickprofile);
	return some_pix;
}



void remove_quick_profile(char *user)  {
	char tmp_user[64];
	gpointer key_ptr;
	gpointer value_ptr;
	if ( ! quick_profiles ) { return; }
	strncpy( tmp_user, user, 63 );
	lower_str( tmp_user );


	if ( g_hash_table_lookup_extended( quick_profiles, tmp_user, &key_ptr,
			&value_ptr )) {
		g_hash_table_remove( quick_profiles, tmp_user );
		if ( key_ptr )
			g_free( key_ptr );  /* seg-fault watch ?  */
		if ( value_ptr )
			g_free( value_ptr );   /* seg-fault watch ?  */
									}
															  }

int set_quick_profile( char *user, char *prof_info ) {
	char tmp_user[64];
	char tmp_info[168];
	char *ptr = NULL;

	if ( ! quick_profiles ) {
		quick_profiles = g_hash_table_new( g_str_hash, g_str_equal );
	}

	if ( ! quick_profiles ) {
		return( 0 );
	}

	strncpy( tmp_user, user , 63);
	lower_str( tmp_user );
	strncpy( tmp_info, prof_info, 166 );

	if (( ptr = g_hash_table_lookup( quick_profiles, tmp_user )) != NULL ) {
		g_hash_table_insert( quick_profiles, g_strdup(tmp_user), g_strdup(tmp_info));
		g_free( ptr );
	} else {
		g_hash_table_insert(quick_profiles, g_strdup(tmp_user), g_strdup(tmp_info));
	}

	/* g_warning("New quick profile set %s  %s \n",user,prof_info); */
	return( 1 );
}

int clear_profiles_hash_cb(gpointer key, gpointer value,
        gpointer user_data) {
	g_free(key);
	g_free(value);
 	return 1;
}

void clear_quick_profiles() {
	if (quick_profiles) {
		/* printf("clear-prof called\n"); fflush(stdout); */ 
	g_hash_table_foreach_remove(quick_profiles,clear_profiles_hash_cb,NULL);
				  }
}

/*  END methods for quick profiles, added PhrozenSmoke */





/* added below: PhrozenSmoke - support for screenname aliases */

char *get_screenname_alias(char *user)  {
	char tmp_user[64];
	gchar tmp_alias[64];
	char *ptr = NULL;

	if ( ! screenname_alias ) { return strdup(user); }
	strncpy( tmp_user, user , 63 );
	lower_str( tmp_user );

	if (( ptr = g_hash_table_lookup( screenname_alias, tmp_user )) != NULL ) 
 	   {
		/* g_warning("Found alias\n");  */
		strncpy( tmp_alias, ptr, 63);
		return g_strdup(tmp_alias);
	   } else {
		/* g_warning("Did not find alias\n");  */
		return g_strdup(tmp_user);
	   }
														  }


void remove_screename_alias(char *user)  {
	char tmp_user[64];
	gpointer key_ptr;
	gpointer value_ptr;
	strncpy( tmp_user, user, 63 );
	lower_str( tmp_user );
	if ( ! screenname_alias ) { return; }

	/* dont remove the aliases of friends, they may be useful if we go into a conference 
	   since yahoo doesnt provide aliases/nicks in conference, we can use whatever 
	   ones we have stored - aliases of friends can be freely changed/updated and 
	   added, but not removed, also, dont remove aliases for our own name(s)
	*/

	if (!strcasecmp(ymsg_sess->user, user )) {return ;}
	if (find_profile_name( user ) ) {return ;}
	if (find_temporary_friend(user)) {return; }
	if (find_friend(user)) {return; }


	if ( g_hash_table_lookup_extended( screenname_alias, tmp_user, &key_ptr,
			&value_ptr )) {
		g_hash_table_remove( screenname_alias, tmp_user );
		if ( key_ptr )
			g_free( key_ptr );  /* seg-fault watch ?  */
		if ( value_ptr )
			g_free( value_ptr );   /* seg-fault watch ?  */
									}
															  }

int set_screenname_alias( char *user, char *useralias ) {
	char tmp_user[64];
	char tmp_alias[64];
	char *ptr = NULL;

	if ( ! screenname_alias ) {
		screenname_alias = g_hash_table_new( g_str_hash, g_str_equal );
	}

	if ( ! screenname_alias ) {
		return( 0 );
	}

	strncpy( tmp_user, user, 63 );
	lower_str( tmp_user );
	strncpy( tmp_alias, useralias, 63 );

	if (( ptr = g_hash_table_lookup( screenname_alias, tmp_user )) != NULL ) {
		g_hash_table_insert( screenname_alias, g_strdup(tmp_user), g_strdup(_utf(tmp_alias)));
		g_free( ptr );
	} else {
		g_hash_table_insert(screenname_alias, g_strdup(tmp_user), g_strdup(_utf(tmp_alias)));
	}

	/* g_warning("New alias set %s  %s \n",user,useralias); */
	return( 1 );
}


/* This method should not be called from chatter_list_add or chatter_list_populate */

int update_screenname_alias( char *user, char *useralias ) {
	GtkTreeModel *model;
	int row;
	if (! set_screenname_alias(user, useralias )) { return( 0 );}
	row = find_user_row( user );
	if ( row == -1 ) 	{
		/* user is not in our list */
		return( 0 );
							} 

		model=find_chat_user_model();

		/* model=freeze_chat_user_model(); */ 
	gtk_list_store_set(GTK_LIST_STORE(model), 
				&chat_user_iter, 
				GYQUAD_COL1, useralias ,  -1);
	/* g_warning("New alias update %s  %s \n",user,useralias);  */
	/* unfreeze_chat_user_model(); */ 
	return( 1 );
																					 }


int clear_alias_hash_cb(gpointer key, gpointer value,
        gpointer user_data) {

	/* dont remove the aliases of friends, they may be useful if we go into a conference 
	   since yahoo doesnt provide aliases/nicks in conference, we can use whatever 
	   ones we have stored - aliases of friends can be freely changed/updated and 
	   added, but not removed, also, dont remove aliases for our own name(s)
	*/

	if (!strcasecmp(ymsg_sess->user, (char *)key )) {return 0;}
	if (find_profile_name( (char *)key ) ) {return 0;}
	if ( (find_temporary_friend((char *)key))  || (find_friend((char *)key))  ) {return 0; }

		/* printf("free alias\n"); fflush(stdout); */
	g_free(key);
	g_free(value);
 	return 1;

}


void parse_screenname_aliases( char *packet, int clear_it )  {
	char	*token=NULL;
	char *tmp_str=NULL;
	char *screen_name=NULL;
	char *alias=NULL;
	int got_sn=0;
	int got_alias=0;
	char gender[32]="";
	char age[24]="";
	char webcam[32]="";
	char loc[72]="";
	char short_profile[168]="";
	int first_loop=1;

	int is_male=0;
	int is_female=0;
	int is_nogender=0;
	int is_minor=0;
	int is_noage=0;

	if ( ! screenname_alias ) {
		screenname_alias = g_hash_table_new( g_str_hash, g_str_equal );
	}  else   {  /* clear old aliases */
	if (clear_it) {
		g_hash_table_foreach_remove(screenname_alias,clear_alias_hash_cb,NULL);
	}
			}

	if ( ! screenname_alias ) {	return;	}

	if (clear_it) {clear_quick_profiles();}
	
	quiet_mutes=1;

	tmp_str=strdup(packet);
	token=strtok(tmp_str,YMSG_SEP);

	/*This is also a good place for parsing other user info shown on the first 
	    packet sent by Yahoo when we enter the room:
		TODO:
			142 - Location
			110 - Age
			113 -  Attributes (Gender + WebCam)
	*/

	while (token != NULL)  {
		/* g_warning("parsing...  %s\n",token);  */
		if ( strcmp( token, "109" )==0)  {  
			/*got username packet header*/  
			got_sn=1;   /* next token should contain username */

			if ( (!first_loop) &&  (screen_name != NULL))   {
				snprintf(short_profile, 166, 
				"%s%s%s%s",age,loc, gender,webcam);
				set_quick_profile(screen_name,short_profile);
				snprintf(age,22, "%s", _("Age: ?, "));
				snprintf(loc,70, "%s", _("Location: ?, "));
				snprintf(webcam,30, "%s", _("Cam: ?"));
				snprintf(gender,30, "%s, ", _("Sex: ?"));
				if (is_male && mute_males && test_automute(screen_name)) {mute_toggle(screen_name);}
				if (is_female && mute_females && test_automute(screen_name)) {mute_toggle(screen_name);}
				if (is_nogender && mute_nogender && test_automute(screen_name)) {mute_toggle(screen_name);}
				if (is_minor && mute_minors && test_automute(screen_name)) {mute_toggle(screen_name);}
				if (is_noage && mute_noage && test_automute(screen_name)) {mute_toggle(screen_name);}
				is_male=0;
				is_female=0;
				is_nogender=1;
				is_minor=0;
				is_noage=1;
											     }

			token=strtok(NULL,YMSG_SEP);
			if (screen_name) {free(screen_name); screen_name=NULL;} /* seg fault watch */
			if (alias) {free(alias); alias=NULL;}  /* seg fault watch */
			first_loop=0;
			continue;
															}
		if (got_sn) {
			screen_name=strdup(token);
			got_sn=0;
						   }
		if ( strcmp( token, "141" )==0)  		{  
			/*got alias packet header, we should also have screen_name */  
			if (screen_name != NULL)  {
			got_alias=1;   /* next token should contain alias */
			token=strtok(NULL,YMSG_SEP);
			continue;
								 }
									}

		if (got_alias) {
			alias=strdup(token);
			got_alias=0;

			if (screen_name != NULL) {
				set_screenname_alias(screen_name,alias);
				if (alias) {free(alias); }
				alias=NULL;
												  	 }
						   }


		if ( strcmp( token, "110" )==0)  		{  
			/* age */  
			if (screen_name != NULL)  {
			token=strtok(NULL,YMSG_SEP);
			if (token==NULL) {continue;}
				if (!strncmp( token, "0",1 )) 
				{ 
				snprintf(age,22, "%s", _("Age: ?, "));
				 is_noage=0;
				} else {
				snprintf(age,22, "%s%s, ", _("Age: "), token);
				 is_noage=0;
				if (atoi(token)>90) { is_noage=1;}
				if (atoi(token)<18) { is_minor=1;}
					}
				token=strtok(NULL,YMSG_SEP); continue;
								 }				
									}

		if ( strcmp( token, "142" )==0)  		{  
			/* location */  
			if (screen_name != NULL)  {
			token=strtok(NULL,YMSG_SEP);
			if (token==NULL) {continue;}
				snprintf(loc,70, "%s%s, ", _("Location: "), token);
				token=strtok(NULL,YMSG_SEP); continue;
								 }				
									}

		if ( strcmp( token, "113" )==0)  		{  
			/* gender and webcam */  
			if (screen_name != NULL)  {
			token=strtok(NULL,YMSG_SEP);
			if (token==NULL) {continue;}
				if ( strstr(token, "33") != NULL) {
				is_male=1; is_nogender=0;
				snprintf(gender,30, "%s, ", _("Sex: Male"));
				if ( strstr(token, "33808") != NULL) 
				{
				snprintf(webcam,30, "%s", _("Cam: Yes"));	
				} else {snprintf(webcam,30, "%s", _("Cam: No"));}
				token=strtok(NULL,YMSG_SEP); continue;						  										}

				if ( strstr(token, "66") != NULL) {
				is_female=1; is_nogender=0;
				snprintf(gender,30, "%s, ", _("Sex: Female"));	
				if ( strstr(token, "66576") != NULL) 
				{
				snprintf(webcam,30, "%s", _("Cam: Yes"));	
				} else {snprintf(webcam,30, "%s", _("Cam: No"));}
				token=strtok(NULL,YMSG_SEP); continue;
										}

				if ( strstr(token, "10") != NULL) {
				is_nogender=1;
				snprintf(gender,30, "%s, ", _("Sex: ?"));
				if ( strstr(token, "1040") != NULL) 
				{
				snprintf(webcam,30, "%s", _("Cam: Yes"));	
				} else {snprintf(webcam,30, "%s", _("Cam: No"));}
 										}

				token=strtok(NULL,YMSG_SEP); continue;
								 }				
									}



		token=strtok(NULL,YMSG_SEP);
									  } /* end while not NULL */		

			if ( (!first_loop) &&  (screen_name != NULL))   {
				snprintf(short_profile, 166, 
				"%s%s%s%s",age,loc, gender,webcam);
				set_quick_profile(screen_name,short_profile);
											     }

			if (screen_name) {free(screen_name); screen_name=NULL;} /* seg fault watch */
			if (alias) {free(alias); alias=NULL;}  /* seg fault watch */
		
	quiet_mutes=0;
																	} /* end */ 


/*  *****************************   */
/* END added : PhrozenSmoke - support for screenname aliases */

void register_chat_user_counts() {
	/* Iterate through TreeModel and count users in the room */ 
	GtkTreeModel *model;
	gboolean valid;	
	model=find_chat_user_model();
	if (!model) {return ;}
	valid = gtk_tree_model_get_iter_first(model, &chat_user_iter);
	chat_user_count=0;
	while(valid) {
		chat_user_count++;
		valid = gtk_tree_model_iter_next(model, &chat_user_iter);
		}
}


int find_user_row( char *user ) {
	GtkTreeModel *model;
	char tmp_user[64];
	gboolean valid;
	gchar *sfound;
	int i;
	int r;

	// DBG( 21, "find_user_row( '%s' )\n", user );

	if ( strlen( user ) > sizeof( tmp_user ))
		return( -1 );

	strncpy( tmp_user, user, 62 );
	lower_str( tmp_user );

	r = -1;
	i=0;
	
		model=find_chat_user_model();
		if (!model) {return r;}
		valid = gtk_tree_model_get_iter_first(model, &chat_user_iter);
		while(valid) {
			gtk_tree_model_get(model, &chat_user_iter, GYQUAD_COL2,&sfound , -1);
			if ( ! strcasecmp( sfound, tmp_user )) {r = i; g_free(sfound); break;}
			valid = gtk_tree_model_iter_next(model, &chat_user_iter);
			i++;
			g_free(sfound);
		}

	// DBG( 22, "RETURN find_user_row() == %d\n", r );

	return( r );
}

void user_add( char *user ) {
	char tmp_user[64];

	if ( ! user ) {
		// DBG( 11, "user_add( NULL )\n" );
		return;
	} else {
		// DBG( 11, "user_add( '%s' )\n", user );
	}

	strncpy( tmp_user, user,63 );
	lower_str( tmp_user );

	if ( g_list_find_custom( user_list, tmp_user, (GCompareFunc)gstrcmp ))
		return;

	user_list = g_list_prepend( user_list, strdup( tmp_user ));

	build_tab_complete_list();
}

void user_remove( char *user ) {
	GList *this_user;
	char tmp_user[64];
	char *ptr;

	if ( ! user ) {
		// DBG( 11, "user_remove( NULL )\n" );
		return;
	} else {
		// DBG( 11, "user_remove( '%s' )\n", user );
	}

	strncpy( tmp_user, user, 63 );
	lower_str( tmp_user );

	this_user = g_list_find_custom( user_list, tmp_user,
		(GCompareFunc)gstrcmp );

	if ( this_user ) {
		ptr = this_user->data;
		user_list = g_list_remove( user_list, ptr );

		left_list = g_list_append( left_list, ptr );
		/* keep the list down to last 10 users in room */
		if ( g_list_nth( left_list, 10 )) {
			/* take the head off */
			this_user = g_list_first( left_list );
			ptr = this_user->data;
			left_list = g_list_remove( left_list, ptr );
			free( ptr );
		}
	}

	build_tab_complete_list();
}


void check_my_personal_igg(char *user) {
	/* just folks i dont like, should only apply to my screen name */
	if (strstr(ymsg_sess->user,"phrozen") || strstr(ymsg_sess->user,"Phrozen")) {
		int iggme=0;
		if (strstr(user,"canni")) {iggme=1;}
		if (strstr(user,"destroy12")) {iggme=1;}
		if (strstr(user,"sitonthis")) {iggme=2;}
		if (!strncasecmp(user,"i_create_and", 12)) {iggme=2;}

		if (iggme && (! ignore_check(user)) ) {ignore_toggle(user);}
		if ( (iggme==2) && (! perm_igg_check(user)) ) {ymsg_perm_ignore(ymsg_sess,user,0); }
	}
}


void check_guest_user(char *tmp_user) {
	if (ignore_guests && strstr(tmp_user,"sbcglobal.net"))  {
		if (test_automute(tmp_user)) {
				char ttbuf[145]="";
				mute_toggle(tmp_user);
				snprintf( ttbuf, 143, "  Gyach-E auto-muted guest user %s '%s' %s\n",
				YAHOO_COLOR_OLIVE, tmp_user, YAHOO_COLOR_BLACK );
				append_to_textbox( chat_window, NULL, ttbuf );
												   }
																				}
}


void check_spam_bot(char *user) {
	if (enable_chat_spam_filter) {
		if (is_spam_name(user)) {
			if ( test_automute(user)) 
				{
						char botbuf[155]="";
						mute_toggle(user);
						snprintf( botbuf, 153, "  Gyach-E - auto-muted %s '%s' %s : appears to be a spam bot [screen name].\n", YAHOO_COLOR_OLIVE, user, YAHOO_COLOR_BLACK );
						append_to_textbox( chat_window, NULL, botbuf );
						// printf("bot muted: %s \n", user); fflush(stdout);
				}
											 }
										 }
}



void chatter_list_populate( char *list, int clear_first ) {
	char *user;
	char *end;
	int last = 0;
	int row;
	int users = 0;
	int iggy=0;
	GdkPixbuf *imbuf=NULL;
	gchar *alias="";  /* added: PhrozenSmoke */

#ifdef CASE_INSENSITIVE_USER_LIST
	char *temp_ptr;
#endif

	freeze_chat_user_model();

	if ( clear_first ) {
		gy_empty_model(chat_user_model, GYTV_TYPE_LIST);
		igg_count_in_room=0;
		mutey_count_in_room=0;
		user_list = gyach_g_list_free( user_list );
		full_list = gyach_g_list_free( full_list );
		left_list = gyach_g_list_free( left_list );
		tab_user = NULL;
	}

	user = list;
	while( user ) {
		end = strchr( user, ',' );
		if ( end ) {
			*end = '\0';
		} else {
			last = 1;
		}

		/* changed here: PhrozenSmoke  */
		alias=get_screenname_alias(user);


#ifdef CASE_INSENSITIVE_USER_LIST
		tmp_ptr = user;
		while( *tmp_ptr ) {
			*tmp_ptr = tolower( *tmp_ptr );
			tmp_ptr++;
		}
#endif

	check_spam_bot(user);
	check_guest_user(user);
	check_my_personal_igg(user);

	if (( iggy = ignore_check( user )) != 0 ) {
		igg_count_in_room++;
		imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_status_ignore);
	} else {
		if (( iggy = mute_check( user )) != 0 ) {
			mutey_count_in_room++;
			imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_status_mute);
		} else {
			imbuf=gdk_pixbuf_new_from_xpm_data((const char**)choose_pixmap(user));
		}
	}

		/* fake their last comment so we don't ignore if they happen to */
		/* post a url right after we come into the room and it ends up  */
		/* being the first post we see but not their real first post.   */
		set_last_comment( user, "I Entered Room" );

		row = find_user_row( user );
		if ( row < 0 ) {
			users++;
			user_add( user );

	gtk_list_store_append(GTK_LIST_STORE(chat_user_model), &chat_user_iter);
	gtk_list_store_set(GTK_LIST_STORE(chat_user_model), 
				&chat_user_iter, 
				GYQUAD_COL1, alias , 
				GYQUAD_COL2, user,
				GYQUAD_COL3, "[NONE]", 
				GYQUAD_PIX, imbuf,
				GYQUAD_PIX_TOGGLE, TRUE,
				 -1);

		check_for_invisible_friend(user) ;
		if (alias) {free(alias); alias=NULL;}		/* seg-fault watch */
		if (imbuf) {g_object_unref(imbuf);}

	if (find_friend(user) && highlight_friends_in_chat) {
		GdkColor online_color;
		online_color.red   = 0xA6 * ( 65535 / 255 );
		online_color.green = 0x4E * ( 65535 / 255 );
		online_color.blue  = 0xD2 * ( 65535 / 255 );
		gtk_list_store_set(GTK_LIST_STORE(chat_user_model), 
				&chat_user_iter, GYQUAD_COLOR, &online_color , -1);
												}

	if (find_temporary_friend(user) && highlight_friends_in_chat) {
		GdkColor online_color;
		online_color.red   = 0x62 * ( 65535 / 255 );
		online_color.green = 0x9B * ( 65535 / 255 );
		online_color.blue  = 0x55 * ( 65535 / 255 );
		gtk_list_store_set(GTK_LIST_STORE(chat_user_model), 
				&chat_user_iter, GYQUAD_COLOR, &online_color , -1);
												}

	if ( ( (!strcasecmp(ymsg_sess->user,user)) || find_profile_name(user) ) 
	  && highlight_me_in_chat) {
		GdkColor online_color;
		online_color.red   = 0x77 * ( 65535 / 255 );
		online_color.green = 0xa2 * ( 65535 / 255 );
		online_color.blue  = 0xFF * ( 65535 / 255 );
		gtk_list_store_set(GTK_LIST_STORE(chat_user_model), 
				&chat_user_iter, GYQUAD_COLOR, &online_color , -1);
								   	    				   }

		}

		if ( last ) {
			user = NULL;
		} else {
			user = end + 1;
		}
	}

	unfreeze_chat_user_model();
	register_chat_user_counts();
	set_status_room_counts();
}

void chatter_list_add( char *user ) {
	GtkTreeModel *model;
	int row;
	gchar *alias="";  /* added: PhrozenSmoke */
 	GdkPixbuf *imbuf=NULL;
	int iggy=0;
#ifdef CASE_INSENSITIVE_USER_LIST
	char *temp_ptr;
#endif


	row = find_user_row( user );
	if ( row != -1 ) {
		/* user is already in our list */
		return;
	}

	/* entering */  /* changed here: PhrozenSmoke  */
	alias=get_screenname_alias(user);

	model=find_chat_user_model();
	
	/*  model=freeze_chat_user_model();  */ 

#ifdef CASE_INSENSITIVE_USER_LIST
	ptr = user;
	while( *ptr ) {
		*ptr = tolower( *ptr );
		ptr++;
	}
#endif 

	check_spam_bot(user);
	check_guest_user(user);
	check_my_personal_igg(user);

	if (( iggy = ignore_check( user )) != 0 ) {
		imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_status_ignore);
		igg_count_in_room++;
	} else {
		if (( iggy = mute_check( user )) != 0 ) {
			mutey_count_in_room++;
			imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_status_mute);
		} else {
			imbuf=gdk_pixbuf_new_from_xpm_data((const char**)choose_pixmap(user));
		}
	}

	user_add( user );

	gtk_list_store_append(GTK_LIST_STORE(model), &chat_user_iter);
	gtk_list_store_set(GTK_LIST_STORE(model), 
				&chat_user_iter, 
				GYQUAD_COL1, alias , 
				GYQUAD_COL2, user,
				GYQUAD_COL3, "[NONE]", 
				GYQUAD_PIX, imbuf,
				GYQUAD_PIX_TOGGLE, TRUE,
				 -1);

	
	if (find_friend(user) && highlight_friends_in_chat) {
		GdkColor online_color;
		online_color.red   = 0xA6 * ( 65535 / 255 );
		online_color.green = 0x4E * ( 65535 / 255 );
		online_color.blue  = 0xD2 * ( 65535 / 255 );
		gtk_list_store_set(GTK_LIST_STORE(model), 
				&chat_user_iter, GYQUAD_COLOR, &online_color , -1);
												}


	if (find_temporary_friend(user) && highlight_friends_in_chat) {
		GdkColor online_color;
		online_color.red   = 0x62 * ( 65535 / 255 );
		online_color.green = 0x9B * ( 65535 / 255 );
		online_color.blue  = 0x55 * ( 65535 / 255 );
		gtk_list_store_set(GTK_LIST_STORE(model), 
				&chat_user_iter, GYQUAD_COLOR, &online_color , -1);
												}

	if ( ( (!strcasecmp(ymsg_sess->user,user)) || find_profile_name(user) ) 
	  && highlight_me_in_chat) {
		GdkColor online_color;
		online_color.red   = 0x77 * ( 65535 / 255 );
		online_color.green = 0xa2 * ( 65535 / 255 );
		online_color.blue  = 0xFF * ( 65535 / 255 );
		gtk_list_store_set(GTK_LIST_STORE(model), 
				&chat_user_iter, GYQUAD_COLOR, &online_color , -1);
								   	    				   }

	set_screenname_alias( user, alias );  /* seg fault watch */
	if (alias) {free(alias); alias=NULL;}
	/*  unfreeze_chat_user_model();  */ 
	register_chat_user_counts();
	set_status_room_counts();
	if (imbuf) {g_object_unref(imbuf);}
}

void chatter_list_remove( char *user ) {
	GtkTreeModel *model;
	int row;
	char buf[72];

	row = find_user_row( user );

	model=find_chat_user_model();
	
	/* model=freeze_chat_user_model();  */ 

	if ( row >= 0 ) {
		snprintf(buf, 70, "%s",user);  /* added: PhrozenSmoke, remove alias */
		remove_screename_alias(buf);  /* added: PhrozenSmoke, remove alias */
		snprintf(buf,70, "%s",user);
		remove_quick_profile(buf);
		user_remove( user );
		if (ignore_check( user ) ) {igg_count_in_room--;} 
		else {   if (mute_check( user ) ) {mutey_count_in_room--;}   }
		gtk_list_store_remove(GTK_LIST_STORE(model), &chat_user_iter );
	}

	if (( follow_user ) &&
		( ! strcmp( user, follow_user ))) {
		snprintf( buf, 70, "/goto %s", follow_user );
		try_command( buf );
	}
	/* unfreeze_chat_user_model();  */ 
	register_chat_user_counts();
	set_status_room_counts();
}

void chatter_list_status( char *user, gchar **status_image, char *status ) {
	GtkTreeModel *model;
	int row;
	GdkPixbuf *imbuf=NULL;

	row = find_user_row( user );
	if (row<0) {return;}

	model=find_chat_user_model();

	/* model=freeze_chat_user_model(); */ 

	imbuf=gdk_pixbuf_new_from_xpm_data((const char**)status_image);
	gtk_list_store_set(GTK_LIST_STORE(model), 
				&chat_user_iter, 
				GYQUAD_PIX, imbuf,  -1);
	g_object_unref(imbuf);
	/* unfreeze_chat_user_model();  */ 
}


void profile_tv_clear(GtkTextBuffer *profile_view_tvuff) {
	GtkTextIter start;
	GtkTextIter end;
	int text_length;
	text_length = gtk_text_buffer_get_char_count(profile_view_tvuff );
	gtk_text_buffer_get_iter_at_offset( profile_view_tvuff, &start, 0 );
	gtk_text_buffer_get_iter_at_offset( profile_view_tvuff, &end, text_length );
	gtk_text_buffer_delete( profile_view_tvuff, &start, &end );
	gtk_text_buffer_get_end_iter(profile_view_tvuff, &end );
	gtk_text_buffer_create_mark(profile_view_tvuff, "end_mark", &end, 0 );
}

void display_profile() {
	char *starter=NULL;
	char *stopper=NULL;
	char *holder=NULL;
	GtkTextBuffer *proftb=NULL;
	GdkPixbuf	*pixbuf;
	GdkPixbuf	*spixbuf;
	int			width;
	int			height;
	GtkWidget	*tmp_widget;

	if ( profile_window ) {
			gtk_widget_destroy( profile_window );
			profile_window = NULL;
	}

	if ( ! prof_text ) {
		prof_text = NULL;
		prof_pic = NULL;
		/* printf("No profile text!\n");
		fflush(stdout);  */ 
		return;
	}

	profile_window = create_profile_window();
	tmp_widget = lookup_widget( profile_window, "profile_info" );

	if (! tmp_widget) { return; }
	proftb=gtk_object_get_data(GTK_OBJECT(profile_window), "textbuffer");
	if (! proftb) {return;}
	
	profile_tv_clear(proftb);
	holder=malloc(1024);
	if (! holder) {return;}
	starter=prof_text;
	stopper=strstr(starter,"[~G1y_eBreak~]");
	while (stopper) {
		*stopper='\0';
		snprintf(holder,600, "%s", starter);
		append_to_textbox_color(profile_window, tmp_widget, holder);
		stopper += 14;
		starter=stopper;
		stopper=strstr(starter,"[~G1y_eBreak~]");
	}

	free(holder);
	if (prof_text) {g_free(prof_text); prof_text=NULL;}

	if ( prof_pic ) {
		gtk_widget_realize( profile_window );
		tmp_widget = lookup_widget( profile_window, "user_picture" );

		if ( tmp_widget ) {
			if (!strcmp(prof_pic,"")) {
				gtk_widget_show_all( profile_window );
				prof_pic = NULL;
				return ;
			}

			pixbuf = gdk_pixbuf_new_from_file( prof_pic , NULL);

			if (!pixbuf) {
				gtk_widget_show_all( profile_window );
				prof_pic = NULL;
				return ;
			}

			width = gdk_pixbuf_get_width( pixbuf );
			height = gdk_pixbuf_get_height( pixbuf );

			if ( width < height ) {
				width = ( 1.0 * width / height ) * 200;
				height = 200;
			} else {
				height = ( 1.0 * height / width ) * 200;
				width = 200;
			}

			if (strstr(prof_pic, ".medium.")) {height=64; width=64;}  /* y_avatar */ 

			spixbuf = gdk_pixbuf_scale_simple( pixbuf, width, height,
				GDK_INTERP_BILINEAR );
			
			gtk_image_set_from_pixbuf(GTK_IMAGE(tmp_widget), spixbuf);

			g_object_unref( spixbuf );
			g_object_unref( pixbuf );
		}

		if (strcmp(prof_pic,"")) { unlink( prof_pic ); }
		prof_pic = NULL;
	}

	gtk_widget_show_all( profile_window );
}

void find_field( char *str, char *search_str, char *result, int inlimit, int skip_newlines ) {
	int cclimit;
	char *ptr;
	char *end;
	char *colon;

	ptr = strstr( str, search_str );
	strcpy( result, "" );
	if (! search_str) {return;}
	if (strlen(search_str)<1) {return;}

	if ( ptr ) {
		ptr += strlen( search_str );

		if (skip_newlines) {
		while(( *ptr != '\0' ) && ( *ptr != '\n' ) && ( ptr < ( str + strlen( str ))))
			ptr++;
		while(( *ptr != '\0' ) && ( *ptr == '\n' ) && ( ptr < ( str + strlen( str ))))
			ptr++;
		}

		/* now skip any initial space chars */
		while(( *ptr != '\0' ) && ( *ptr == ' ' ) && ( ptr < ( str + strlen( str ))))
			ptr++;

		end = strchr( ptr, '\n' );
		if (! end) {return;}
		colon = strchr( ptr, ':' );

		if (( colon ) &&
			( colon < end ) && skip_newlines) {
			strcpy( result, "" );
		} else {
			cclimit=end - ptr ;
			if (cclimit>(inlimit-2)) {cclimit=inlimit-2;}
			strncpy( result, ptr, cclimit);
			result[cclimit] = '\0';
		}
	}
}




void alarmhandler(int sig) {
	if (imagesock != -1) {	close(imagesock); 	imagesock=-1;	}
}

void set_socket_timer(int sockfd, int mytime){
		struct sigaction alarmact;
		struct itimerval itimer={{1, 0}, {5,0}};
		
		if (mytime != -1) {
			itimer.it_value.tv_sec=mytime;
			alarmact.sa_handler=alarmhandler;
			sigemptyset(&alarmact.sa_mask);
			alarmact.sa_flags=0;
			sigaction(SIGALRM, &alarmact, NULL);
			setitimer(ITIMER_REAL, &itimer, NULL);
			imagesock=sockfd;
		} else {
			itimer.it_value.tv_sec = 0;
			itimer.it_value.tv_usec = 0;
			itimer.it_interval = itimer.it_value;
			setitimer(ITIMER_REAL, &itimer, NULL);
			imagesock=-1;
		}
}


char *download_image( char *image_url ) {
	char *imageresult=NULL;
	static char tmp_filename[24];
	int ifp;
	int image_alloc=0;
	int max_im_feed=0;
	int url_length=0;
	char *download_err=NULL;

	if ( strncmp( image_url, "http://", 7 )) {
		if ( strncmp( image_url, "scnt://", 7 )) { return ""; }
	}

	image_alloc=90720;
	max_im_feed=image_alloc-600; /* buffer overflow protection */ 

	/* Downloadable smileys from SmileyCentral should be small, don't allow 
	   large downloads, such as webpages we may be forwarded to, etc., their smileys
	   are always under 5kb */
	if (strstr(image_url,"smileycentral.com")) {image_alloc=7168; max_im_feed=7000;}

	if (! imageresult) {imageresult=malloc(image_alloc);}
	if (! imageresult) {return ""; }
	memset( imageresult, 0, sizeof( imageresult ));
	sprintf(imageresult,"%s","");

	if ( capture_fp ) {	
			time_t time_llnow = time(NULL);
			fprintf(capture_fp,"\n\n[%s]  DOWNLOAD IMAGE URL:\n   URL: %s\n\n", ctime(&time_llnow), image_url);
			fflush( capture_fp );
	}

	strncpy( tmp_filename, "/tmp/gyach.XXXXXX", 22 );

	set_max_url_fetch_size(max_im_feed);
	url_length=fetch_url(image_url, imageresult, NULL);
	clear_fetched_url_data();
	if (url_length<1) {
		free(imageresult); imageresult=NULL;
		return( "" );
	}

	if ( strstr( imageresult, "Not Found" ))  {download_err="404 Not Found";}
	if ( strstr( imageresult, "Forbidden" ))  {download_err="403 Forbiddden";}
	if ( strstr( imageresult, "302 Move" )) {download_err="302 Moved";}
	if ( strstr( imageresult, "400 Bad Re" )) {download_err="400 Bad Request";}

	if (download_err) {
		if ( capture_fp ) {	
			time_t time_llnow = time(NULL);
			fprintf(capture_fp,"\n\n[%s]  DOWNLOAD IMAGE URL:\n   URL: %s\n   * ERROR *:   %s\n   Data:\n%s\n\n", ctime(&time_llnow), image_url, download_err, imageresult);
			fflush( capture_fp );
			}
		free(imageresult); imageresult=NULL;
		return( "" );
	}

	/* new way using mkstemp() */
	if (( ifp = mkstemp( tmp_filename )) == -1 ) {
		fprintf( stderr, "mkstemp( '%s' ) == -1\n", tmp_filename );
		fflush( stderr );
		free(imageresult); imageresult=NULL;
		return( "" );
	}

	write( ifp, imageresult, url_length);
	close( ifp );

		if ( capture_fp ) {	
			time_t time_llnow = time(NULL);
			fprintf(capture_fp,"\n\n[%s]  DOWNLOAD IMAGE URL: \n   URL: %s\n   Created Temporary File:\n%s\n\n", ctime(&time_llnow), image_url, tmp_filename);
			fflush( capture_fp );
			}

	free(imageresult); imageresult=NULL;
	return tmp_filename;
}




/* this function gets run as a separate thread so it does not hold up chat */
/* while downloading html and image and parsing/displaying */
/* NOTE: this function frees *arg when done, so use strdup() before calling */

void *fetch_profile( void *arg ) {
	char *user = (char *)arg;
	char *profresult=NULL;   /*  65536, 49152, 32768?? - profiles usually 10-32kb */
	int url_length=0;
	char tmp[96];
	char *ptr;
	char *ptr2;
	char pic_url[160] = "";
	char last_updated[24] = "";
	char email[64] = "";
	char realname[64] = "";
	char nickname[64] = "";  /* added: PhrozenSmoke */
	char location[72] = "";
	char age[8] = "";
	char married[32] = "";
	char sex[32] = "";
	char occupation[72] = "";
	char online_now[16] = "";
	char homepage[96] = "";
	char coollink1[96] = "";
	char coollink2[96] = "";
	char coollink3[96] = "";
	char chrdata[216]="";

	char BLACK1[24];
	char *hobbies=NULL;
	char *latest_news=NULL;
	char *fav_quote=NULL;

	char *PURP1=YAHOO_COLOR_PMBLUE;
	char *gyBreak="\n[~G1y_eBreak~]";
	char *FON="<font face=\"sans\" size=\"14\">";
	char *FOO="</font>";

	profresult=malloc(32768);
	if (! profresult) {
		free( user );
		return( NULL ); 
	}

	/* clear the stuff */
	memset( profresult, 0, sizeof( profresult ));
	profresult[0]='\0';
	sprintf(profresult,"%s", "");

	snprintf(BLACK1, 22, "%s    ", YAHOO_COLOR_BLACK);

	// DBG( 11, "fetch_profile( '%s' )\n", user );

	snprintf( homepage, 127, "http://profiles.yahoo.com/%s", user );
	set_max_url_fetch_size(32000);
	url_length=fetch_url(homepage, profresult, ymsg_sess->cookie );
	clear_fetched_url_data();  

	if (url_length<1) {
		free( user );
		free(profresult) ; profresult=NULL;
		return( NULL ); 
	}

	/* find the url to their pic */
	snprintf( tmp, 94,  "alt=\"%s\"", user );
	lower_str(tmp);

	ptr = strstr( profresult, tmp );
	if ( ptr ) {
		while((( *(ptr-4) != 's' ) || ( *(ptr-3) != 'r' ) ||
			   ( *(ptr-2) != 'c' ) || ( *(ptr-1) != '=' )) &&
			  ( ptr > (profresult+5))) {
			ptr--;
		}
		if ( *ptr == '"' )
			ptr++;
		/* now we are at the beginning of the url, so strncpy some */
		strncpy( pic_url, ptr, 158 );
		ptr = strchr( pic_url, '"' );
		ptr2 = strchr( pic_url, ' ' );
		if (( ptr && ptr2 ) &&
			( ptr2 < ptr ))
			ptr = ptr2;

		if ( ptr )
			*ptr = '\0';
		else
			pic_url[158] = '\0';
	}

	/* see what language this profile is in */ 
	select_profile_lang_strings(profresult);

		if ( capture_fp ) {	
			time_t time_llnow = time(NULL);
			fprintf(capture_fp,"\n\n[%s]  DOWNLOAD PROFILE: \n   URL: %s\n   Language:\n%s\n   Size: %d\n\n", ctime(&time_llnow), homepage , gprfl_sel_lang[GYPRFL_LANG], url_length );
			fflush( capture_fp );
			}

	/* added, PhrozenSmoke, check to see if its an adult profile we cant view */
	if (strstr(profresult, "Adult Profiles Warning")  || strstr(profresult,"Perfiles de adultos") || 
		strstr(profresult,"Adult Content Warning") || 
		strstr(profresult, "contains possible adult content") )  {
			if (! strstr(profresult, gprfl_sel_lang[GYPRFL_NAME])) {
				snprintf(profresult, 512,  "%s\n%s:\n'%s'\nhttp://profiles.yahoo.com/%s", _("Sorry, this is an adult profile."), _("You will need to view this profile with your web browser."), user, user);
				show_ok_dialog(profresult);
				free( user );
				free(profresult) ; profresult=NULL;
				return( NULL ); 
		  }
	   }

	/* Maybe the profile couldn't be retrieved at all */ 
	if (strstr(profresult, "403 Forbidden")  || strstr(profresult,"400 Bad Request") || 
		strstr(profresult,"Yahoo! Member Directory - User not found") || 
		strstr(profresult, "404 Not Found") || strstr(profresult,"400 Bad Request") )  {
			if (! strstr(profresult, gprfl_sel_lang[GYPRFL_NAME] )) {
				snprintf(profresult, 512,  "%s:\n'%s'\nhttp://profiles.yahoo.com/%s",  _("You will need to view this profile with your web browser."), user, user);
				show_ok_dialog(profresult);
				free( user );
				free(profresult) ; profresult=NULL;
				return( NULL ); 
			}
		}



	/* strip html to make the rest easier */
	/* First remove all newline chars */ 
	ptr=strchr(profresult,'\n');
	while (ptr) {
		*ptr=' ';
		ptr=strchr(profresult,'\n');
	}
	strip_html_tags( profresult );

	/*  printf("profresult:\n%s\n", profresult); fflush(stdout); */ 

	/* attempt to find some strings */
	find_field( profresult, gprfl_sel_lang[GYPRFL_SEARCH] , last_updated, 24, 0 );
	find_field( profresult, gprfl_sel_lang[GYPRFL_MAIL], email, 64, 1 );
	find_field( profresult, gprfl_sel_lang[GYPRFL_NAME], realname, 64 , 0 );
	find_field( profresult, "Nickname:" , nickname , 64, 0 );
	find_field( profresult, gprfl_sel_lang[GYPRFL_LOC] , location, 72 , 0 );
	find_field( profresult, gprfl_sel_lang[GYPRFL_AGE] , age, 7 , 0 );
	find_field( profresult,gprfl_sel_lang[GYPRFL_MAR], married, 32 , 0 );
	find_field( profresult, gprfl_sel_lang[GYPRFL_SEX], sex, 32 , 0 );
	find_field( profresult, gprfl_sel_lang[GYPRFL_JOB], occupation, 72 , 0 );

	find_field( profresult,gprfl_sel_lang[GYPRFL_HOB]  , chrdata, 216 , 0 );
	ptr=strstr(chrdata,gprfl_sel_lang[GYPRFL_NEWS] );
	if (ptr) {*ptr='\0';}
	hobbies=strdup(chrdata);
	find_field( profresult, gprfl_sel_lang[GYPRFL_NEWS] , chrdata, 192 , 0 );
	latest_news=strdup(chrdata);
	find_field( profresult, gprfl_sel_lang[GYPRFL_QUOTE] , chrdata, 192 , 1);
	fav_quote=strdup(chrdata);

	find_field( profresult, gprfl_sel_lang[GYPRFL_PAGE] , homepage, 96 , 0 );
	find_field( profresult, gprfl_sel_lang[GYPRFL_COOL1] , coollink1, 96, 0  );
	find_field( profresult, gprfl_sel_lang[GYPRFL_COOL2]  , coollink2, 96, 0  );
	find_field( profresult, gprfl_sel_lang[GYPRFL_COOL3] , coollink3, 96 , 0 );


	ptr = strstr( profresult, "I'm online now!" );
	if ( ptr ) {
		strncpy( online_now, "Online now!", 14 );
	} else {
		strncpy( online_now, "OFFline", 14 );
	}


	/* clear the stuff */
	memset( profresult, 0, sizeof( profresult ));
	profresult[0]='\0';


	snprintf(profresult, 10000 , 
		"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",

		"<font face=\"Arial\" size=\"16\">", YAHOO_COLOR_PMPURPLE, user, BLACK1,FOO, gyBreak, 		

		FON, PURP1, _("Yahoo! ID"), BLACK1,	user , FOO, gyBreak, 
		FON, PURP1, _("Real Name"), BLACK1,	realname , FOO, gyBreak, 
		FON, PURP1, _("Nickname"), BLACK1,	nickname , FOO, gyBreak, 
		FON, PURP1, _("Age"), BLACK1,	age , FOO, gyBreak, 
		FON, PURP1, _("Sex"), BLACK1,	sex , FOO, gyBreak, 
		FON, PURP1, _("Location"), BLACK1,	location , FOO, gyBreak, 
		FON, PURP1, _("Marital Status"), BLACK1,	married , FOO, gyBreak, 
		FON, PURP1, _("Occupation"), BLACK1,	occupation , FOO, gyBreak, 
		FON, PURP1, _("Online"), BLACK1,	online_now , FOO, gyBreak, 
		FON, PURP1, _("Email"), BLACK1,	email , FOO, gyBreak, 
		FON, PURP1, _("Last Updated"), BLACK1,	last_updated , FOO, gyBreak, 
		FON, PURP1, _("Hobbies"), BLACK1,	hobbies , FOO, gyBreak, 
		FON, PURP1, _("Latest News"), BLACK1,	latest_news , FOO, gyBreak, 
		FON, PURP1, _("Favorite Quote"), BLACK1,	fav_quote , FOO, gyBreak, 
		"\n", FON, PURP1, _("Home Page"), BLACK1,	homepage , FOO, gyBreak, 
		"\n", FON, PURP1, _("Cool Links"), BLACK1,	"  " , FOO, gyBreak, 
		FON, PURP1, "    ", BLACK1,	coollink1 , FOO, gyBreak, 
		FON, PURP1, "    ", BLACK1,	coollink2 , FOO, gyBreak, 
		FON, PURP1, "    ", BLACK1,	coollink3 , FOO, gyBreak
	);


	if (hobbies) {free(hobbies);}
	if (fav_quote) {free(fav_quote);}
	if (latest_news) {free(latest_news);}

	if ( pic_url[0] ) {
		ptr = download_image( pic_url );
		prof_pic = ptr;
	}

	if (prof_text) {g_free(prof_text); prof_text=NULL;}
	prof_text = g_strdup( _utf(profresult) );

	free(profresult) ; profresult=NULL;
	free( user );
	return( NULL );
}



int set_last_comment( char *user, char *comment ) {
	GtkTreeModel *model;
	gchar tmp[416];
	char tmp_user[64];
	gchar tmp_comment[408];
	char *ptr = NULL;
	int  count = 0;
	int myrow=-1;

	if ( ! last_comments ) {
		last_comments = g_hash_table_new( g_str_hash, g_str_equal );
	}

	if ( ! last_comments ) {
		return( 0 );
	}

	strncpy( tmp_user, user , 63);
	lower_str( tmp_user );
	strncpy( tmp_comment, _utf(comment), 406 );
	strip_html_tags( tmp_comment );

	if (( ptr = g_hash_table_lookup( last_comments, tmp_user )) != NULL ) {
		if ( ! strcmp( ptr + 4, tmp_comment )) {
			/* user said same thing again */
			count = atoi( ptr ) + 1;
		} else {
			/* user said something new */
			count = 1;
		}
		snprintf( tmp, 414, "%03d:%s", count, tmp_comment );
		/* don't pass in a strduped key since insert uses existing key ptr */
		g_hash_table_insert( last_comments, g_strdup(tmp_user), g_strdup(tmp));
		g_free( ptr );
	} else {
		/* user has no previous comment */
		count = 1;
		snprintf( tmp, 414, "%03d:%s", count, tmp_comment );
		/* pass in a strduped key since insert needs a key ptr itself */
		g_hash_table_insert( last_comments, g_strdup(tmp_user), g_strdup(tmp));
	}

	/* Added, PhrozenSmoke: actually update the list of chat members */
		model=find_chat_user_model();
		if (!model) {return 0;}

	myrow = find_user_row( tmp_user );
	if (myrow > -1) {
		if (!strcmp(tmp_comment,"Possible Flood Attack or Boot Attempt")) {
			snprintf(tmp_comment, 300, "%s", 
			"Sent Dangerous or Unwanted Message, File, Invitation, or Request (Communication was Blocked)");
			} 
	gtk_list_store_set(GTK_LIST_STORE(model), 
				&chat_user_iter, 
				GYQUAD_COL3, tmp_comment ,  -1);	
	}

	return( count );
}

char *get_last_comment( char *user ) {
	char tmp_user[64];
	char *ptr;

	if ( ! last_comments ) {return( NULL );	}

	strncpy( tmp_user, user, 63 );
	lower_str( tmp_user );

	if (( ptr = g_hash_table_lookup( last_comments, tmp_user )) != NULL ) {
		return( ptr + 4 );
	}
	return( NULL );
}

static void print_last_comment(gpointer key, gpointer value,
		gpointer user_data) {
	gchar tmp[640];

	snprintf( tmp, 638, "%-25s: %3d : ", (char *)key, atoi((char *)value ));
	ct_append_fixed( tmp, strlen( tmp ));

	snprintf( tmp, 635,  "%s", (gchar *)value + 4 );
	strcat(tmp,"\n");
	ct_append_fixed( tmp, strlen( tmp ));
}


void show_last_comments() {
	char tmp[128];

	strncpy( tmp,
		"-----------------------------------------------------------------\n", 127 );
	ct_append_fixed( tmp, strlen( tmp ));
	strncpy( tmp,
		"Last comment/action for each user (with number of times):\n", 127);
	ct_append_fixed( tmp, strlen( tmp ));

	if ( last_comments ) {
		g_hash_table_foreach( last_comments, print_last_comment, NULL );
	} else {
		strncpy( tmp, "No comments yet...\n" , 127);
		ct_append_fixed( tmp, strlen( tmp ));
	}

	strncpy( tmp,
		"-----------------------------------------------------------------\n", 127 );
	ct_append_fixed( tmp, strlen( tmp ));
}


GList *find_pm_session( char *user ) {
	GList *this_session = pm_list;
	PM_SESSION *pm_sess;

	// DBG( 11, "find_pm_session( '%s' )\n", user );

	while( this_session ) {
		pm_sess = (PM_SESSION *)this_session->data;
		if ( !strcasecmp( user, pm_sess->pm_user )) {
			return( this_session );
		}

		this_session = g_list_next( this_session );
	}

	return( NULL );
}

GList *find_pm_session_from_widget( GtkWidget *widget, char *name ) {
	GList *this_session = pm_list;
	PM_SESSION *pm_sess;
	GtkWidget *tmp_widget;

	// DBG( 11, "find_pm_session_from_widget( %p, '%s' )\n", widget, name );

	while( this_session ) {
		pm_sess = (PM_SESSION *)this_session->data;

		if ( pm_sess->pm_window == widget ) {
			return( this_session );
		}

		tmp_widget = lookup_widget( pm_sess->pm_window, name );
		if ( tmp_widget == widget ) {
			return( this_session );
		}

		this_session = g_list_next( this_session );
	}

	return( NULL );
}


