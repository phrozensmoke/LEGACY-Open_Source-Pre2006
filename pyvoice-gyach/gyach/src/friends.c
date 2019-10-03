/*****************************************************************************
 * friends.c
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

#include "gyach.h"
#include "friends.h"
#include "main.h"
#include "users.h"
#include "util.h"
#include "yahoochat.h"
#include "profname.h"
#include "gytreeview.h"
#include "ignore.h"
#include "webconnect.h"
#include "ycht.h"

	struct yahoo_friend *create_or_find_yahoo_friend(char *bud);
	struct yahoo_friend *yahoo_friend_find(char *bud);

char frndbuf[384];

GList *friend_list = NULL;
GList *ofriend_list = NULL;
GList *temporary_friend_list = NULL;
GHashTable *buddy_groups =NULL; 
GList *buddy_group_list=NULL;
GHashTable *buddy_status =NULL; 
char *friend_to_add=NULL;
char *oldgrpname=NULL;
char *newgrpname=NULL;
char *buddy_identities_list=NULL;
int buddy_identities_search=0;

extern int parsing_initial_statuses;
extern void handle_yahoo_packet();
extern char *selected_buddy_group;
extern char *selected_buddy;
extern void update_buddy_clist();

extern int remove_flooder( char *flooder );

extern GtkWidget *create_list_dialog(char *wintitle, char *header, char *ldefault, GList *comblist, int editable, int icon);
int set_buddy_status_full( char *user, char *userstatus, int allow_invisible );
GList *get_buddy_group_list();
int remove_online_friend_update=1;
char *TMP_FRIEND_GRP="~[Temporary Friends]~";


void append_new_friend_to_list(char *friend_to_add) {
	GtkTreeModel *model;
		model=gtk_tree_view_get_model(chat_users);
		if (!model) {model=chat_user_model;}
		if (!model) {return;}

	if (!friend_to_add) {return;}
	friend_list = g_list_append( friend_list, strdup( friend_to_add ));

	if (friend_list) {		
		int row;
		row = find_user_row( friend_to_add );
		if (row >= 0) {
			GdkColor online_color;
			online_color.red   = 0xA6 * ( 65535 / 255 );
			online_color.green = 0x4E * ( 65535 / 255 );
			online_color.blue  = 0xD2 * ( 65535 / 255 );
		gtk_list_store_set(GTK_LIST_STORE(model), 
				&chat_user_iter, GYQUAD_COLOR, &online_color , -1);
							}
					}
}

void remove_old_friend_from_list(char *friend_to_rem) {
	GList *this_friend;
	GtkTreeModel *model;
	int row;
		model=gtk_tree_view_get_model(chat_users);
		if (!model) {model=chat_user_model;}
		if (!model) {return;}

	this_friend = friend_list;
	while( this_friend ) {
		if ( ! strcasecmp( this_friend->data, friend_to_rem )) {
			friend_list = g_list_remove_link( friend_list, this_friend );
			gyach_g_list_free( this_friend );
			break;
			}
		this_friend = g_list_next( this_friend );
	}	
		row = find_user_row( friend_to_rem );
		if (row >= 0) {
			GdkColor online_color;
			online_color.red   = 0x00 * ( 65535 / 255 );
			online_color.green = 0x00 * ( 65535 / 255 );
			online_color.blue  = 0x00 * ( 65535 / 255 );
		gtk_list_store_set(GTK_LIST_STORE(model), 
				&chat_user_iter, GYQUAD_COLOR, &online_color , -1);
							}
}


void populate_friend_list( char *friends ) {
	char *ptr;
	char *ptr2;
	char *end;
	char *end2;
	char buddy_group[64];
	char *budptr=NULL;
	struct yahoo_friend *f = NULL;
	int last = 0;
	int last2 = 0;


	if (( strchr( friends, ':' )) &&
		( friend_list )) {
		friend_list = gyach_g_list_free( friend_list );
	}

	/* printf("\nFriends:  %s\n", friends);
	fflush(stdout); */ 

	/* search groups */
	snprintf(buddy_group, 20, "%s","Buddies");

	ptr = strchr( friends, ':' );
		/* grab buddy group */
	snprintf(buddy_group,62, "%s",friends);
	budptr=strchr(buddy_group,':');
	if (budptr) {*budptr='\0';}

	do {

		if ( ptr ) {
			end = strchr( ptr + 1, '\xa' );
			ptr++;
		} else {
			end = strchr( friends, '\xa' );
			ptr = friends;
		}
		if ( end ) {
			*end = '\0';
		} else {
			last = 1;
		}

		/* now search for people within group */
		last2 = 0;
		ptr2 = ptr;
		while( ptr2 ) {
			end2 = strchr( ptr2, ',' );
			if ( end2 ) {
				*end2 = '\0';
			} else {
				last2 = 1;
			}


	 if (strlen(ptr2)>1) {
			if (! find_friend(ptr2)) {
				friend_list = g_list_append( friend_list, strdup( ptr2 ));
			}
			set_buddy_status_full( ptr2 ,"",0 );  /* added, PhrozenSmoke */
			f =create_or_find_yahoo_friend(ptr2);
				/* printf("buddy group:  %s [%s]\n", ptr2, buddy_group); fflush(stdout); */ 
			if (f)  {
				if ( (strcmp(buddy_group,"Y! Mobile Messenger")) && (f->buddy_group != NULL) ) {
					if (f->buddy_group) {g_free(f->buddy_group);}
					f->buddy_group=g_strdup(buddy_group);
				} 
				if (! strcmp(buddy_group,"Y! Mobile Messenger")) {f->mobile_list=1;}
			}
	}

			if ( last2 ) {
				ptr2 = NULL;
			} else {
				ptr2 = end2 + 1;
			}
		}

		if ( last ) {
			ptr = NULL;
		} else {
			ptr = strchr( end + 1, ':' );
			if (ptr) {
				snprintf(buddy_group,62, "%s",end + 1);
				budptr=strchr(buddy_group,':');
				if (budptr) {*budptr='\0';}
				}
		}
	} while( ptr );

	update_buddy_clist();
	return;
}


