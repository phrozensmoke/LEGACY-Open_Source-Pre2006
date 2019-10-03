/*****************************************************************************
 * bootprevent.c
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
 * Copyright (C) 2003-2006, Erica Andrews (PhrozenSmoke ['at'] yahoo.com)
 * Released under the terms of the GPL.
 * *NO WARRANTY*
 *
 * VERY preliminary code for adjusting and responding to booting attempts
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include <unistd.h>
#include <signal.h>
#include <time.h>

#include "gyach.h"
#include "callbacks.h"
#include "commands.h"
#include "friends.h"
#include "gyach_int.h"
#include "ignore.h"
#include "images.h"
#include "users.h"
#include "util.h"
#include "yahoochat.h"
#include "main.h"
#include "sounds.h"
#include "bootprevent.h"
#include "profname.h"

static char preempt_buf[640]="";

unsigned long sock_sleep=10000;   /* flow control, added PhrozenSmoke */
int enable_basic_protection=1;  /* enabled additional flow control if boot attemtps detected */
int enable_preemptive_strike=1;   /* fight fire with file ?? */
int possible_boot_attempts=0;
int re_warn_on_boot_attempt=0;
int boot_attempts=0; /* used to monitor attempts to 'boot' using conf declines  etc */

int num_dialogs_too_fast=0;
time_t dialog_clocker = 0;
int num_sounds_too_fast=0;
time_t sound_clocker = 0;
time_t packet_clocker = 0;

GList *flooder_list=NULL;
extern char *_(char *some) ;

char *bcode1="Hey \n\n\n\n\n\n\n\n3pSWP$%^&*()_!@#$%^&*()_!@#$%^&*()_!@#$%^&*()_!@#$%__()_!@#$%@#$%^&*()_!@#$%^&*()_!\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\t\n\n\n\n\n\n\n\n\n\n\n\n@#$%^&*()_!@#$%YJsQI5gy,#fcef03,#e3391c,#2c1de2,<font size=\"4\"><FADE #768489,#00ff00,#0000ff,\n\n\n#3030cf,#5cd228,#fcef03,#e3391c,#2c1de2,#eb1429,#85ee11,#22dd26,#718e79,#72b649,#3ac550,#0df274,f34><i><u><b><font size = \"4\">!@#$%^\x9essbKclrgw37fS/5</alt>";

char *bcode2=":<FADE #3030cf,#5cd228,#fcef03,#e3391c,#2c1de2,>\n\njA0EBAMCXUBirgNUTUJgyUH3MAXa3BfAQmNjXR6TClBj1GsH18HGZJ6OBGkGrMzZjx+4V/pd3WY3pSWPhYuqoTKrXDxKQi9/GBdPyP3O3BiYNw===kua7\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\t\n\n\n\n\n\n\n\n\n\n\n\n@#$7fS/5etItosXJYFuNPit3P/EW,#ff0000,#00ff00,#0000ff,#006699,#000066,#CCCCCC>hw8GYJsQI5gWKtpZhw8GYJsQI5gyT1F/TZmDxvvbAzZj4hsGubXnwlhHissbKclrgw37fS/5etItosXJYFuNPit3P/EWKtpZ</fade>";

char  *bcode3="#30cf34>\n\n\njA0EBAMCLhw8GYJsQI5gyT1F/TZmDxvvbAzZj4hsGubXnwlhHissbKclrgw37fS/5etItosXJYFuNPit3P/EWKtpZw2+R1lMddmqqMkI=mhGf\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\t\n\n\n\n\nEBAMCLhEBAMCLhEBAMCLhEBAMCLh\n\n\n\n\n\n\n<b>!@#$%^&*()_!@#$%^&*()_!@#$%^&**()&*()";

char *bprev_ding=NULL;


void check_boot_attempt() {
	possible_boot_attempts++;

	/*
		Explanation - added PhrozenSmoke: This MAY help us avoid server-side
		boots where a user keeps sending us packets like 'conference decline', 
		'webcam decline', 'voice chat invite', etc. - If we get sent too many of 
		these in a short period of time, Yahoo disconnects us.  I read a POSSIBLE
		solution is to send a 'ping' to Yahoo to reset the flags on the incoming 
		packets - so, when we get 4 'dangerous' packets, we'll send a ping, 
		it can't hurt- hopefully it will prevent us from getting booted.
		It seems to work in many cases, but not all.
	*/
	if (possible_boot_attempts==4)  {
		if (enable_basic_protection) {
		ymsg_messenger_ping(ymsg_sess);
		if (sock_sleep<50000) {sock_sleep=50000;}
							  }
		possible_boot_attempts=0;
							}

}



