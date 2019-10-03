/*****************************************************************************
 * spamcheck.c
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
 * Copyright (C) 2003 Erica Andrews (PhrozenSmoke ['at'] yahoo.com)
 * Released under the terms of the GPL.
 * *NO WARRANTY*
 *
 * Basic module for checking for spam and spam bots
 *****************************************************************************/



#include "gyach.h"
#include "users.h"
#include "friends.h"
#include "util.h"
#include "profname.h"

char *spam_alias=NULL;

/* Known, legit consonant combinations */ 
char *consonant_combos[] = {"bl", "br", "bs", "cc", "ch", "ck", "cr", "ct", "dd", "dr", "dw", "ff", "fr", "ft", "gg", "gh", "gr", "gy", "hw", "lc", "ld", "lk", "ll", "lm", "lr", "ls", "lt", "lv", "lw", "mb", "mm", "mp", "ms", "nc", "nd", "nf", "ng", "nh", "nk", "nn", "nr", "ns", "nt", "nv", "nz", "pc", "pd", "ph", "pl", "pp", "pr", "ps", "pt", "rc", "rd", "rf", "rg", "rk", "rm", "rn", "rp", "rr", "rs", "rt", "rv", "rw", "sc", "sh", "sk", "sl", "sm", "sp", "ss", "st", "tc", "tf",  "th", "tl", "tm", "tr", "ts", "tt", "tz", "wh", "wl", "wn", "xp", "xt", NULL};

		/* exceptions for 'net grammar', commonly used abbreviations */
		/* also word fragments, if present, could create false positives */
		char *abbrevs[] = {
			"blk", "wht", "blck", "swt", "latn",  "pr", "grl", "sht", "sml", "tll", "tny", 
			"shrt",	"lil",	"nsty",	"pzzy",	"pssy",	"dck",	"qt","2b", "4u", "2u", "drty", "btch", 
			"2c", "nv", "b4", "crzy", "sft", "ldy", "sxy", "sxxy", "lvn", "luvn", "nj", "fck", 
			"_u_","_4_", "_c_", "_2_", "_n_", "_y_", "_n_", "_b_", "_o_", "_i_",
			"nkd",  "nakd", "hrny", "hny",  "str", "ight", "and", "ought",  "aught", "off", "skyn",    
			"atch",  "utch",  "itch",  "etch",  "ytch",  "otch",  "ms", "mr", "sprt", "pnk", 
			"grn", "prpl", "ppl",  "plz", "dr", "brk", "sch", "own", "awn", "chr", "luvs", 
			"ong", "ang", "ing", "ung", "yng", "eng", "scr", "cpl", "sng", "mph", "dj", 
			"sbcglobal", "bbw", "cpls", "cplz", "miss", "tha", "thr", "the", "irls", "arls", "mply", 
			"thi", "tho", "wha", "who", "_nu_", "mc", "lyck", "slvr", "gldn", "irlz", "arlz", 
			"33k", "3ak", "olks", 
			NULL
								};

		/* abbreviations only acceptable at the beginning of a name */
		char *abbrevs_two[] = {
			"u_","4_", "c_", "2_", "n_", "y_", "n_", "b_", "o_", "i_", "ny", "nu_", 
			NULL
								};


		/* abbreviations - usually bots if appearing at beginning of name */
		char *abbrevs_bad[] = {
			"xxx_","nakd_", "hrny_", "cam_", "camgrl_", "camgirl_", "camgyrl_", "bif_", "hny_",
			"cams_", "xlx_", "swf_", "sub_", "naked_","nude_","pics_", "pic_", "www_", "nst_",  "super_btry", "nsty_", "h0rny_", "coed_",
			NULL
								};


		/* spam text in comments - when seen along with 'http' */
		char *spam_comments[] = {
			"pics", "web cam","my cam","cam ","cum", "100%", "webcam", "movies", "britt",
			"dialer", "creditcard", "credit card", "click here", "click this", "popups", "see link", 			
			"cams",	"camz", "spam", "porn", "make money", "$", "see me", "watch me",  "steamy", 
			"explicit", "xxx", "nude", "naked", "playing with", "play with", "get paid", " antics", 
			"click me", "click my", "live show", "free!", "now!", "join!",  "web-cam", "trial", 
			"join now", "meet single","free to join","join free", "strip", "mastur", "britt", 
			"singles", "personal", "escort", " dating", 
			NULL
								};

		char *spam_comments2[] = {"http", "www.", "teen", "cum", 
			"hard", "wet", "pic", "free","sex", "check out", "check it", NULL};

		char *spam_probe[] = {"explicit","xxx" , "porn", NULL};

		/* some spam bots are kind enough to put [SPAMBOT] 
		  or other noticeable tags in their postings, some other things 
		here are almost always spam when seen in chat */

		char *spam_bot_flags[] = {
			"[spambot]", "(spambot)", "[spam]",			
			"brittanyz", "brittaniz", "britty", "brittzy",  /* known spam bot series */
			 "(spam bot)", "(spam-bot)", "[spam-bot]", "[spam bot]",
			"click here", "click this", "see link", "click me", "join now!", "join for free","join free", 
			"h0rny", "www.britt", 
			 NULL						  };