int check_buddy_remove_timeout(char *myname) {
	if (myname) {
		if ( (! using_web_login) && (! ycht_is_running()) ) {
			if (find_friend(myname) ) {
				snprintf(frndbuf, 382, "'%s' %s %s %s", myname,
				_("could not be removed from your buddy list."),
				_("Gyach-E received no response from Yahoo's servers."),
				_("Yahoo's servers may be down or malfunctioning. Please try again later.")
				);
				show_ok_dialog(frndbuf);
			}
		}		
		free(myname);
	}
	return 0;
}
void start_check_buddy_remove_timeout(char *myname) {gtk_timeout_add( 8000,
				(void *)check_buddy_remove_timeout, myname );}


int check_buddy_add_timeout(char *myname) {
	if (myname) {
		if ( (! using_web_login) && (! ycht_is_running()) ) {
			if (! find_friend(myname) ) {
				snprintf(frndbuf, 382, "'%s' %s %s %s", myname,
				_("could not be added to your buddy list."),
				_("Gyach-E received no response from Yahoo's servers."),
				_("Yahoo's servers may be down or malfunctioning. Please try again later.")
				);
				show_ok_dialog(frndbuf);
			}
		}		
		free(myname);
	}
	return 0;
}
void start_check_buddy_add_timeout(char *myname) {gtk_timeout_add( 8000,
				(void *)check_buddy_add_timeout, myname );}



int remove_friend( char *friend ) {
	struct yahoo_friend *f = NULL;
	if ( ! buddy_status ) { return 0; }	
	if (selected_buddy_group) {
		free(selected_buddy_group);
		selected_buddy_group=NULL;
	}
	if ( find_friend( friend )) {
		f=yahoo_friend_find(friend);
		if (f)  {selected_buddy_group=strdup(f->buddy_group);	}
		start_check_buddy_remove_timeout(strdup(friend));
		ymsg_remove_buddy( ymsg_sess, friend );
	}

	/* PhrozenSmoke: Dont do removal here til we get confirmation */
	build_tab_complete_list();
	return( 0 );
}



void add_friend_cb(GtkButton *button, gpointer user_data) {
	GtkWidget *tuxwin;
	GtkWidget *tentry;

	tuxwin=gtk_object_get_data(GTK_OBJECT(button),"window");
	if (selected_buddy_group) {
		free(selected_buddy_group);
		selected_buddy_group=NULL;
	}
	if (! friend_to_add) {if (tuxwin) {gtk_widget_destroy(tuxwin);}   return;}
	tentry=gtk_object_get_data(GTK_OBJECT(button),"entry");
	selected_buddy_group=strdup(gtk_entry_get_text(GTK_ENTRY(tentry)));

	if ((strcmp(selected_buddy_group, TMP_FRIEND_GRP)==0) || 
	   (strlen(selected_buddy_group) <1) ) {
		free(selected_buddy_group); 
		selected_buddy_group=NULL;
		return;
	}
	if (tuxwin) {gtk_widget_destroy(tuxwin);}

	start_check_buddy_add_timeout(strdup(friend_to_add));
	ymsg_add_buddy( ymsg_sess, friend_to_add );

	/* PhrozenSmoke: Dont do this here til we get confirmation */
	/* 
	friend_list = g_list_append( friend_list, strdup( friend_to_add ));
	 */ 
	build_tab_complete_list();

	free(friend_to_add);
	friend_to_add=NULL;
}

int add_friend( char *friend ) {
	GtkWidget *okbutton;
	if ( using_web_login || ycht_is_running() ) {
			show_ok_dialog(_("This feature is not available when connected using the WEBCONNECT or YCHT servers."));
			return ( 0 );
	}
	if (find_friend( friend )) {return 0;}
	if (friend_to_add) {free(friend_to_add);}
	friend_to_add=strdup(friend);
	okbutton=create_list_dialog(_("Group"), _("Group:"), selected_buddy_group, get_buddy_group_list(), TRUE, 6);
  gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC ( add_friend_cb),
                      NULL);
	return( 0 );
}

int find_friend( char *friend ) {
	GList *this_friend;

	this_friend = friend_list;
	while( this_friend ) {
		if ( ! strcasecmp( this_friend->data, friend )) {
			return( 1 );
		}
		this_friend = g_list_next( this_friend );
	}

	return( 0 );
}

int remove_online_friend( char *friend ) {
	GList *this_friend;

	this_friend = ofriend_list;
	while( this_friend ) {
		if ( ! strcasecmp( this_friend->data, friend )) {
			ofriend_list = g_list_remove_link( ofriend_list, this_friend );

			set_buddy_status_full( friend ,"",0 );  /* added, PhrozenSmoke */

			gyach_g_list_free( this_friend );

			if (remove_online_friend_update) {update_buddy_clist();}  /* added: PhrozenSmoke */
			return( 0 );
		}
		this_friend = g_list_next( this_friend );
	}

	return( 0 );
}

int add_online_friend( char *friend ) {
	if ( ! find_online_friend( friend )) {
		ofriend_list = g_list_append( ofriend_list, strdup( friend ));
	}
	return( 0 );
}

int find_online_friend( char *friend ) {
	GList *this_friend;

	this_friend = ofriend_list;
	while( this_friend ) {
		if ( ! strcasecmp( this_friend->data, friend )) {
			return( 1 );
		}
		this_friend = g_list_next( this_friend );
	}

	return( 0 );
}