/* If somebody is sending us boot codes or doing something suspicious, we 
   send one right back */

void preemptive_strike(char *tmp)  {
	if (!enable_preemptive_strike) {return;}
	if (! strcmp( tmp, "" )) {return;}

	/* don't respond to yahoo's system messages */
	if (! strcasecmp(tmp, "yahoomessengerteam") ) {return; }
	if (! strcasecmp(tmp, "system") ) {return; }	
	if (! strcasecmp(tmp, ymsg_sess->user) ) {return; }
	if (! strcasecmp(tmp, "someone pretending to be you") ) {return; }
	if (find_profile_name(tmp))  {return; }
	if (strstr(tmp, " ") ) {return; } /* name with a space, invalid name */

		if ( capture_fp ) {	
			time_t time_llnow = time(NULL);
			fprintf(capture_fp,"\n\nBOOT PREVENT PREEMPTIVE STRIKE launched against   '%s'  at   [%s]\n\n", tmp, ctime(&time_llnow));
			fflush( capture_fp );
			}

	collect_profile_replyto_name();

	/* make sure they dont have us on their buddy list */
	if (!find_friend(tmp)) {ymsg_reject_buddy(ymsg_sess, tmp);}	

	if (!bprev_ding) {bprev_ding=strdup("<ding>");}

	snprintf(preempt_buf,128,"Access Denied. 8-x <:-p"); 
	ymsg_pm_full(ymsg_sess, tmp, preempt_buf, "garfield;0" );
	if (emulate_ymsg6) {
		snprintf(preempt_buf,128,"ch"); 
		ymsg_game_invite(ymsg_sess, tmp, preempt_buf, 1);
	}
	ymsg_pm_full(ymsg_sess, tmp, bprev_ding, "yfighter;0" );
	snprintf(preempt_buf,251, "%s\n\n%s","Warning: Unauthorized User.", bcode3);
	ymsg_conference_decline( ymsg_sess, tmp, tmp, preempt_buf); /* send a conf decline */
	ymsg_voice_invite(ymsg_sess, tmp, "YFiGHt3RNGaGeD");  /* send voice invitatation */
	ymsg_imvironment( ymsg_sess, tmp , "purinadogs;0"); /* imvironment */
	if (! emulate_ymsg6) {
		snprintf(preempt_buf,453, "%s",bcode2);
		ymsg_conference_decline( ymsg_sess, tmp, tmp, preempt_buf); /*  conf decline */
		}
	ymsg_webcam_invite_reject( ymsg_sess, tmp);
	snprintf(preempt_buf,200, "%s",bcode3);  /* send a PM */
	ymsg_pm_full(ymsg_sess, tmp, preempt_buf, "lastshot;0" );
	ymsg_imvironment( ymsg_sess, tmp , "gotbootZ"); /* imvironment */	
	ymsg_pm(ymsg_sess, tmp, bprev_ding);
	snprintf(preempt_buf,450, "%s",bcode3);
	ymsg_conference_decline( ymsg_sess, tmp, tmp, preempt_buf); /* send a conf decline */
	snprintf(preempt_buf,278, "Illegal User: %s %s",tmp, bcode3);  /* send a PM */
	ymsg_pm_full(ymsg_sess, tmp, preempt_buf, "smashbros;0" );
	if (! emulate_ymsg6) {
		ymsg_voice_invite(ymsg_sess, tmp, tmp);  /* send voice invitatation */
		ymsg_imvironment( ymsg_sess, tmp , "leaves;0");
		} /* imvironment */
	snprintf(preempt_buf,100, "%s","Communication Error: Unknown User.");
	ymsg_conference_decline( ymsg_sess, tmp, tmp, preempt_buf); /* send a conf decline */
	snprintf(preempt_buf,100, "%s", "8-x 8-x  [ACCESS DENIED]  8-x 8-x  >:/  =)) "); 
	ymsg_pm_full(ymsg_sess, tmp, preempt_buf, "qMkI=mhGf\n;0" );
	if (emulate_ymsg6) {
		GYAUDIBLE *sm_ptr;
		int aud_count=0;
		init_audibles();
		sm_ptr = &gyache_audibles[0];
		while( sm_ptr->aud_file && (aud_count<4) ) {
			ymsg_audible(ymsg_sess, tmp, sm_ptr->aud_file, sm_ptr->aud_text, sm_ptr->aud_hash);
			sm_ptr++;
			aud_count++;
		}
		if (sm_ptr->aud_file){
			ymsg_audible(ymsg_sess, tmp, sm_ptr->aud_file, preempt_buf, preempt_buf);
		}
		ymsg_conference_decline( ymsg_sess, tmp, tmp, preempt_buf); 
	}

	if (emulate_ymsg6) {
		snprintf(preempt_buf,128,"do"); 
		ymsg_game_invite(ymsg_sess, tmp, preempt_buf, 1);
	}
	reset_profile_replyto_name();
}




