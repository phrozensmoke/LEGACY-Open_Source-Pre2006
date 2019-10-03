/*****************************************************************************
 * yahoochat.h
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
 * cpinkham@infi.net, cpinkham@bc2va.org
 * http://www4.infi.net/~cpinkham/gyach/
 *****************************************************************************/

#ifndef _YAHOOCHAT_H_
#define _YAHOOCHAT_H_

#include <sys/types.h>

#define YMSG_USER_SIZE		64
#define YMSG_PASSWORD_SIZE	32
#define YMSG_ROOM_SIZE		64
#define YMSG_HOST_SIZE		128
#define YMSG_COOKIE_SIZE	1024
#define YMSG_DATA_SIZE		2048

#ifndef INADDR_NONE
#define INADDR_NONE             ((unsigned long int) 0xffffffff)
#endif

typedef struct YMSG_PACKET_STRUCT {
	int		type;								/* packet type */
	int		size;								/* data length */
	char	data[YMSG_DATA_SIZE+1];			/* packet data */
} YMSG_PACKET;

typedef struct YMSG_SESSION_STRUCT {
	int		sock;								/* network socket */
	int		quit;								/* true if should quit */
	char	user[YMSG_USER_SIZE+1];			/* login */
	char	password[YMSG_PASSWORD_SIZE+1];	/* password */
	char	room[YMSG_ROOM_SIZE+1];			/* current room */
	char	req_room[YMSG_ROOM_SIZE+1];			/* requested room */
	char	host[YMSG_HOST_SIZE+1];			/* hostname to connect to */
	int		port;								/* port to connect to */
	char	proxy_host[YMSG_HOST_SIZE+1];		/* hostname to connect to */
	int		proxy_port;							/* port to connect to */
	char	cookie[YMSG_COOKIE_SIZE+1];		/* cookie(s) */
	int		session_id;							/* session id */
	int		io_callback_tag;					/* input callback tag */
	int		ping_callback_tag;					/* ping callback tag */
	YMSG_PACKET	pkt;						/* data packet */
	YMSG_PACKET	last_pkt;					/* last data packet */
	int		suppress_dup_packets;				/* sup dup consecutive pkts */
	char	error_msg[129];						/* error message if needed */
	int		debug_packets;						/* debug in/out packets */
} YMSG_SESSION;

extern char *away_msgs[];

extern int login_name_changed;
extern char *verify_passphrase;
extern char *last_credentials_key;
extern int sharing_bimages;

extern void show_yahoo_packet();
extern void handle_dead_connection (int broken_pipe, int disable_reconn, int disable_dialog, char *reason);
extern int show_cam_is_on;
extern int show_radio_is_on;
extern int in_a_chat;
extern char *selected_buddy_group;

#define YMSG_CHAT_PORT		5050

#define YMSG_SEP			"\xC0\x80"

#define YMSG_BUDDY_ON		0x01
#define YMSG_BUDDY_OFF		0x02
#define YMSG_AWAY			0x03
#define YMSG_BACK			0x04
#define YMSG_PM			0x06
#define YMSG_MAILSTAT		0x09
#define YMSG_MAIL			0x0b
#define YMSG_NEW_CONTACT	0x0f
#define YMSG_PM_RECV		0x20
#define YMSG_GAMES_ENTER		0x28  /* added: PhrozenSmoke */
#define YMSG_GAMES_LEAVE		0x29  /* added: PhrozenSmoke */
#define YMSG_FILETRANSFER	0x46
#define YMSG_VOICECHAT		0x4a
#define YMSG_NOTIFY		0x4b
#define YMSG_P2P		0x4d  /* peer-to-peer file is coming */ /* added: PhrozenSmoke */
#define YMSG_P2PASK		0x4f   /*asks can we do peer-to-peer */ /* added: PhrozenSmoke */
/* added: PhrozenSmoke, web cam invites */
#define YMSG_WEBCAM	0x50  
#define YMSG_LOGIN 	  	0x54
#define YMSG_COOKIE 	  	0x55
#define YMSG_GET_KEY   	0x57
#define YMSG_ADD_BUDDY		0x83
#define YMSG_REM_BUDDY		0x84
#define  YMSG_REJECTBUDDY	  0x86
#define YMSG_ONLINE		0x96
#define YMSG_GOTO			0x97
#define YMSG_JOIN			0x98
#define YMSG_EXIT			0x9b
#define YMSG_INVITE		0x9d
#define YMSG_LOGOUT		0xa0
#define YMSG_PING			0xa1   /* Chat room pings */
#define YMSG_COMMENT		0xa8
  /* added: PhrozenSmoke, other packets */