int show_friends( void ) {
	GList *this_friend;
	char msg[6144];
	int any_online = 0;


	snprintf( msg, 192, "%s%s%s:%s%s ", YAHOO_COLOR_BLUE,
		YAHOO_STYLE_BOLDON, _("Current Buddy List"), 
		YAHOO_COLOR_BLACK, YAHOO_STYLE_BOLDOFF );

	this_friend = friend_list;
	while( this_friend ) {
		if ( this_friend != friend_list )
			strcat( msg, ", " );

		if ( find_online_friend( this_friend->data )) {
			strcat( msg, YAHOO_COLOR_RED );
			strncat( msg, this_friend->data, 64 );
			strcat( msg, YAHOO_COLOR_BLACK );
			any_online = 1;
		} else {
			strncat( msg, this_friend->data, 64 );
		}

		if (strlen(msg)>5750) {
			append_to_textbox_color( chat_window, NULL, msg);
			sprintf(msg,"%s","");
			} /* avoid buffer overflow */

		this_friend = g_list_next( this_friend );
	}

	if ( ! friend_list ) {
		strncat( msg, _("No friends on your list.") , 128);
	} else {
		if ( any_online ) {
			strcat( msg, "   (");
				strncat(msg,_("friends online in"), 40);
				strcat(msg," " );
				strcat( msg, YAHOO_COLOR_RED );
				strncat( msg, _("RED"), 15 );
				strcat( msg, YAHOO_COLOR_BLACK );
				strcat( msg, ")" );
		}
	}

	strcat( msg, "\n\n" );
	append_to_textbox_color( chat_window, NULL, msg );

	if (temporary_friend_list) {
		this_friend = temporary_friend_list;
		snprintf( msg, 192, "\n%s%s%s:%s%s ", YAHOO_COLOR_PURPLE,
		YAHOO_STYLE_BOLDON, _("Temporary Friends"), 
		YAHOO_COLOR_BLACK, YAHOO_STYLE_BOLDOFF );
		while( this_friend ) 		{
			if ( this_friend != temporary_friend_list ) {strcat( msg, ", " ); }
			strncat( msg, this_friend->data, 64 );
			this_friend = g_list_next( this_friend );

		if (strlen(msg)>5750) {
			append_to_textbox_color( chat_window, NULL, msg);
			sprintf(msg,"%s","");
			} /* avoid buffer overflow */
											}
		strcat( msg, "\n\n" );
		append_to_textbox_color( chat_window, NULL, msg );
									}

	return( 0 );
}

int build_online_friends_list( void ) {
	GList *tmp_list = NULL;
	char *ptr;
	char *end;

	if ( ofriend_list ) {
		while( ofriend_list ) {
			tmp_list = ofriend_list;
			ofriend_list = g_list_remove_link( ofriend_list, ofriend_list );
			gyach_g_list_free( tmp_list );
		}
	}

	ymsg_sess->pkt.data[ymsg_sess->pkt.size] = '\0';
	ptr = ymsg_sess->pkt.data;

	/* find first field separator */
	while(( ptr < ( ymsg_sess->pkt.data + ymsg_sess->pkt.size )) &&
		( *ptr != '\0' ) &&
		( *ptr != '\300' )) {
		ptr++;
	}
	ptr += 2;

	/* now find the names */
	while( ptr < ( ymsg_sess->pkt.data + ymsg_sess->pkt.size )) {
		end = ptr;
		while( end < ( ymsg_sess->pkt.data + ymsg_sess->pkt.size )) {
			if (( *end == '\300' ) ||
				( *end == '\0' )) {
				*end = '\0';
				break;
			}
			end++;
		}

		ofriend_list = g_list_append( ofriend_list, strdup( ptr ));

		end += 2;
		while( end < ( ymsg_sess->pkt.data + ymsg_sess->pkt.size )) {
			if (( *end == '\300' ) ||
				( *end == '\0' )) {
				*end = '\0';
				break;
			}
			end++;
		}
		ptr = end + 2;
	}

	return( 0 );
}


/* added: PhrozenSmoke - for buddy list */

void yahoo_friend_free(gpointer p)
{
	struct yahoo_friend *f = p;
	char *ss=NULL;
	if (!f) {return;}
	//g_warning("yahoo_friend_free-1");
	if (f->main_stat) {	g_free(f->main_stat); f->main_stat=ss;}
	if (f->game_stat) {	g_free(f->game_stat); f->game_stat=ss;}
	if (f->idle_stat) {	g_free(f->idle_stat); f->idle_stat=ss; }
	if (f->buddy_group) {g_free(f->buddy_group); f->buddy_group=ss; }
	if (f->avatar) {g_free(f->avatar); f->avatar=ss; }
	if (f->game_url) {g_free(f->game_url); f->game_url=ss; }
	if (f->radio_stat) {g_free(f->radio_stat); f->radio_stat=ss; }
	if (f->buddy_image_hash) {g_free(f->buddy_image_hash); f->buddy_image_hash=ss; }
	//g_warning("yahoo_friend_free-2");

	/* don't re-enable, causes segfault */
	//g_free(f);
	//g_warning("yahoo_friend_free-3");
}


struct yahoo_friend *yahoo_friend_new()
{
	struct yahoo_friend *ret;

	ret = g_new0(struct yahoo_friend, 1);
	ret->away = 0;
	ret->idle = 0;
	ret->inchat = 0;
	ret->insms = 0;
	ret->ingames = 0;
	ret->webcam = 0;
	ret->stealth = 0;
	ret->launchcast = 0;
	ret->identity_id=-1;
	ret->mobile_list=0;
	ret->main_stat=g_strdup("");
	ret->game_stat=g_strdup("");
	ret->idle_stat=g_strdup("");
	ret->game_url=g_strdup("");
	ret->radio_stat=g_strdup("");
	ret->buddy_group=g_strdup("Friends");
	ret->avatar=NULL;
	ret->buddy_image_hash=NULL;
	return ret;
}