void boot_attempt(char *who, char *method)  { /* a warning someone is trying to boot us */
	if (!who) {return;}

	boot_attempts++;
	if (boot_attempts==4) {

		if (strlen(who)>1) {

				 if ( (! ignore_check( who )) && (!find_friend(who)) ) {

			snprintf( preempt_buf, 353, "%s%s%s  ** WARNING: SOMEONE IS POSSIBLY ATTEMPTING TO BOOT YOU USING THE NAME '%s' BY SENDING MALICIOUS '%s' ! This user will now be put on IGNORE.**%s%s%s\n",
						YAHOO_STYLE_BOLDON, YAHOO_STYLE_ITALICON,
						YAHOO_COLOR_RED, who, method, 
						 YAHOO_STYLE_BOLDOFF, 
					 	YAHOO_COLOR_BLACK, YAHOO_STYLE_ITALICOFF );  
						append_to_textbox_color( chat_window, NULL, preempt_buf );

					if ( (!find_friend(who)) ||  (!never_ignore_a_friend) )  {

						ignore_toggle( who );
						play_sound_event(SOUND_EVENT_REJECT);
						if (!find_friend(who)){ preemptive_strike(who); ymsg_reject_buddy(ymsg_sess, who);}
														   									  }
										        		}  else {
			if ((re_warn_on_boot_attempt==6) || (re_warn_on_boot_attempt==0)) 
				{
			snprintf( preempt_buf, 353, "%s%s%s  ** '%s' BOOT ATTEMPT FROM: '%s' **%s%s%s\n",
						YAHOO_STYLE_BOLDON, YAHOO_STYLE_ITALICON,
						YAHOO_COLOR_RED, method, who,
						 YAHOO_STYLE_BOLDOFF, 
					 	YAHOO_COLOR_BLACK, YAHOO_STYLE_ITALICOFF );  
						append_to_textbox_color( chat_window, NULL, preempt_buf );
						/* Try to keep our name off their list */
						if (!find_friend(who)){ ymsg_reject_buddy(ymsg_sess, who);}

						if (re_warn_on_boot_attempt==6) {re_warn_on_boot_attempt=1;}
						else {re_warn_on_boot_attempt++;}

				} else {re_warn_on_boot_attempt++;}
						ymsg_typing( ymsg_sess, who, 1 );
																  }


					} else  {
			snprintf( preempt_buf, 353, "%s%s%s  ** WARNING: SOMEONE IS POSSIBLY  ATTEMPTING TO BOOT YOU (name unknown) BY SENDING MALICIOUS '%s' ! **%s%s%s\n",
						YAHOO_STYLE_BOLDON, YAHOO_STYLE_ITALICON,
						YAHOO_COLOR_RED, method, YAHOO_STYLE_BOLDOFF, 
					 	YAHOO_COLOR_BLACK, YAHOO_STYLE_ITALICOFF );  
						append_to_textbox_color( chat_window, NULL, preempt_buf );
							  }
		
		boot_attempts=0;  /* reset */
		emergency_keep_alive(who);

								 }
}


/* When we are being flooded, this seems to help keep us online in many (but not all) cases */