/* added: PhrozenSmoke, basic chat spam/bot filter stuff */
int is_vowel(int c) {
	if (c=='a') {return 1;}
	if (c=='e') {return 1;}
	if (c=='i') {return 1;}
	if (c=='o') {return 1;}
	if (c=='u') {return 1;}
	if (c=='y') {return 1;}
return 0;
}


int is_spam_name(char *who) {
	if (!who) {return 0;}
	if (!enable_chat_spam_filter) {return 0;}
	if (! strcasecmp(who, "yahoomessengerteam") ) {return 0; }  /* y! sys message */
	if (! strcasecmp(who, "system") ) {return 0; }  /* y! sys message */
	if (!strcasecmp(who,ymsg_sess->user))   {return 0;}	
	if (find_profile_name(who))  {return 0; }
	if (find_friend(who)) {return 0;}
	if (find_temporary_friend(who))  {return 0;}
	
	/* this method handles possible bot names of 5+ chars that do NOT have 
            a 'nick' assigned to them */



	// if (!strchr(who,'@')) {return 0;}
	if (strlen(who)<6) {return 0;}			
	else {
		int looks_bottish=0;
		float percentage;
		char wwho[64]="";
		char consonant_cmp[8];
		int known_combo=0;
		int consec_consonants=0;
		int consec_numbers=0;
		int consec_vowels=0;
		int num_vowels=0;
		int slimit=0;
		int atlimit=0;
		int underscores=0;
		int conresets=0;
		char lastchar='@'; /* this should never appear in a screenname */
		char **cptr = abbrevs_bad;
		

		sprintf(wwho, "%s", ""); /* clear any old data */
		strncpy(wwho,who,62);
		lower_str( wwho );

		if ( strstr(wwho,"porn" )) {return 1;}
		if ( strstr(wwho,"camz" )) {return 1;}
		if ( strstr(wwho,"webcam" )) {return 1;}
		if ( strstr(wwho,"cams" )) {return 1;}
		if ( strstr(wwho,"pics" )) {return 1;}
		if ( strstr(wwho,"brittanyz" )) {return 1;}  /* known spam bot series */
		if ( strstr(wwho,"brittaniz" )) {return 1;}  /* known spam bot series */
		if ( strstr(wwho,"britty" )) {return 1;}  /* known spam bot series */
		if ( strstr(wwho,"brittzy" )) {return 1;}  /* known spam bot series */
		if ( strstr(wwho,"brttz" )) {return 1;}  /* known spam bot series */
		if ( strstr(wwho,"datingis" )) {return 1;}  /* known spam bot series */
		if ( strstr(wwho,"localdating" )) {return 1;}  /* known spam bot series */
		if ( strstr(wwho,"singles" )) {return 1;}  /* known spam bot series */
		if ( strstr(wwho,"personals" )) {return 1;}  /* known spam bot series */
		if ( strstr(wwho,"escort" )) {return 1;}  /* known spam bot series */

		while( *cptr ) {
			if (!strncasecmp(wwho,*cptr, strlen(*cptr) )) {return 1;}
			cptr++;
							}

		/* detect bot-like names with _  format such as 'dmuteqno_qfwfwnku' and 
		    'aybbyuum_oxakyokj'  */
		/* also variations of this, like:  'uhfljlbl_eifaycns29'  */

		/* Examine stuff after the underscore
		  to catch stuff like bethany_larajpnw283  dionne_ymawpdcx965
		  Spamish after the underscore
		 */
		if ( (strlen(wwho) > 8) && strchr(wwho,'_')) {
			char aftinspect[12]="";
			char *aftunder=strrchr(wwho,'_');
			if (aftunder) {
				snprintf(aftinspect, 9, "%s", aftunder+1); 
				if (strlen(aftinspect)>=8) {
					aftinspect[8]='\0';
					if (is_spam_name(aftinspect)) {return 1;}
					}
				}
		}

		if (strlen(wwho)==9) { /* smaller variants like 'jlbl_eifa' */
			if (wwho[4]=='_') {looks_bottish=1;}
										}

		if (strlen(wwho)>15) {
			/* usually the 5th or 9th char is an underscore */			
			if (wwho[8]=='_') {
				looks_bottish=1;
				int very_bottish=0;

				if (strlen(wwho)==17) {very_bottish=1;}

				if ((strlen(wwho)>=19) && 
					(strlen(wwho)<=21))	{
					if (isdigit(wwho[17])) 		{
						if (isdigit(wwho[18])) 		{ 
							very_bottish=1;
															}
														}
												 	}

				if ( very_bottish ) {   
					/* make sure this is the only underscore */
					/* if not, further inspection is warranted, could be innocent bystander */
					int ifound=0;
					int iprogress=0;
					while (iprogress<16)  {
						if  (wwho[iprogress]=='_') { ifound++;}
						iprogress++;
													}
					if (ifound<2) {return 1; } /* only underscore, probably bot name */
												 }
									 }
								}


	/* assume it's NOT a bot if they took the time to make a 'nickname' */
	spam_alias=get_screenname_alias(who);
	if (spam_alias) {
		if (strcasecmp(who,spam_alias)) {free(spam_alias); spam_alias=NULL; return 0;}
		free(spam_alias); spam_alias=NULL;
	}


		/* ignore trailing numbers */
		slimit=strlen(wwho)-1;
		while (slimit>2) {
			if ( isdigit(wwho[slimit])) { wwho[slimit]='\0'; slimit--; continue;}
			if (wwho[slimit]=='_') {wwho[slimit]='\0'; slimit--; continue; }
			break; 
								}

		// printf("who-extract: %s\n",wwho); fflush(stdout);

		slimit=strlen(wwho);
		while (atlimit<slimit) {
			if (wwho[atlimit]=='\0') {break;}

			if (consec_consonants==2) {
				known_combo=0;
				snprintf(consonant_cmp, 4, "%c%c", wwho[atlimit-1], wwho[atlimit] );
				cptr=consonant_combos;
				if ((! strchr(consonant_cmp, '_')) && (! strchr(consonant_cmp, 'y')) ) {
					while( *cptr ) {
						if (!strncasecmp(consonant_cmp,*cptr, 2)) {	known_combo=1; 	break;	}
						cptr++;
								}
						} else {known_combo=1; }
				if (! known_combo) {consec_consonants++; looks_bottish=1;} /* penalize */
			}

			if (consec_consonants>3) {
						consec_consonants=0; 
						conresets++;
						/* five consonants in a row, almost always spammer */
						if ( (! is_vowel(wwho[atlimit])) && (! isdigit(wwho[atlimit])) ) {conresets++;}
												}

			// printf("consec_consonants:  %d\n", consec_consonants); fflush(stdout);

			if (consec_numbers>2) { if (! looks_bottish) {return 0; } }

			if (isdigit(wwho[atlimit])) {
				if (consec_vowels<4) {consec_vowels=0;}

				if (consec_consonants==3) {
						percentage=(float) (((float)num_vowels/(float)atlimit) * 100.0);
						if ( percentage >= 25.0) {consec_consonants=0;}
													 }

				consec_numbers++;
				lastchar=wwho[atlimit];
				atlimit++; 
				continue;
									
			} else {	
				if (consec_numbers<3) {consec_numbers=0;} 
					}
		
			if (! is_vowel(wwho[atlimit])) {
				if (consec_vowels<4) {consec_vowels=0;}

				if (wwho[atlimit] != '_') 
				{ 
					if (wwho[atlimit] != lastchar) {consec_consonants++;} 

				} else {	
					underscores++;
					if (consec_numbers<3) {consec_numbers=0;} 
					if (consec_consonants<4) {consec_consonants=0;}
						}
				lastchar=wwho[atlimit];
				 atlimit++; 
				continue;
													  } else {
				if ( (wwho[atlimit] != lastchar) && (wwho[atlimit] != 'y')) {consec_vowels++;}
				
				if (consec_consonants<4) {
					if ( (wwho[atlimit] == 'y') && (consec_consonants==3)) 
						{consec_consonants++; } else {	consec_consonants=0; }
													 }
				num_vowels++;
							 								 }

			lastchar=wwho[atlimit];
			atlimit++;

										}
	
		/* probably not a bot, just a strange, intricate name */
		if (consec_numbers>2) { if (! looks_bottish) { return 0;} }


		/* add 'vowel points' for known abbreviations */
		cptr = abbrevs;
		while( *cptr ) {
			if ( strstr(wwho,*cptr )) {
				num_vowels++; consec_consonants--; conresets--; 
				if (strlen(*cptr )>=4) {consec_consonants=0; conresets=0;}
											  }
			cptr++;
							}

		cptr = abbrevs_two;
		while( *cptr ) {
			if (!strncasecmp(wwho,*cptr, strlen(*cptr) )) {num_vowels++;  consec_consonants--; conresets--;}
			cptr++;
							}


		/* 4 vowels in a row is normally somebody typing junk for a screenname */
		if (consec_vowels>4) {return 1;}
		if (conresets>1) {return 1;}



		/* 5 consonants in a row is normally somebody typing junk for a screenname */
		if ( (conresets>0) &&  (consec_consonants>2)) {return 1;}

		if (looks_bottish) {
			if (consec_vowels>3) {return 1;}
								}


		/* Just in case, avoid possible division by zero error */
		if ( (slimit-underscores)<1 )  {return 0;}

		/* vowels should make up no less than 20% of any word */
		percentage=(float) (((float)num_vowels/(float) (slimit-underscores) ) * 100.0);
		/* printf("percentage: %f\n",percentage); fflush(stdout); */

		if ( percentage >= 21.5) {
			if (! looks_bottish) { return 0;}
			else  {
				if (percentage >= 23.5) { return 0; }
					}
											}

		/* 4 consonants in a row is normally somebody typing junk for a screenname */
		if (conresets>0) {return 1;}
		if ( percentage >= 20.0) {return 0;}
		if (consec_consonants>3) {return 1;}
		if (looks_bottish) {return 1; }
		if ( percentage <  19.5) {return 1;}
		}

return 0;
}