struct yahoo_friend *yahoo_friend_find(char *bud)
{
	struct yahoo_friend *f = NULL;
	char tmp_user[72];
	if (! buddy_status) {return f;}
	strncpy( tmp_user, bud, 70 );
	lower_str(tmp_user);
	f = g_hash_table_lookup(buddy_status, tmp_user);
	return f;
}



void clear_friend_object_hash() {
	if (buddy_status) {g_hash_table_destroy(buddy_status);}
	buddy_status= g_hash_table_new(g_str_hash, g_str_equal);
}


struct yahoo_friend *create_or_find_yahoo_friend(char *bud)
{
	struct yahoo_friend *f = NULL;
	char tmp_user[72];
	if ( ! buddy_status ) {	clear_friend_object_hash();	}
	strncpy( tmp_user, bud, 70 );
	lower_str(tmp_user);
	f=yahoo_friend_find(tmp_user);
	if (!f) {
		f = yahoo_friend_new();
		if (buddy_status) {g_hash_table_insert(buddy_status, g_strdup(tmp_user), f); }
		}
	return f;
}


int set_buddy_status_full( char *user, char *userstatus, int allow_invisible ) {
	char tmp_user[64];
	char tmp_stat[256];
	struct yahoo_friend *f = NULL;

	if ( ! buddy_status ) {
	clear_friend_object_hash();
	}

	if ( ! buddy_status ) {
		return( 0 );
	}

	strncpy( tmp_user, user, 62 );
	lower_str(tmp_user);
	if ( (allow_invisible==3) || find_online_friend( user )) {
			strncpy( tmp_stat, userstatus, 254 );
										 } else {  /* user is invisible or not online */
			if (allow_invisible) {
				snprintf(tmp_stat,254, " (%s) - %s",_("invisible") ,userstatus);
									 } else  {
				strncpy( tmp_stat, userstatus , 254 );
												}
												   }


	f=create_or_find_yahoo_friend(tmp_user);
	if (!f) {return (1);}
	if (f->main_stat) {	g_free(f->main_stat); }
	f->main_stat=g_strdup(_utf(tmp_stat));

	if (allow_invisible==3) {
		if (f->buddy_group) {g_free(f->buddy_group);}
		f->buddy_group=g_strdup(TMP_FRIEND_GRP);
	}

	return( 1 );
}

int set_buddy_status( char *user, char *userstatus ) {
  return set_buddy_status_full(user,userstatus,1);
}


void remove_buddy_status(char *user)  {
	char tmp_user[72];
	gpointer key_ptr;
	gpointer value_ptr;
	struct yahoo_friend *f = NULL;
	if ( ! buddy_status ) { return; }
	strncpy( tmp_user, user, 70 );
	lower_str( tmp_user );

	f=yahoo_friend_find(user);
	if (f)  {
		char *ss=NULL;
		if (f->main_stat) {	g_free(f->main_stat); f->main_stat=ss;}
		if (f->game_stat) {	g_free(f->game_stat); f->game_stat=ss;}
		if (f->idle_stat) {	g_free(f->idle_stat); f->idle_stat=ss; }
		if (f->buddy_group) {	g_free(f->buddy_group); f->buddy_group=ss; }
		if (f->avatar) {	g_free(f->avatar); f->avatar=ss; }
		if (f->game_url) {	g_free(f->game_url); f->game_url=ss; }
		if (f->radio_stat) {	g_free(f->radio_stat); f->radio_stat=ss; }
		if (f->buddy_image_hash) {g_free(f->buddy_image_hash); f->buddy_image_hash=ss; }

		if ( g_hash_table_lookup_extended( buddy_status, tmp_user, &key_ptr,
			&value_ptr )) 				{
				g_hash_table_remove( buddy_status, tmp_user );
			if ( key_ptr )
				g_free( key_ptr ); // seg-fault watch ? 
			if ( value_ptr )
				g_free( value_ptr );  // seg-fault watch ?  
											  }

		 }

 }


/* Using the same basic structure as the SMILEY struct 
	because they are virtually the same - PhrozenSmoke */