#define YMSG_CREATE_ROOM	0xa9
#define YMSG_GAMEMSG	0x2a      /* Messages coming from Y! Games */
#define YMSG_SYSMSG		0x14       /* Y! system messages */
#define YMSG_MSGRPING		0x12       /* Y! messenger pings */
#define YMSG_CALENDAR		0xd       /* Y! calendar events */
#define YMSG_NOTICIAS		0x12c       /* news alerts from alerts.yahoo.com */
#define YMSG_IDDEACT		0x08       /* buddy ID deactivated, logoff */
#define YMSG_IDACT		0x07       /* buddy ID activated, logon */
#define YMSG_USERSTAT		0xa       /* user status, buddy list refresh  */
#define YMSG_ADDIGG		0x11       /* added perm ignore  */
#define YMSG_TOGGLEIGG		0x85       /* toggled perm ignore  */
#define YMSG_PASSTHRU		0x16       /* server passthrough  */

/* Internally used YMSG packet types */
#define YMSG_GYE_EXIT	0x37  /* Force disconnect from Yahoo */
#define YMSG_GYE_CONFEXIT	0x38  /* Force disconnect from conference */

#define YMSG_STATUS_TYPING		0x16 

/* Conference stuff, added: PhrozenSmoke  */
#define	 YMSG_CONFINVITE	0x18
#define	 YMSG_CONFLOGON		0x19
#define	 YMSG_CONFDECLINE	0x1a
#define	 YMSG_CONFLOGOFF	0x1b
#define	 YMSG_CONFADDINVITE		0x1c
#define	 YMSG_CONFMSG		0x1d

#define YMSG_GROUP_RENAME 0x89
#define YMSG_GOTGROUP_RENAME 0x13


/* Packets being used in YMSG-6 (beta) */

#define YMSG_VERIFY		0x4c
#define YMSG_STEALTH		0xba  /* stealth invisibility */
#define YMSG_STEALTH_PERM		0xb9  /* stealth invisibility */
#define YMSG_SETTINGS	0x15  /* on windows, Y! tells us which tab to show on messenger */
#define YMSG_BUDDY_STATUS	0xc6  /* all-purpose status packets */
#define YMSG_GAME_INVITE	0xb7  /* game invitation */
#define YMSG_AUDIBLE	0xd0  /*  audibles  */
#define YMSG_VISIBILITY	0xc5  /*  toggle visibility  */
#define YMSG_SIGNOFF	0xc0  /*  logoff messenger  */

#define YMSG_BIMAGE	0xbe  /* buddy display image */
#define YMSG_BIMAGE_UPDATED	0xbd  /* a buddy image has changed */
#define YMSG_BIMAGE_SEND	0xc2  /* uploading buddy image */
#define YMSG_BIMAGE_TOGGLE	0xc1 /* toggle permissions for showing buddy images */
#define YMSG_AVATAR_UPDATED	0xbc  /* we or somebody else update an avatar */
#define YMSG_AVATAR_TOGGLE	0xc7  /* toggle permissions for showing avatars */

int ymsg_upload_bimage(YMSG_SESSION *session, char *filename);
int ymsg_bimage_update(YMSG_SESSION *session, char *who,char *b_hash);
int ymsg_bimage_notify(YMSG_SESSION *session, char *who, char *b_url, char *b_hash);
int ymsg_bimage_accept(YMSG_SESSION *session, char *who, int bimage);
int ymsg_bimage_toggle(YMSG_SESSION *session, char *who, int bimage);
int ymsg_avatar_toggle(YMSG_SESSION *session, int avatar_on);