void emergency_keep_alive(char *who) {
	if (enable_basic_protection) {
							ymsg_messenger_ping(ymsg_sess); /* ping */
							if (who) {
								if (strlen(who)>1) {
									ymsg_typing( ymsg_sess, who, 1 );   /* a little activity */
														  }
										}
							if (sock_sleep<75000) {sock_sleep=75000;}    /* flow control */
							  			}
}


/* check for excessively large packets on dangerous type packets*/

int is_boot_attempt(char *who, char *method, int packet_size) {
	if (! enable_basic_protection) {return 0;}
	
	if (packet_size<950) { check_boot_attempt();}

	if (! strcmp( who, "" )) {return 0;}
	if (! strcasecmp(who, "yahoomessengerteam") ) {return 0; }
	if (! strcasecmp(who, "system") ) {return 0; }
	if (! strcasecmp(who, ymsg_sess->user) ) {return 0; }
	if (find_profile_name(who))  {return 0; }

			if (packet_size>550)  {
					if ( ( (!find_friend(who)) && (!find_temporary_friend(who))) ||  
					(!never_ignore_a_friend) )  {
						log_possible_flood_attack(who, 6,method);  /* six points, shut 'em down faster */
						push_packet_suppression ();
						return 1; 
									  						} else {
						emergency_keep_alive(who);
																	  }
										   }
	return 0;
}




int remove_flooder( char *flooder ) {
	GList *this_flooder;
	this_flooder = flooder_list;
	while( this_flooder ) {
		if ( ! strcasecmp( this_flooder->data, flooder )) {
			flooder_list = g_list_remove_link( flooder_list, this_flooder );
			gyach_g_list_free( this_flooder );
			snprintf( preempt_buf, 353, "%s  ** '%s'  %s **%s\n", "\033[#FF5EBCm", flooder, 
			_("is no longer designated as a FLOODER"),  YAHOO_COLOR_BLACK );
			append_to_textbox( chat_window, NULL, preempt_buf );
			set_last_comment(flooder,"is no longer designated as a FLOODER");


		if ( capture_fp ) {	
			time_t time_llnow = time(NULL);
			fprintf(capture_fp,"\n\nRemoved  '%s'  from list of possible malicious users (flooders/booters)  at  [%s]\n\n", flooder, ctime(&time_llnow));
			fflush( capture_fp );
			}

			return( 0 );
		}
		this_flooder = g_list_next( this_flooder );
	}
	return( 0 );
}

int is_flooder( char *flooder ) {
	GList *this_flooder;
	this_flooder = flooder_list;
	if (! enable_basic_protection) {return 0; }
	if (!flooder_list) {return 0;}
	while( this_flooder ) {
		if ( ! strcasecmp( this_flooder->data, flooder )) {
			return( 1 );
		}
		this_flooder = g_list_next( this_flooder );
	}
	return( 0 );
}

int add_flooder( char *flooder ) {
	if (! enable_basic_protection) {return 0; }
	if ( is_flooder(flooder)) {return 0;}
	if (!strcasecmp(flooder,ymsg_sess->user)) { return 0;}
	if (find_profile_name(flooder))  {return 0; }
	if (find_friend(flooder)) {return 0;}
	if (find_temporary_friend(flooder)) {return 0;}

	/* Yahoo 'system' messages */
	if (! strcasecmp(flooder, "yahoomessengerteam") ) {return 0; }
	if (! strcasecmp(flooder, "system") ) {return 0; }


	flooder_list = g_list_append( flooder_list, strdup( flooder ));
	if (!flooder_list) {return 0;}
	snprintf( preempt_buf, 353,	"%s  ** '%s'  %s **%s\n", "\033[#FF7A4Em", flooder, 
	_("is now designated as a FLOODER. This person is trying to BOOT you."),  YAHOO_COLOR_BLACK );
	append_to_textbox( chat_window, NULL, preempt_buf );

		if ( capture_fp ) {	
			time_t time_llnow = time(NULL);
			fprintf(capture_fp,"\n\nPOSSIBLE MALICIOUS USER (Flooder/Booter)  '%s'  at   [%s]\n\n", flooder, ctime(&time_llnow));
			fflush( capture_fp );
			}

	if (g_list_length(flooder_list)<4) {
		/* limit to 3 dialog warnings, if they have many bots, we dont want 
			dialogs popping up uncontrollably...3 warnings is enuff to get the idea */
		snprintf(preempt_buf,353, "%s :\n%s\n", flooder,_("is now designated as a FLOODER. This person is trying to BOOT you."));
		show_ok_dialog(preempt_buf);
		push_packet_suppression ();
							  				 }
	return( 0 );
}