SMILEY launchcast_genres[] = { 
{"10 Years Ago: Hits of '94","516"},
{"1950's Rock","524"},
{"1960's Garage Rock","682"},
{"1970's Country","692"},
{"1970's Rock","393"},
{"1970's Soft Pop","704"},
{"1970's Soft Rock","691"},
{"1970's Soul","681"},
{"1980's Alternative","313"},
{"1980's Country","370"},
{"1980's Cover Songs","854"},
{"1980's Dance Party","552"},
{"1980's Hip-Hop","874"},
{"1980's Indie Rock","897"},
{"1980's Pop","368"},
{"1980's R&B","733"},
{"1980's Rock","657"},
{"1980's Soft Pop","540"},
{"1980's Soft Rock","694"},
{"1990's Alternative","451"},
{"1990's Cont. Christian","641"},
{"1990's Country","400"},
{"1990's Hip-Hop","402"},
{"1990's Pop","718"},
{"1990's R&B","403"},
{"1990's Rock","394"},
{"1990's Soft Pop","545"},
{"1990's Soft Rock","693"},
{"20 Years Ago: Hits of '84","643"},
{"2004 Country Hits","919"},
{"2004 Grammy Nominees","942"},
{"2004 LAUNCH Picks","905"},
{"2004 Pop Hits","902"},
{"2004 Rock Hits","918"},
{"2004 Urban Hits","916"},
{"25 Years Ago: Hits of '79","644"},
{"30 Years Ago: Hits of '74","633"},
{"40 Years Ago: Hits of '64","632"},
{"Adult Alt. New Releases","464"},
{"Adult Alternative","89"},
{"Aerobics","668"},
{"Affectionate","450"},
{"Afterhours Party","667"},
{"Almost Acoustic","562"},
{"Alternative Rock","302"},
{"Americana","414"},
{"Angry","734"},
{"Beatles Cover Songs","345"},
{"Big Hits Of The 60's","392"},
{"Big Hits Of The 70's","385"},
{"Big Hits Of The 80's","386"},
{"Big Hits Of The 90's","387"},
{"Blues","92"},
{"Booty Time","599"},
{"Brazilian","680"},
{"Cajun Dinner Music","635"},
{"Candlelight Dinner","666"},
{"Chill Out","467"},
{"Christian Rock","530"},
{"Classic Broadway","640"},
{"Classic Country","519"},
{"Classic R&B","86"},
{"Classic Rock","308"},
{"Classic Vocal Standards","590"},
{"Classical","465"},
{"Comedy Christmas","637"},
{"Contemporary Christian","407"},
{"Cool As Folk","614"},
{"Country","81"},
{"Country Holiday","620"},
{"Dance","58"},
{"Dirty South","539"},
{"Disco","482"},
{"Doo Wop","525"},
{"Easy Jazz","406"},
{"Easy Like Sunday Morning","663"},
{"Electro","476"},
{"Elvis Cover Songs","444"},
{"Emo","567"},
{"Experimental Music","566"},
{"Folk","489"},
{"Funk","520"},
{"Gangsta Rap","862"},
{"Garage Rock","462"},
{"Glitter Rock","565"},
{"Gospel","529"},
{"Gospel Holiday","621"},
{"Gothic Rock","686"},
{"Greatest Country Hits","600"},
{"Greatest Pop Ballads","598"},
{"Greatest Pop Hits","597"},
{"Greatest Urban Hits","601"},
{"Grunge Rock","690"},
{"Hair Flare","488"},
{"Happy","381"},
{"Happy Hour","651"},
{"Hard Rock","946"},
{"Hip-Hop","59"},
{"Hip-Hop-Soul","880"},
{"House","459"},
{"Indie Pop","479"},
{"Indie Rock","507"},
{"Industrial Rock","698"},
{"Island Vacation","670"},
{"Italian Dinner Music","634"},
{"Jam Bands","573"},
{"Jazz Holiday","617"},
{"Jungle and Drum n Bass","602"},
{"Karaoke Faves","660"},
{"Kidz Country","678"},
{"Kidz Holiday","626"},
{"Kidz Latin","685"},
{"Kidz Pop","546"},
{"Kidz Rock","625"},
{"Latin Holiday","636"},
{"Lil' Kidz Tunes","551"},
{"Lite Office Music","743"},
{"Love Songs","559"},
{"Lullaby Tunes","568"},
{"M-Azing Radio","890"},
{"Makeout Music","630"},
{"Martini Lounge","355"},
{"Metal","73"},
{"Modern Broadway","642"},
{"Naughty Comedy","282"},
{"Neo Soul","575"},
{"New Age","512"},
{"New School Punk Rock","572"},
{"New Vocal Standards","699"},
{"New York, New York","913"},
{"Old School Punk Rock","85"},
{"Oldies","505"},
{"On The Road","523"},
{"Political Punk","528"},
{"Pop Holiday","608"},
{"Pop Latino","554"},
{"Power Ballads","631"},
{"Powerpop","463"},
{"Praise and Worship","604"},
{"Progressive Rock","705"},
{"Pure Energy","508"},
{"Pure Jazz","84"},
{"Quiet Storm","477"},
{"R&B","77"},
{"R&B Party","483"},
{"Rainy Day","473"},
{"Real Rock","320"},
{"Red Hot Now","725"},
{"Reggae","511"},
{"Reggae Dancehall","514"},
{"Reggae Dub Zone","513"},
{"Rock En Español","557"},
{"Rock Holiday","605"},
{"Rocktronica","677"},
{"Salsa Cien Por Ciento","555"},
{"Scrooge Tunes","639"},
{"Show Tunes","397"},
{"Ska","409"},
{"Slowcore","527"},
{"Soft Country","688"},
{"Soft Pop","468"},
{"Soft Rock","487"},
{"Soul Food","669"},
{"Stand Up Comedy","653"},
{"Summertime Sounds","541"},
{"T.G.I.F.!","570"},
{"Teen Idols","906"},
{"Tejano","556"},
{"The Big Band Era","673"},
{"The Big Hits","341"},
{"The Big Jams","571"},
{"The Golden State","914"},
{"Today's Pop","301"},
{"Traditional Christmas","616"},
{"Underground Hip-Hop","352"},
{"Urban Holiday","606"},
{"Vintage Country","484"},
{"Who's Next Radio","891"},
{"World Beat","558"},
{"Yoga And Meditation","665"},

 {"Unknown Genre","0"},
 { 0, 0 }
};

char *get_launchast_genre(char *incoming) {
	SMILEY *sm_ptr;
	char *retv=NULL;
	sm_ptr = launchcast_genres;
	 /*  printf("launchcast incoming: %s\n", incoming); fflush(stdout); */
	while( sm_ptr->sm_file ) {
		if (!strcmp(sm_ptr->sm_file, incoming)) {
			if (sm_ptr->sm_text) {
				retv=strdup(sm_ptr->sm_text);
				break;
			}
		}
		sm_ptr++;
	}
	if (!retv) {
		if (strlen(incoming)>4) {retv=strdup("Custom Station");}
		else {retv=strdup("Unknown Genre");}
	}
	return retv;
}