int is_valid_system_message(char *who) {
	/*
	This method checks the validity of yahoo 'system'
	messages, packet 0x14 (20) which spam bots have
	been using lately to send system messages as PMs,
	because they know most Yahoo clients will always 
	let system messages through unchecked. Yahoo 
	REALLY needs to disallow random users from sending
	'system' messages; Valid system messages usually
	have 'SYSTEM' set as the name in field 4, or less often,
	something like 'YahooMessengerTeam'
	*/ 
	if (!who) {return 0;}
	if (!enable_chat_spam_filter) {return 1;}
	if (! strcasecmp(who, "yahoomessengerteam") ) {return 1; }  /* y! sys message */
	if (! strcasecmp(who, "system") ) {return 1; }  /* y! sys message */
		/* This might still let a few bogus msgs in, but just
		to be sure we dont miss any system messages..., 
		Allow it in if the first 5 letters are 'yahoo' and it
		does not appear to be a spam bot name  */
	if (! strncasecmp(who, "yahoo", 5) ) {
		if (is_spam_name(who)) {return 0;}
		return 1;
	}  
		/* allow ourselves and our friends, though they 
		really shouldn't be sending system messages */
	if (!strcasecmp(who,ymsg_sess->user))   {return 1;}	
	if (find_profile_name(who))  {return 1; }
	if (find_friend(who)) {return 1;}
	if (find_temporary_friend(who))  {return 1;}
	return 0;
}