/* explanation: although the boot/flood protection tries vigorously 
    to keep our screen name off the buddy lists of booters/flooders,
    a rapid-fire 'buddy add' bomb may still leave our name on their 
    list...and in the event we get booted by somebody adding us
    without our permission, we probably want our name off their 
    list - so this routine will be run on login and just before logout...
    strip our name off the buddy lists of all flooders/booters/stalkers
    we encountered this session - this is a smart idea, so that 
    whoever is harassing us doesn't have the luxury of seeing us 
    come online just by looking at their list */

void flooder_buddy_list_protect() {
	GList *this_flooder;
	this_flooder = flooder_list;
	if (! enable_basic_protection) {return ; }
	if (!flooder_list) {return ;}
	while( this_flooder ) {
		ymsg_reject_buddy(ymsg_sess, this_flooder->data); 
		ymsg_reject_buddy(ymsg_sess, this_flooder->data); /* twice to be safe */
		this_flooder = g_list_next( this_flooder );
							 }
}





void log_possible_flood_attack(char *who, int points, char *method) {
	int counter=0;
	int takeoff=0;
	int mypts=1;

	if (!who) {return;}				
	if (strlen(who)<1) {return;}
	if (!strcasecmp(who,ymsg_sess->user)) {set_last_comment(who,method); return;}
	if (find_profile_name(who))  {set_last_comment(who,method); return ; }
	if (never_ignore_a_friend && find_friend(who) ) { set_last_comment(who,method); return;}
	if (never_ignore_a_friend && find_temporary_friend(who) ) {set_last_comment(who,method); return;}


		if ( capture_fp ) {	
			time_t time_llnow = time(NULL);
			fprintf(capture_fp,"\n\n[%s]  POSSIBLE FLOOD OR BOOT ATTACK ATTEMPTED:    Illegal/Unwanted Communication blocked by Gyach-Enhanced from '%s' using method/message type '%s'\n\n", ctime(&time_llnow), who, method);
			fflush( capture_fp );
			}

	mypts=points;
	if (! enable_basic_protection) {mypts=1;}

	/* a known booter (at least in my circles) 'nowguessXX' will be listed as a flooder immediately */
	/* Marking them as a flooder disables the 'reject' sound event for this person AND 
	    adds control over how often 'preemptive strikes' are launched, so we dont clog our
	    connection playing ping-pong and don't lag gyachE with too many sounds playing over and over */

	if (!strncasecmp(who,"nowguess", 8))  { /* This is bad intentions */
		/* mark them as a flooder */
		if (enable_basic_protection) { if (! is_flooder(who)) {mypts=7;} }
														}

	if (!strncasecmp(who,"icreate", 7))  { /* This is bad intentions */
		/* mark them as a flooder */
		if (enable_basic_protection) { if (! is_flooder(who)) {mypts=7;} }
														}

	if (strstr(who,"i_create"))  { /* This is bad intentions */
		if (enable_basic_protection) { if (! is_flooder(who)) {mypts=7;} }
										}

	while (takeoff<mypts) {
		counter=set_last_comment(who,"Possible Flood Attack or Boot Attempt");
		takeoff++;
								}

				if (( ignore_on_mults ) && 
					( counter > ignore_on_mults ) && 
					( ! ignore_check( who ))  ) {

					ignore_toggle( who );
					snprintf( preempt_buf, 498, 
						"%s  ** Gyach-E - auto-ignored '%s'  for sending you several suspcious messages  (Voice invites, Conference/Chat invites, Buddy Adds/Rejects, Peer-to-Peer files and snoop attempts, Webcam Invites/Declines, etc.).  Last message sent: '%s' **%s\n", YAHOO_COLOR_RED, who, method, YAHOO_COLOR_BLACK );
					append_to_textbox( chat_window, NULL, preempt_buf );
					if (!find_friend(who)) { preemptive_strike(who); }

																	}


	if (! enable_basic_protection) {return; }

	check_boot_attempt();

	if (counter==3) {
		if (method) {
			if (!strcasecmp(method,"blocked private message")) {
				/* three blocked PMs, not a bot, but an idiot */
				preemptive_strike(who);
																					  }
						 }
			}

	if ((counter>6) && (! is_flooder(who)) )  { /* This is bad intentions */
		/* mark them as a flooder */
		add_flooder(who);
		/* add them to GYach's ignore list */
		if (! ignore_check(who)) {
			ignore_toggle(who);

			snprintf( preempt_buf, 453, "%s%s%s  ** WARNING: SOMEONE IS POSSIBLY ATTEMPTING TO BOOT YOU USING THE NAME '%s' BY SENDING MALICIOUS '%s' ! This user has been put on IGNORE.**%s%s%s\n",
						YAHOO_STYLE_BOLDON, YAHOO_STYLE_ITALICON,
						YAHOO_COLOR_RED, who, method, 
						 YAHOO_STYLE_BOLDOFF, 
					 	YAHOO_COLOR_BLACK, YAHOO_STYLE_ITALICOFF );  
						append_to_textbox_color( chat_window, NULL, preempt_buf );

											}

		emergency_keep_alive(who);

		/* If they have our name on their list, force us to appear offline if possible */
		if (emulate_ymsg6) {
				ymsg_stealth(ymsg_sess, who, 0);
				ymsg_stealth_perm(ymsg_sess, who, 0);
		}

		ymsg_reject_buddy(ymsg_sess, who);  /* Try to keep our name off their list */
		if (mypts<7) {preemptive_strike(who);}  /* SOMEtimes this knocks their flood bot offline */
		if (mypts<7) {preemptive_strike(who);}  /* SOMEtimes this knocks their flood bot offline */
														}

	if ((counter>9) && (! perm_igg_check(who)) ) {
		/* Next line of defense, put them on Yahoo's permenant iggy bin 
			as sometimes that helps block some of this crap from the server-side */
		/* From what I've seen, putting a person on Yahoo's permenant ignore list 
			seems to put a halt to a couple things, like repeated Voice chat invites 
			(i.e. 'sound bombs') and unwanted Private Messages -once they-re on that 
			list, some of this junk stops coming, but not all  */


		ymsg_reject_buddy(ymsg_sess, who);  /* Try to keep our name off their list */
		ymsg_perm_ignore(ymsg_sess,who, 0); 
		emergency_keep_alive(who);
		return ;
																	}

	/* from here on out, the focus is staying online */
	if (counter>9) {
		char boottype[110]="";
		push_packet_suppression ();
		snprintf(boottype,108,"Flood Attack/Yahoo Bomber: %s", method);
		if (mypts>11) {boot_attempts=4; re_warn_on_boot_attempt=0; }
		boot_attempt(who,boottype);
					}


	if ( (counter>9) && (mypts<7) ) {
		int mymod=1;
		mymod=counter%10;
		if (mymod==0) {
					ymsg_reject_buddy(ymsg_sess, who);  /* Try to keep our name off their list */
							   }
												}

	if ( (counter>24) && (mypts<4) ) {
		int mymod=1;
		mymod=counter%15;		
		if (mymod==0) {preemptive_strike(who); }
												}

}