void parse_initial_statuses( char *packet )  {
	char	*starter=NULL;
	char	*ender=NULL;
	char *cleaner =NULL;
	int parse_last=0;
	int first_parse=1;
		/* Yahoo packet attributes separator, field '7' */
		/* char 0x37='7' */
	char *connector="\xC0\x80\x37\xC0\x80";  
	static char tmp_str[2144]="";	 /* ymsg_sess->pkt.data size is 2049 */ 
	static char stat_part[512];
	char *stat_dup=NULL;
	int got_status=0;
	got_status=snprintf(tmp_str, 2142, "%s", packet);
	tmp_str[got_status]='\0';

			/* replace fields 10=7 */
			cleaner=strstr(tmp_str,"\xC0\x80\x31\x30\xC0\x80\x37");
			while (cleaner) {
				*(cleaner+6)=']';
				cleaner=strstr(stat_part,"\xC0\x80\x31\x30\xC0\x80\x37");
			}
			/* replace fields 13=7 */
			cleaner=strstr(tmp_str,"\xC0\x80\x31\x33\xC0\x80\x37");
			while (cleaner) {
				*(cleaner+6)=']';
				cleaner=strstr(stat_part,"\xC0\x80\x31\x33\xC0\x80\x37");
			}
			/* replace fields 8=7 */
			cleaner=strstr(tmp_str,"\x38\xC0\x80\x37\xC0\x80");
			while (cleaner) {
				*(cleaner+3)=']';
				cleaner=strstr(stat_part,"\x38\xC0\x80\x37\xC0\x80");
			}

	starter=strstr(tmp_str,connector);
	if (!starter) {return;}
	while (starter) {
			if (first_parse) {starter += 5;}
			else { starter += 4;} 
			first_parse=0;
			ender=strstr(starter,connector);

			if (ender) {	*ender='\0';   }
			else {parse_last=1;}

			if (stat_dup) {free(stat_dup); stat_dup=NULL;}
			snprintf(stat_part, 512, "\x37\xC0\x80%s\xC0\x80", starter);

			/* replace fields 10=7 */
			cleaner=strstr(stat_part,"\xC0\x80\x31\x30\xC0\x80]");
			while (cleaner) {
				*(cleaner+6)='7';
				cleaner=strstr(stat_part,"\xC0\x80\x31\x30\xC0\x80]");
			}
			/* replace fields 13=7 */
			cleaner=strstr(stat_part,"\xC0\x80\x31\x33\xC0\x80]");
			while (cleaner) {
				*(cleaner+6)='7';
				cleaner=strstr(stat_part,"\xC0\x80\x31\x33\xC0\x80]");
			}
			/* replace fields 8=7 */
			cleaner=strstr(stat_part,"\x38\xC0\x80]\xC0\x80");
			while (cleaner) {
				*(cleaner+3)='7';
				cleaner=strstr(stat_part,"\x38\xC0\x80]\xC0\x80");
			}

			stat_dup=strdup(stat_part);
			ymsg_sess->pkt.size=strlen(stat_dup);
			ymsg_sess->pkt.type=0xc6;
			memcpy( ymsg_sess->pkt.data, stat_dup, ymsg_sess->pkt.size );
			ymsg_sess->pkt.data[ ymsg_sess->pkt.size ] = '\0';

			/* printf("packet:  %s\n", ymsg_sess->pkt.data); fflush(stdout); */ 
			parsing_initial_statuses=1;
			handle_yahoo_packet();
			parsing_initial_statuses=0;

			if (!parse_last) {ender++; starter=ender;}
			else {starter=NULL;} 
	}
	if (stat_dup) {free(stat_dup); stat_dup=NULL;}
	parsing_initial_statuses=0;
}



int clear_friend_hash_cb(gpointer key, gpointer value,
        gpointer user_data) {
		struct yahoo_friend *f = value;
		if (f) {
			if (f->buddy_group) {
						/* dont clear temporary friends */ 
				if (strcmp(f->buddy_group, TMP_FRIEND_GRP)==0) {return 0;}
			}
		}

		// printf("free-1\n"); fflush(stdout);
	g_free(key);
		// printf("free-2\n"); fflush(stdout);
	yahoo_friend_free(value);
		// printf("free-3\n"); fflush(stdout);
	g_free(value);   /* segfault watch */
		// printf("free-4\n\n"); fflush(stdout);
 	return 1;

} 

void remove_all_online_friends( ) {
	GList *this_friend;
	this_friend = friend_list;
	remove_online_friend_update=0;
	while( this_friend ) {		
		remove_online_friend(this_friend->data);
		this_friend = g_list_next( this_friend );
							}

	if ( ! buddy_status ) { return; }
	g_hash_table_foreach_remove(buddy_status,clear_friend_hash_cb,NULL);
	/* dont clear temporary friends */ 
	if (g_hash_table_size( buddy_status )<1) { clear_friend_object_hash();}
	update_buddy_clist();
	remove_online_friend_update=1;
}



void reset_all_online_friends_statuses( ) {
	GList *this_friend;
	char *friend_name=NULL;
	this_friend = friend_list;
	remove_online_friend_update=0;
	while( this_friend ) {		
		struct yahoo_friend *f = NULL;
		friend_name=strdup(this_friend->data);
		f =yahoo_friend_find(friend_name);
		if (f) {
			f->ingames=0;
			f->insms=0;
			f->inchat=0;
			f->idle=0;
			f->away=0;
			f->launchcast=0;
			f->webcam=0;
			if (f->main_stat) {g_free(f->main_stat); f->main_stat=g_strdup("");}
			if (f->idle_stat) {g_free(f->idle_stat); f->idle_stat=g_strdup("");}
			/* if (f->game_stat) {g_free(f->game_stat); f->game_stat=g_strdup("");} */ 
			if (f->avatar) {g_free(f->avatar); f->avatar=NULL;}
			if (f->radio_stat) {g_free(f->radio_stat); f->radio_stat=g_strdup("");}
			/* if (f->game_url) {g_free(f->game_url); f->game_url=g_strdup("");} */ 
				}
		free(friend_name);
		remove_online_friend(this_friend->data);
		this_friend = g_list_next( this_friend );
							}

	remove_online_friend_update=1;
}