int is_valid_game_sys_msg_content(char *who, char *content) {
	/*
	This method checks the validity of the CONTENT
	of yahoo 'system' and 'game' messages: Particularly,
	these messages should NOT contain things such 
	as <FADE or <ALT tags or color escape sequences
	such as \033[#C81447m.  Spam bots are now 
	using a serious of color escape sequences to 
	similate 'faders' when sending system and game
	messages that contain spam.  Official system 
	messages from Yahoo do not contain exotic
	color tags, and users truly sending messages 
	from the yahoo 'games' area do not have access
	to things like faders and color tags in the Java-based
	IM window used in that area.  So if we are getting 
	really fancy formatting in these messages, chances are
	it is spam. The same is true for the older PM messages
	using packet 'YMSG_PM_RECV' instead of 'YMSG_PM':
	Usually only spam bots use 'YMSG_PM_RECV', this 
	older PM packet was not intended to carry exotic 
	formatting tags.
	*/ 
	if (!who) {return 0;}
	if (!content) {return 0;}
	if (!enable_chat_spam_filter) {return 1;}
	if (! strcasecmp(who, "yahoomessengerteam") ) {return 1; }  /* y! sys message */
	if (! strcasecmp(who, "system") ) {return 1; }  /* y! sys message */
	if (!strcasecmp(who,ymsg_sess->user))   {return 1;}	
	if (find_profile_name(who))  {return 1; }
	if (find_friend(who)) {return 1;}
	if (find_temporary_friend(who))  {return 1;}
	if ( strstr(content,"\033[#") )  {return 0;}
	if ( strstr(content,"<FADE") || strstr(content,"<ALT"))  {return 0;}
	if ( strstr(content,"<fade") || strstr(content,"<alt"))  {return 0;}
	if ( strstr(content,"<Fade") || strstr(content,"<Alt"))  {return 0;}
	if (is_spam_name(who)) {return 0;}
	return 1;
}