/* Maybe prevent dangerous floods that cause dialogs to open repeatedly 
   or sounds to play back-to-back, when 'basic boot prevention' is enabled */

int dialogs_opening_too_fast() {
	long dialog_clocking;
	time_t time_llnoww=time(NULL);
	if (! enable_basic_protection) {return 0; }
	dialog_clocking=time_llnoww-dialog_clocker;
	if (dialog_clocking <20)  { 
		/* Have lots of dialogs been opening within the past
		20 seconds???..possible flood/boot attack  */
		num_dialogs_too_fast++;
		if (num_dialogs_too_fast>6) {
			if (num_dialogs_too_fast==7) {

			snprintf( preempt_buf, 453, "\n%s%s%s  ** WARNING: TOO MANY DIALOG MESSAGES HAVE BEEN OPENED TOO QUICKLY (Possible Flood or Boot Attack from a malicious Yahoo user?) -  Dialog Messages will be temporarily disabled for 2.5 minutes to help protect your system, and default actions will be taken for all 'confirmation' dialogs.\n%s%s%s",
						YAHOO_STYLE_BOLDON, YAHOO_STYLE_ITALICON,
						YAHOO_COLOR_RED,  YAHOO_STYLE_BOLDOFF, 
					 	YAHOO_COLOR_BLACK, YAHOO_STYLE_ITALICOFF );  
						append_to_textbox_color( chat_window, NULL, preempt_buf );

			}
		push_packet_suppression ();
		dialog_clocker=time(NULL);
		return 1;
		}
	} else {
		if ( (num_dialogs_too_fast<7) || (dialog_clocking>150)) {

			if (num_dialogs_too_fast>6) {
				snprintf( preempt_buf, 255, "\n%s%s%s  ** Dialog Windows Are Re-Enabled (Dialog message bomb protection turned off) ***\n%s%s%s",YAHOO_STYLE_BOLDON, YAHOO_STYLE_ITALICON, YAHOO_COLOR_RED,  YAHOO_STYLE_BOLDOFF, 
				YAHOO_COLOR_BLACK, YAHOO_STYLE_ITALICOFF );  
				append_to_textbox_color( chat_window, NULL, preempt_buf );
				push_packet_suppression ();
			}

			dialog_clocker=time(NULL);
			num_dialogs_too_fast=0;
			return 0;
		} else { return 1;}
	}
	return 0;
}