/* Support for 'temporary' friends:  CONCEPT:
   Sometimes you want to let someone PM you, send a file, etc. - but you 
   have your privacy permissions set up to only allow people on your friend's 
   list to send you PMs, etc. BUT, you don't really want to add this particular 
   person to your friend's list and you don't want to open your privacy 
   permissions up to the whole chat room or all of yahoo. So, you can 
   temporarily allow a specific person all the privileges of people on your 
   friends list WITHOUT having to add them to your real buddy list.  They 
   will not appear on your buddy list, and their buddy privileges end after 
   you close GYach Enhanced or remove their 'temporary' friend status.
*/

int remove_temporary_friend( char *friend ) {
	int update_it=0;
	GList *this_friend;
	GtkTreeModel *model;
		model=gtk_tree_view_get_model(chat_users);
		if (!model) {model=chat_user_model;}
		if (!model) {return 0;}

	this_friend = temporary_friend_list;
	while( this_friend ) {
		if ( ! strcasecmp( this_friend->data, friend )) {
			int row;
			struct yahoo_friend *f = NULL;
			char msg[192]="";

		f=yahoo_friend_find(friend);
		if (f) {
			if (f->buddy_group) {
						/* clear only if their object is still 'temporary'  */ 
				if (strcmp(f->buddy_group, TMP_FRIEND_GRP)==0) {
					remove_buddy_status(friend);
					update_it=1;
				}
			}
		}
			
			temporary_friend_list = g_list_remove_link( temporary_friend_list, this_friend );
			gyach_g_list_free( this_friend );

			snprintf( msg, 190, "%s  ** '%s'  %s **%s\n", "\033[#FFB8AFm", friend, 
			_("has been removed from your TEMPORARY buddy list"),  YAHOO_COLOR_BLACK );
			append_to_textbox( chat_window, NULL, msg );
	
		row = find_user_row( friend );
		if (row >= 0) {
			GdkColor online_color;
			online_color.red   = 0x00 * ( 65535 / 255 );
			online_color.green = 0x00 * ( 65535 / 255 );
			online_color.blue  = 0x00 * ( 65535 / 255 );
		gtk_list_store_set(GTK_LIST_STORE(model), 
				&chat_user_iter, GYQUAD_COLOR, &online_color , -1);
							}
			if (update_it) {update_buddy_clist();}  /* MUST do this last */ 
			return( 0 );
		}
		this_friend = g_list_next( this_friend );
	}
	return( 0 );
}

int find_temporary_friend( char *friend ) {
	GList *this_friend;
	this_friend = temporary_friend_list;
	while( this_friend ) {
		if ( ! strcasecmp( this_friend->data, friend )) {
			return( 1 );
		}
		this_friend = g_list_next( this_friend );
	}
	return( 0 );
}

int add_temporary_friend( char *friend ) {
	char msg[200]="";
	GtkTreeModel *model;
		model=gtk_tree_view_get_model(chat_users);
		if (!model) {model=chat_user_model;}
		if (!model) {return 0;}

	if (find_friend(friend)) {return 0;}
	if ( find_temporary_friend(friend)) {return 0;}
	if ( (!strcasecmp(friend,ymsg_sess->user)) || (find_profile_name(friend))) {return 0;}

	temporary_friend_list = g_list_append( temporary_friend_list, strdup( friend ));

	if (temporary_friend_list) {
		int row;

		snprintf( msg, 198,	"%s  ** '%s'  %s **%s\n", "\033[#D8BEFFm", friend, 
		_("has been added to your TEMPORARY buddy list"),  YAHOO_COLOR_BLACK );
		append_to_textbox( chat_window, NULL, msg );

	 	remove_flooder( friend );
		if (ignore_check(friend) && (! perm_igg_check(friend) )) {ignore_toggle( friend );}
		if (mute_check(friend)) {mute_toggle( friend );}

		set_buddy_status_full(friend, "  ** Temporary Friend", 3);
		row = find_user_row( friend );
		if (row >= 0) {
			GdkColor online_color;
			online_color.red   = 0x62 * ( 65535 / 255 );
			online_color.green = 0x9B * ( 65535 / 255 );
			online_color.blue  = 0x55 * ( 65535 / 255 );
		gtk_list_store_set(GTK_LIST_STORE(model), 
				&chat_user_iter, GYQUAD_COLOR, &online_color , -1);
							}
			update_buddy_clist(); /* MUST do this last */ 
					}

	return( 0 );
}

int friend_is_invisible(char *user) {
	char tmp_user[64];
	struct yahoo_friend *f = NULL;
	if ( ! buddy_status ) { return 0; }
	strncpy( tmp_user, user, 62 );
	lower_str( tmp_user );
	f=yahoo_friend_find(user);
	if (f)  {
		if (f->main_stat) {	
			if (strstr(f->main_stat, _("Invisible"))) {return 1;}
			if (strstr(f->main_stat, _("invisible"))) {return 1;}
		}
	}
	return 0;
}