int ymsg_open_socket( YMSG_SESSION *session );
int ymsg_recv_data( YMSG_SESSION *session );
int ymsg_request_key( YMSG_SESSION *session );
int ymsg_login( YMSG_SESSION *session, char *key );
int ymsg_online( YMSG_SESSION *session );
int ymsg_join( YMSG_SESSION *session );
int ymsg_comment( YMSG_SESSION *session, char *text );
int ymsg_emote( YMSG_SESSION *session, char *text );
int ymsg_think( YMSG_SESSION *session, char *text );
int ymsg_logout( YMSG_SESSION *session );
int ymsg_ping( YMSG_SESSION *session );
int ymsg_away( YMSG_SESSION *session, char *msg );
int ymsg_back( YMSG_SESSION *session );
int ymsg_pm( YMSG_SESSION *session, char *remote_user, char *msg );
int ymsg_add_buddy( YMSG_SESSION *session, char *friend );
int ymsg_remove_buddy( YMSG_SESSION *session, char *friend );
int ymsg_goto( YMSG_SESSION *session, char *friend );
int ymsg_invite( YMSG_SESSION *session, char *remote_user, char *room );
int ymsg_typing( YMSG_SESSION *session, char *remote_user, int typing );
int ymsg_send_file( YMSG_SESSION *session, char *who, char *msg, char *filename );

/* added: PhrozenSmoke */
int ymsg_create_room( YMSG_SESSION *session, char *category, char *room, char *topic, char *attribs );
int ymsg_p2p_ack( YMSG_SESSION *session, char *who );
int ymsg_group_rename( YMSG_SESSION *session, char *ngroup );

int ymsg_conference_join( YMSG_SESSION *session);
int ymsg_conference_msg( YMSG_SESSION *session, char *msg);
int ymsg_conference_decline( YMSG_SESSION *session, char *room, char *who_list, char *msg);
int ymsg_conference_invite( YMSG_SESSION *session, char *who, char *msg);
int ymsg_conference_leave( YMSG_SESSION *session);
int ymsg_conference_msg_echo( YMSG_SESSION *session);

int ymsg_reject_buddy( YMSG_SESSION *session, char *who );
int ymsg_refresh( YMSG_SESSION *session);

int ymsg_passthrough( YMSG_SESSION *session );
int ymsg_p2p_redirect( YMSG_SESSION *session, char *who, char *short_file, char *long_file ) ;
int ymsg_leave_chat( YMSG_SESSION *session );
int ymsg_messenger_logout( YMSG_SESSION *session );
int ymsg_contact_info( YMSG_SESSION *session, char *remote_user, char *info );
int ymsg_invite_with_mesg( YMSG_SESSION *session, char *remote_user, char *room, char *mesg );
int ymsg_messenger_ping( YMSG_SESSION *session );
int ymsg_perm_ignore( YMSG_SESSION *session, char *who, int remove_ignore );
int ymsg_web_login( YMSG_SESSION *session, char *cookie);
int ymsg_webcam_invite_reject( YMSG_SESSION *session, char *who);
int ymsg_invite_with_mesg_extended( YMSG_SESSION *session, char *remote_user, char *room, char *msg );
int ymsg_voice_invite( YMSG_SESSION *session, char *who, char *room );
int ymsg_imvironment( YMSG_SESSION *session, char *who, char *imvironment);
int ymsg_pm_full( YMSG_SESSION *session, char *remote_user, char *msg, char *imvironment );



int ymsg_stealth_perm( YMSG_SESSION *session, char *who, int visible );
int ymsg_stealth( YMSG_SESSION *session, char *who, int visible );
int ymsg_get_key( YMSG_SESSION *session );
int ymsg_request_login( YMSG_SESSION *session );
int ymsg_audible(YMSG_SESSION *session, char *pm_user, char *aud_file, char *aud_text, char  *aud_hash);
int ymsg_game_invite(YMSG_SESSION *session, char *who, char *game, int invite);


char *get_random_conference_name( YMSG_SESSION *session);
void check_proxy_config();

#endif /* #ifndef _YAHOOCHAT_H_ */