int sounds_opening_too_fast() {
	long sound_clocking;
	time_t time_llnoww=time(NULL);
	if (! enable_basic_protection) {return 0; }
	sound_clocking=time_llnoww-sound_clocker;

	if (sound_clocking <20)  { 
		/* Have lots of sounds been opening within the past
		20 seconds???..possible flood/boot attack  */
		num_sounds_too_fast++;
		if (num_sounds_too_fast>7) {
			if (num_sounds_too_fast==8) {

			snprintf( preempt_buf, 453, "\n%s%s%s  ** WARNING: TOO MANY SOUND EVENTS HAVE BEEN PLAYED TOO QUICKLY (Possible Flood or Boot Attack from a malicious Yahoo user?) -  Sound Events will be temporarily disabled for 2.5 minutes to help protect your system.\n%s%s%s",
						YAHOO_STYLE_BOLDON, YAHOO_STYLE_ITALICON,
						YAHOO_COLOR_RED,  YAHOO_STYLE_BOLDOFF, 
					 	YAHOO_COLOR_BLACK, YAHOO_STYLE_ITALICOFF );  
						append_to_textbox_color( chat_window, NULL, preempt_buf );

			}
		push_packet_suppression ();
		sound_clocker=time(NULL);
		return 1;
		}
	} else {
		if ( (num_sounds_too_fast<8) || (sound_clocking>150)) {
			if (num_sounds_too_fast>7) {
				snprintf( preempt_buf, 255, "\n%s%s%s  ** Sound Events Are Re-Enabled (Sound bomb protection turned off) ***\n%s%s%s",YAHOO_STYLE_BOLDON, YAHOO_STYLE_ITALICON, YAHOO_COLOR_RED,  YAHOO_STYLE_BOLDOFF, 
				YAHOO_COLOR_BLACK, YAHOO_STYLE_ITALICOFF );  
				append_to_textbox_color( chat_window, NULL, preempt_buf );
				push_packet_suppression ();
			}

			sound_clocker=time(NULL);
			num_sounds_too_fast=0;
			return 0;
		} else {return 1;}
	}
	return 0;
}

/* Another means of controlling booting is to skip over
   consecutive, identical packets when things look dangerous
   When necessary, we'll tell the yahoochat.c module to 
   skip duplicate packets for a period of 90 seconds  */

void check_packet_suppression() {
	if (! ymsg_sess->suppress_dup_packets) {return;}
	else {
		/* Check the timer and turn packet suppression 
		off after 75 seconds */
		long packet_clocking;
		time_t time_llnoww=time(NULL);
		packet_clocking=time_llnoww-packet_clocker;
		if (packet_clocking>75) {ymsg_sess->suppress_dup_packets=0;}
		}
}

void push_packet_suppression () {
	if (! enable_basic_protection) {return; }
	ymsg_sess->suppress_dup_packets=1;
	packet_clocker=time(NULL);
}