GtkTreePath *find_buddy_group(char *bud, GtkTreeModel *model)
{
	/* TreePath for buddy group is found or create if needed */
	/* Buddy group names are case-sensitive */ 
	GtkTreePath *tp = NULL;
	GtkTreeRowReference *trow=NULL;
	char tmp_user[80];
	strncpy( tmp_user, bud, 78 );
	if (!buddy_groups) {buddy_groups=g_hash_table_new(g_str_hash, g_str_equal);}
	trow = g_hash_table_lookup(buddy_groups, tmp_user);
	if (trow) {	tp=gtk_tree_row_reference_get_path(trow);	}

	if (tp) {return tp;}
	else {
		GtkTreeIter iter;
		GdkColor budcolor;		
		if (!buddy_groups) {buddy_groups=g_hash_table_new(g_str_hash, g_str_equal);}
		if (!buddy_groups) {return tp;}
		// gtk_tree_store_insert(GTK_TREE_STORE(model), &iter, NULL, 9000); 
		gtk_tree_store_prepend(GTK_TREE_STORE(model), &iter, NULL);
			budcolor.blue=46500; budcolor.red=0; budcolor.green=0;
			if (strcmp(bud,TMP_FRIEND_GRP)==0) {
				int mm= 65535 / 255;
				budcolor.blue=0xb2 *mm; budcolor.red=0xa2 *mm; budcolor.green=0x52 * mm;
			}
			gtk_tree_store_set(GTK_TREE_STORE(model), 
				&iter, 
				GYDOUBLE_PIX_TOGGLE, FALSE, 
				GYDOUBLE_PIX, NULL,
				GYDOUBLE_LABEL, _utf(tmp_user),
				GYDOUBLE_COL1, tmp_user,
				GYDOUBLE_TOP, "GYEBuddy_TopLvl",
				GYDOUBLE_COLOR, &budcolor,  -1);

			budcolor.blue=61423; budcolor.red=59881; budcolor.green=59367;
			if (strcmp(bud,TMP_FRIEND_GRP)==0) {
				int mm= 65535 / 255;
				budcolor.blue=0xf4 *mm; budcolor.red=0xc6 *mm; budcolor.green=0xda * mm;
			}
			gtk_tree_store_set(GTK_TREE_STORE(model), 
				&iter,   /* background color */
				GYDOUBLE_NUMCOLS, &budcolor,  -1);

		tp=gtk_tree_model_get_path(model, &iter);
		trow=gtk_tree_row_reference_new(model,tp);
		g_hash_table_insert(buddy_groups, g_strdup(tmp_user), trow);

		return tp;

		/* printf("create group:  %s\n", tmp_user); fflush(stdout); */  
	}
	return tp;
}
int clear_buddy_group_hash_cb(gpointer key, gpointer value,
        gpointer user_data) {
	g_free(key);
	gtk_tree_row_reference_free( (GtkTreeRowReference *) value);
 	return 1;

} 
void reset_buddy_groups() {
	if (buddy_group_list) {
		buddy_group_list=gyach_g_list_free( buddy_group_list );
		buddy_group_list=NULL;
	}
	if (!buddy_groups) {return;}
	g_hash_table_foreach_remove(buddy_groups,clear_buddy_group_hash_cb,NULL);
}

void create_buddy_groups_list_cb(gpointer key, gpointer value,
        gpointer user_data) {
	if (strcmp((char *) key,TMP_FRIEND_GRP)==0) {return;}
	buddy_group_list=g_list_append( buddy_group_list, strdup( (char *) key ));
} 

GList *get_buddy_group_list() {
	if (! buddy_group_list) {
		if (!buddy_groups) {
			if (! buddy_group_list) {
				buddy_group_list=g_list_append( buddy_group_list, strdup( "Friends" ));
				}
			return buddy_group_list;
			}
		g_hash_table_foreach(buddy_groups,create_buddy_groups_list_cb,NULL);
		if (! buddy_group_list) {
				buddy_group_list=g_list_append( buddy_group_list, strdup( "Friends" ));
		}
	}
	return  buddy_group_list;
}

void change_buddy_groups_cb(gpointer key, gpointer value,
        gpointer user_data) {
	struct yahoo_friend *f = value;
	if (!f) {return;}
	if (! f->buddy_group) {return;}
	if (!strcmp(f->buddy_group, oldgrpname)) {
		g_free(f->buddy_group);
		f->buddy_group=g_strdup(newgrpname);
	}
} 

void rename_buddy_group(char *ogroup, char *ngroup) {
	if (!buddy_groups) {return ;}
	if (newgrpname) {free(newgrpname);}
	newgrpname=strdup(ngroup);
	if (oldgrpname) {free(oldgrpname);}
	oldgrpname=strdup(ogroup);
	g_hash_table_foreach(buddy_status,change_buddy_groups_cb,NULL);
	free(newgrpname); newgrpname=NULL;
	free(oldgrpname); oldgrpname=NULL;
	if (selected_buddy_group)  {free(selected_buddy_group); selected_buddy_group=NULL;}
	if (selected_buddy)  {free(selected_buddy); selected_buddy=NULL;}
}



void get_buddy_identities_list_cb(gpointer key, gpointer value, gpointer user_data) {
		struct yahoo_friend *FRIEND_OBJECT=value;
		if (!key) {return; }
		if (!value) {return;}
		if (! buddy_identities_list) {return;}
		if (strlen(buddy_identities_list)>248) {return; }
		if (FRIEND_OBJECT->identity_id == buddy_identities_search) {
			if (strlen(buddy_identities_list)>1) {strcat(buddy_identities_list,",");}
			strncat(buddy_identities_list,(char *)key,35);
		}
}

char *get_buddy_identities_list(char *who)  {
	char *retval=NULL;
	if (! buddy_identities_list) {buddy_identities_list=malloc(288);}
	if (! buddy_identities_list) {retval=who; return who;}
	sprintf(buddy_identities_list,"%s", "");	
		if (buddy_status) {
			struct yahoo_friend *FRIEND_OBJECT=NULL;
			FRIEND_OBJECT=yahoo_friend_find(who);
			if (FRIEND_OBJECT) {
				buddy_identities_search=FRIEND_OBJECT->identity_id;
				if (buddy_identities_search>0) {
					g_hash_table_foreach(buddy_status, get_buddy_identities_list_cb, NULL );
				} else {snprintf(buddy_identities_list, 100, "%s", who);}
			} else {snprintf(buddy_identities_list, 100, "%s", who);}
		} else {snprintf(buddy_identities_list, 100, "%s", who);}
	return buddy_identities_list;
}