int is_chat_spam(char *tmpstr, char *tmpstr2)   {
		static char spam_buff[296];


	if (!enable_chat_spam_filter) {return 0;}
	if (! strcasecmp(tmpstr, "yahoomessengerteam") ) {return 0; }  /* y! sys message */
	if (! strcasecmp(tmpstr, "system") ) {return 0; }  /* y! sys message */
	if (!strcasecmp(tmpstr,ymsg_sess->user))   {return 0;}	
	if (find_profile_name(tmpstr))  {return 0; }
	if (find_friend(tmpstr)) {return 0;}
	if (find_temporary_friend(tmpstr))  {return 0;}

	/* never flag Gyach Enhanced auto-responses as spam */
	if (strstr(tmpstr2, "GYACH ENHANCED Automated Response")) {return 0;}
	if (strstr(tmpstr2, "PhrozenSmoke's GYach Enhanced")) {return 0;}
	if (strstr(tmpstr2, "http://phpaint.sourceforge.net")) {return 0;}


	/* mal-formed fader, alt-tags usually come from bots, ex. <#990099> */
	if (strstr(tmpstr2, "<#"))  	  {return 1;}  

	/* mal-formed font tag, usually from spam bots */
	if (strstr(tmpstr2, "<font YHLT"))  	  {return 1;}  
	if (strstr(tmpstr2, "<font size=\"4>"))  	  {return 1;}  


	if (enable_chat_spam_filter) {
	char **cptr = spam_comments;
	char **sprobe = spam_probe;
	memset( spam_buff, 0, sizeof( spam_buff ));
	snprintf(spam_buff, 294, "%s", tmpstr2);  /* Should be enough sample bytes to examine */
	strip_html_tags(spam_buff);
	lower_str( spam_buff );

	/* lines almost always from spam bots: http://myaccount@nastysite.com */
	/* The most common and easiest to spot */

	if ( strstr(spam_buff, "http:")  &&  strstr(tmpstr2, "@") )  	{return 1;}


	/* some spam bots are kind enough to put [SPAMBOT] or similar in their postings */
	/* Check for blatant Spamming first */

		cptr = spam_bot_flags;
		while( *cptr ) {
			if ( strstr(spam_buff,*cptr )) { return 1; }
		    cptr++;
							}		


				/* lines likely to be from spam bots, containing 'http' and 'webcam' etc. */
				if ( strstr(spam_buff, "http:") || 
					((strstr(spam_buff, ".com") || strstr(spam_buff, ".net")) && (!strstr(spam_buff, "@")) )
					 ) 			{
						cptr = spam_comments;
						while( *cptr ) {
							if (strcmp("http:",*cptr) && 
							strcmp("www.",*cptr) ) {	
								if ( strstr(spam_buff,*cptr )) { return 1; }
												  			  }
						cptr++;
										  }		
								  					    				}


				/* 'view my explicit pics', etc. */
		while( *sprobe ) {

				if (strstr(spam_buff, *sprobe) )  				{
						cptr = spam_comments;
						while( *cptr ) {
							if (strcmp(*sprobe,*cptr) ) {	
								if ( strstr(spam_buff,*cptr )) { return 1; }
												  			        }
						cptr++;
										    }						

						cptr = spam_comments2;
						while( *cptr ) {
							if (strcmp(*sprobe,*cptr) ) {	
								if ( strstr(spam_buff,*cptr )) { return 1; }
												  			        }
						cptr++;
										    }			
	 													     					}
			sprobe++;
					} /* end while sprobe */					


	/* check for junk text in first part of string like: '302 87l21 1w8j8 m4463 4knq8'	*/
	if (strlen(spam_buff) >15) {
		char junk_buf[32];
		int char_next_to_num=0;
		int letters_found=0;
		int numbers_found=0;
		int spaces_found=0;
		int istart=0;
		int limited=0;
		limited=snprintf(junk_buf, 30, "%s", spam_buff);
		while (istart<limited) {
			if (junk_buf[istart]=='\0') {break;}
			if (junk_buf[istart]==' ') {spaces_found++; istart++; continue;}
			if (! isdigit(junk_buf[istart])) { /* letter */
				if (istart>1) {
					if (isdigit(junk_buf[istart-1])) {char_next_to_num++;}
				}
				letters_found++; istart++; continue;
			} else { /* number */
				if (istart>1) {
					if ((! isdigit(junk_buf[istart-1])) && (junk_buf[istart-1] != ' ')) {char_next_to_num++;}
				}
				numbers_found++; istart++; continue;
			}
			istart++;
		}
		if (char_next_to_num>5) {return 1;}
		if ( (char_next_to_num>2) && strstr(spam_buff,"http:") ) {return 1;}
		if ( (char_next_to_num>1) && (numbers_found>2) && 
			(letters_found>2) && (spaces_found>1) ) { return 1;}
	}
					  

										} /* end if enable_spam_filter */
return 0;
}


