/*****************************************************************************
 * ycht.c
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
 * Phrozensmoke ['at'] yahoo.com
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <ctype.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <malloc.h>
#include <netdb.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "gyach.h"
#include "yahoochat.h"
#include "users.h"
#include "friends.h"
#include "util.h"
#include "bootprevent.h"
#include "webconnect.h"
#include "profname.h"
#include "ycht.h"
#include "callbacks.h"


char ycht_host[64];
int ycht_port=8002;
int ycht_sock=-1;
int ycht_only=0;
int ycht_now_online=0;
char *ycht_buf=NULL;
char *ycht_sn=NULL;

/* from yahoochat.c */
extern u_char yrecvbuf[2048];
extern u_char ysendbuf[5120];  
extern u_char ypingbuf[128];
extern int without_yreset;
extern void reset_yping_buf();
extern void reset_yrecv_buf();
extern void reset_ysend_buf();


int ycht_is_running() {
	if (using_web_login) {
		if (ycht_sock != -1 ) {return 1;}
	}
	return 0;
}

void ycht_set_host(char *myhost) {
	snprintf(ycht_host,62,"%s", myhost?myhost:"jcs.chat.dcn.yahoo.com");
}

int ycht_open_socket( ) {
	struct sockaddr_in addr;
	struct hostent *hinfo;
	int sock = -1;

	/* initialize the session vars */
	ycht_sock = -1;
    	memset(&addr ,0 ,sizeof(addr)); 	
	reset_ysend_buf();
	reset_yrecv_buf();
	check_proxy_config();	

	/* figure out the hosts address */
	if ( ymsg_sess->proxy_host[0] && use_proxy ) {
	    addr.sin_addr.s_addr = inet_addr( ymsg_sess->proxy_host ); 
	} else {
	    addr.sin_addr.s_addr = inet_addr( ycht_host ); 
	}
 
    if ( addr.sin_addr.s_addr == (u_int)INADDR_NONE ) {
	set_socket_timer(sock, -1);
	set_socket_timer(sock, 12);

		if ( ymsg_sess->proxy_host[0] && use_proxy ) {
			hinfo = gethostbyname( ymsg_sess->proxy_host ); 
		} else {
			hinfo = gethostbyname( ycht_host ); 
		}

	set_socket_timer(sock, -1);
 
        if (( !hinfo ) ||
			( hinfo->h_addrtype != AF_INET )) {
			if ( ymsg_sess->proxy_host[0] && use_proxy ) {
				snprintf( ymsg_sess->error_msg, 127, 
					"Could not resolve DNS address ( %s ): %d",
					ymsg_sess->proxy_host, errno );
			} else {
				snprintf( ymsg_sess->error_msg,127, 
					"Could not resolve DNS address ( %s ): %d",
					ycht_host, errno );
			}
            return 0; 
        } 
 
        memset( &addr, 0, sizeof(addr)); 
        memcpy((void*)&addr.sin_addr.s_addr, hinfo->h_addr, hinfo->h_length ); 
    } 

    addr.sin_family = AF_INET; 

	/* set the hosts port */
	if ( ymsg_sess->proxy_host[0] && use_proxy) {
	    addr.sin_port = htons( ymsg_sess->proxy_port ); 
	} else {
	    addr.sin_port = htons( ycht_port ); 
	}

	/* create a socket() to connect with and connect() */
    sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ); 
    set_socket_timer(sock,10);
    if ( sock == -1 ) {
		snprintf( ymsg_sess->error_msg, 127, "Socket connection failed: %d", errno );
		 set_socket_timer(sock,-1);
		return( 0 );
	} else if ( connect( sock, (struct sockaddr*)&addr, sizeof(addr)) == -1 ) {
		snprintf( ymsg_sess->error_msg, 127, "Socket connection failed: %d", errno );
		 set_socket_timer(sock,-1);
        	close( sock ); 
		return( 0 );
    } 

    set_socket_timer(sock,-1);

	/* if we are using a proxy host, send the CONNECT string */
	if ( ymsg_sess->proxy_host[0] && use_proxy) {		
		char buf[416];
		char tmp[128];

			if ( capture_fp) {	
				time_t time_llnow = time(NULL);
				fprintf(capture_fp, "\n%s\nPROXY SETTINGS (ycht_open_socket):\nProxy Host: %s\nPort: %d\n\n", ctime(&time_llnow), proxy_host, proxy_port);
				fflush( capture_fp );
			}

		snprintf( buf, 414, "CONNECT %s:%d HTTP/1.1\r\nHost: %s:%d\r\n\r\n", ycht_host,
			ycht_port ,ycht_host, ycht_port  );

		set_socket_timer(sock,10);
		write( sock, buf, strlen( buf ));

			if ( capture_fp) {	
				time_t time_llnow = time(NULL);
				fprintf(capture_fp, "\n%s\nPROXY CONNECT HTTP Data Sent:\nData: %s\n\n", ctime(&time_llnow), buf);
				fflush( capture_fp );
			}

		/* get the response from the proxy */
		set_socket_timer(sock,-1);
		set_socket_timer(sock,20);
		read( sock, tmp, sizeof( tmp ) - 1 );
		set_socket_timer(sock,-1);

			if ( capture_fp) {	
				time_t time_llnow = time(NULL);
				fprintf(capture_fp, "\n%s\nPROXY CONNECT-Server Reply:\nData: %s\n\n", ctime(&time_llnow), tmp);
				fflush( capture_fp );
			}

		if ( (! strstr( tmp, "OK" )) && (! strstr( tmp, "200 " )) ) {
			char *mybr=strchr(tmp,'\n');
			if (mybr) {*mybr='\0';}
			mybr=strchr(tmp,'\r');
			if (mybr) {*mybr='\0';}

			/* proxy failed */
			snprintf( ymsg_sess->error_msg,127, 
				"Error connecting via proxy.  Proxy replied: '%s'", tmp );
			close( sock );
			return( 0 );
		}
	}

	/* success so set our session socket */
	ycht_sock = sock;

	/* sock will be positive here, so return as true */
    return( sock ); 
}

int ycht_sock_send( u_char *buf, int size ) {
	int sent;
	char *ptr = buf;

			if ( capture_fp) {	
				time_t time_llnow = time(NULL);
				int pcounter=0;
				fprintf(capture_fp, "\n%s\nOUTGOING PACKET SENT:\nSize: %d\nData: [", ctime(&time_llnow), size);
				while (pcounter<size){
					if (ptr[pcounter]) {fprintf(capture_fp, "%c", ptr[pcounter]);}
					pcounter++;
				}
				fprintf(capture_fp, "]\n\n");
				fflush( capture_fp );
			}

	while( size ) {
		sent = send( ycht_sock, ptr, size, 0 );
		if ( sent < 1 ) {
			if ((errno != EAGAIN) && (errno != EINTR)) {
				snprintf( ymsg_sess->error_msg, 127, "Error sending to socket: %d",errno );
				ycht_logout(0);
			}
			break;
		}
		ptr += sent;
		size -= sent;
	}
	if (!without_yreset) {reset_ysend_buf();}  /* clear the send buffer */
	return( size == 0 );
}


int ycht_sock_recv(  u_char *buf, int size ) {
	int recvd;
	char *ptr = buf;

	while( size ) {
		recvd = recv( ycht_sock, ptr, size, 0 );
		if ( recvd < 1 ) {
			if ((errno != EAGAIN) && (errno != EINTR)) {
				snprintf(ymsg_sess->error_msg,127,"Error receiving from socket: %d\n", errno );
				ycht_logout(0);
			}
			break;
		}
		ptr += recvd;
		size -= recvd;
	}
	return( size == 0 );
}

u_char *ycht_header( u_char *buf, u_long pkt_type ) {
	u_char *ptr = buf;
	memcpy( buf, "YCHT", 4 );					ptr += 4;  /* msg procotol */
   	*((u_long*)(buf+ 4))=htonl(0x000000ae); ptr += 4; 
   	 *((u_long*)(buf+ 8))=htonl(pkt_type); ptr += 4;  
   	ptr += 4;  	
	return( ptr );
}

int ycht_send_packet( u_char *bufp, int size ) {
	/* set data length in packet */
	*((u_short *)(bufp+14)) = htons( size - 16 );
	/* don't try to send if we're not connected */
	if ( ycht_sock == -1 ) {return( 0 );}
	return ycht_sock_send( bufp, size );
}

int ycht_recv_data( ) {
	reset_yrecv_buf();
	if ( ! ycht_sock_recv( yrecvbuf, 16 )) {	return( 0 );}
	ymsg_sess->pkt.size = ntohs((u_short)*((u_short*)(yrecvbuf+14)));

	if ( ! ycht_sock_recv(  yrecvbuf + 16, ymsg_sess->pkt.size )) {
		return( 0 );
	}

	ymsg_sess->pkt.type = ntohs((u_short)*((u_short*)(yrecvbuf+10)));

	if ( ymsg_sess->debug_packets ) { 
		printf("YCHT-packet type: %d\n", ymsg_sess->pkt.type); fflush(stdout);
		printf("YCHT-packet data: %s\n", yrecvbuf + 16); fflush(stdout);
	}

	/* some flow control to avoid booting, added PhrozenSmoke */
	sock_sleep=10000;
	if (enable_basic_protection) 	{
		int is_dangerous=1;  /* Medium security by default */
		int very_dangerous=0;
		switch (ymsg_sess->pkt.type) {
			case 0x41: /* comment */
			case 0x43: /* emote */
			case 0x42: /* think */
				is_dangerous=0;
				break;
			case 0x45:  /* PM */
				very_dangerous=1;
				break;
		}								

	if (ymsg_sess->pkt.size>800 && is_dangerous) {
		sock_sleep=60000;
		if (very_dangerous) {
			/* ymsg_messenger_ping(ycht_sess);  */ 
			sock_sleep=75000;
			}
																}
	/* normally these are excessively large conference declines or other 'boot' packets */
	if (ymsg_sess->pkt.size>=950) {
		sock_sleep=75000;
		if (very_dangerous)			 {
			/* ymsg_messenger_ping(ycht_sess);  */ 
			sock_sleep=100000;
												}
										}
	}

	yrecvbuf[ymsg_sess->pkt.size + 16] = '\0';
	memcpy( ymsg_sess->pkt.data, yrecvbuf + 16, ymsg_sess->pkt.size );
	ymsg_sess->pkt.data[ ymsg_sess->pkt.size ] = '\0';
	check_packet_suppression();

	/* if suppress duplicate consecutive packets is on then check */
	if (( ymsg_sess->suppress_dup_packets ) &&
		( ! memcmp( &ymsg_sess->last_pkt, &ymsg_sess->pkt, sizeof( ymsg_sess->pkt )))) {
		return( 0 );
	}

	memcpy( &ymsg_sess->last_pkt, &ymsg_sess->pkt, sizeof( ymsg_sess->pkt ));
	return( 1 );
}


int ycht_ping( ) {
	u_char *ptr = ycht_header(  ypingbuf, 0x62 );
	int len;
	snprintf( ptr, 3, "%s", "");
	len = 16 + strlen( ptr );
	without_yreset=1;
	ycht_send_packet(  ypingbuf, len );
	reset_yping_buf();
	without_yreset=0;
	return( 0 );
}

int ycht_join( ) {
	char extra[8];
	int len;
	u_char *ptr =NULL;	
	if (! ycht_now_online) {return ycht_online( );}
	reset_ysend_buf();
	ptr= ycht_header(  ysendbuf, 0x11);
	sprintf(extra,"%s", "");
	if (! strstr(ymsg_sess->req_room,":")) {snprintf(extra,6, "%s", ":1");}
	snprintf( ptr, 256, "%s%s", ymsg_sess->req_room, extra );  
	len = 16 + strlen( ptr );
	ycht_send_packet(  ysendbuf, len );	
	return( 0 );
}

int ycht_goto(char *who ) {
	u_char *ptr =NULL;	
	int len;
	reset_ysend_buf();
	ptr= ycht_header(  ysendbuf, 0x25);
	snprintf( ptr, 200, "%s", who );  
	len = 16 + strlen( ptr );
	ycht_send_packet(  ysendbuf, len );	
	return( 0 );
}

int ycht_online( ) {
	char *cbreak=NULL;
	char *cookies=NULL;
	char *cookies1=NULL;
	int appended=0;
	char meme[64];
	static char cookbox[720];
	u_char *ptr = NULL;
	int len;
	if (ycht_sock== -1) {
		if (ycht_open_socket( )==-1) { 
			show_ok_dialog(ymsg_sess->error_msg);
			return 0;
		}
	}
	if (ycht_sock== -1) {return 0;}
	reset_ysend_buf();
	ptr=ycht_header(  ysendbuf, 0x01);
	if (! ycht_sn) {ycht_sn=strdup(ycht_only?ymsg_sess->user:select_profile_name(YMSG_JOIN, 0)); }
	snprintf(meme, 62, "%s",  ycht_sn);
	lower_str(meme);
	sprintf(cookbox,"%s", "");
	cookies1=strdup(ymsg_sess->cookie);
	cookies=cookies1;
		if ( ymsg_sess->debug_packets ) { printf("cookies-in: %s\n", cookies ); fflush(stdout); }
	cbreak = strchr(cookies,' ');
	while (cbreak) {
		*cbreak='\0';
		if ( ((! strncmp(cookies, "Y=", 2)) && (!strstr(cookbox,"Y="))) || 
			((! strncmp(cookies, "T=", 2)) && (!strstr(cookbox,"T=")))
		) {
			if (appended) {strcat(cookbox, " "); strncat(cookbox, cookies, 350 );}
			else {  strncat(cookbox, cookies, 350 );}
			appended=1;
		}
		cbreak++;
		cookies=cbreak;
		cbreak = strchr(cookies,' ');
	}
		if ( ((! strncmp(cookies, "Y=", 2)) && (!strstr(cookbox,"Y="))) || 
			((! strncmp(cookies, "T=", 2)) && (!strstr(cookbox,"T=")))
		) {
			if (appended) {strcat(cookbox, " "); strncat(cookbox, cookies, 350 );}
			else {  strncat(cookbox, cookies, 350 );}
			 strcat(cookbox, ";");
		}
	cbreak=strrchr(cookbox,';');
	if (cbreak) {*cbreak='\0';}
			if ( ymsg_sess->debug_packets ) { 
				printf("cookies-out: %s\n", cookbox ); fflush(stdout);
			} 
	snprintf( ptr, 840, "%s%c%s", meme, 0x01, cookbox );  
	len = 16 + strlen( ptr );
	ycht_send_packet(  ysendbuf, len );	
	if (cookies1) {free(cookies1);}
	return( 0 );
}

int ycht_leave( ) {
	char meme[64];
	int len;
	u_char *ptr =NULL;	
	reset_ysend_buf();
	ptr= ycht_header(  ysendbuf, 0x02);
	if (! ycht_sn) {ycht_sn=strdup(select_profile_name(YMSG_EXIT, 0)); }
	snprintf(meme, 62, "%s", ycht_sn );
	lower_str(meme);
	snprintf( ptr, 128, "%s", meme );  
	len = 16 + strlen( ptr );
	ycht_send_packet(  ysendbuf, len );	
	return( 0 );
}

int ycht_logout(int quiet) {
	if (ycht_sock != -1) {
		ycht_leave( );
		close(ycht_sock);
		ycht_sock= -1;
		if (ycht_sn) {chatter_list_remove( ycht_sn );}
		clear_chat_list_members();
		if (! quiet) {
			ymsg_sess->pkt.type=YMSG_LOGOUT;
			handle_yahoo_packet();
		}
		if (ycht_only && (! quiet)) {
			ymsg_sess->pkt.type=YMSG_GYE_EXIT;
			handle_yahoo_packet();
		}
	}
	ycht_now_online=0;
	ycht_only=0;
	if (ycht_buf) {free(ycht_buf); ycht_buf=NULL;}
	
	if (ycht_sn) { free(ycht_sn); ycht_sn=NULL;}
	return 0;
}

int ycht_comment(  char *comment, int comtype ) {
	int len;
	u_char *ptr=NULL; 
	reset_ysend_buf();
	if ( (comtype==1) || (comtype==2) ) {
		ptr=ycht_header(  ysendbuf, 0x43);
	} else {ptr=ycht_header(  ysendbuf, 0x41);}

	snprintf( ptr, 1800, "%s%c%s", ymsg_sess->room, 0x01, comment	);

	if (comtype==1) { /* emote */
	snprintf( ptr, 1800, "%s%c<FADE #00AABB,#3333bb><font face=\"tahoma\"  size=\"16\">%s</FADE>", ymsg_sess->room, 0x01, comment	);
	}
	if (comtype==2) { /* think */ 
	snprintf( ptr, 1800, "%s%c<FADE #00AABB,#3333bb><font face=\"tahoma\" size=\"16\">. o O ( %s )</FADE>", ymsg_sess->room, 0x01, comment	);
	}

	len = 16 + strlen( ptr );
	ycht_send_packet(  ysendbuf, len );	
	return( 0 );
}



int ycht_pm( char *remote_user, char *msg) {
	int len;
	u_char *ptr =NULL;	
	reset_ysend_buf();
	ptr= ycht_header(  ysendbuf, 0x45);
	snprintf( ptr, 1500, "%s%c%s%c%s%c-1:-1", ycht_sn, 0x01, remote_user, 0x01, msg?msg:" ", 		0x01);  
	len = 16 + strlen( ptr );
	ycht_send_packet(  ysendbuf, len );	
	return( 0 );
}

void ych_hash(int fielder, char *fieldval) {
	char mynum[8];
	if ( ymsg_sess->debug_packets ) { 
		printf("hashed:  %d - %s\n", fielder, fieldval); 
		fflush(stdout);
	}
	snprintf(mynum,6,"%d", fielder);
	strcat(ycht_buf, mynum);	
	strcat(ycht_buf, YMSG_SEP);	
	strcat(ycht_buf, fieldval);	
	strcat(ycht_buf, YMSG_SEP);	
	free(fieldval);
}

/*  
	This method basically takes YCHT packets, converts them 
	to YMSG packets, then sends the YMSG packet to 
	handle_yahoo_packet() for consistency of handling
*/ 
void handle_ycht_packet() {
	int org_pm_setting=0;
	int handled=0;
	int orgpkt=0;

		/* logout */  /*  */  
		if (ymsg_sess->pkt.type== 0x02) { 
			ycht_logout(0);
			return;
		} 

		if (ymsg_sess->pkt.type== 0x01) { /* online */ 
			ycht_now_online=1;
			if (ycht_only) {
				ymsg_sess->pkt.type=YMSG_GET_KEY;
				handle_yahoo_packet();
			}
			ycht_join();
			return;
		}

	if (!ycht_buf) {ycht_buf=malloc(2048);}
	if (!ycht_buf) {return;}
	sprintf(ycht_buf,"%s","");
	org_pm_setting=pm_in_sep_windows;

		orgpkt=ymsg_sess->pkt.type; 

			if (orgpkt==0x46) { /* status */ 	
				char friendy[64];
				char *leaver=NULL;
				ymsg_sess->pkt.type=YMSG_AWAY;
				leaver=strchr(ymsg_sess->pkt.data, 0x2e);
				if (leaver) {*leaver='\0';}
				if (strlen(ymsg_sess->pkt.data)>5) {
					snprintf(friendy,62, "%s", ymsg_sess->pkt.data+5);
					leaver=strrchr(friendy, 0x1b);
					if (leaver) {
						*leaver='\0';
						if ((strlen(friendy)>1) && ycht_only) {
						if (find_friend(friendy)) {
							add_online_friend(friendy);
							leaver++;
							if (strlen(leaver)>3) {
								leaver +=3;
								set_buddy_status(friendy, leaver);
								update_buddy_clist();
							}
						} else {add_temporary_friend(friendy);} 
					}
						
					}
				}
				ych_hash(14, strdup(ymsg_sess->pkt.data));
				handled=1;
			}

			if (orgpkt==0x68) { /* buddy on or off */ 	
				char *onoff=NULL;
				char *leaver=NULL;
				ymsg_sess->pkt.type=YMSG_BUDDY_ON;
				onoff=strstr(ymsg_sess->pkt.data, YMSG_SEP);
				if (onoff) {
					int bonline=0;
					*onoff='\0';
					bonline=atoi(ymsg_sess->pkt.data);
					if (! bonline) {ymsg_sess->pkt.type=YMSG_BUDDY_OFF;}
					onoff += 2;
					leaver=strstr(onoff, YMSG_SEP);
					if (leaver) {*leaver='\0';}
					if (strcasecmp(onoff, ymsg_sess->user) && (! find_profile_name(onoff) )) {
						if ( (strlen(onoff)>1) && ycht_only) {
							if (! find_friend(onoff)) {add_temporary_friend(onoff);}
							ych_hash(7, strdup(onoff));
							ych_hash(10, strdup("0"));
							handled=1;
						}
					}
				}
			}


			if (orgpkt==0x12) { /* leave room */ 	
				char *leaver=NULL;
				ymsg_sess->pkt.type=YMSG_EXIT;
				leaver=strstr(ymsg_sess->pkt.data, YMSG_SEP);
				if (leaver) {
					leaver += 2;
					if (strcasecmp(leaver, ymsg_sess->user) && (! find_profile_name(leaver) )) {
						if (strlen(leaver)>1) {
							ych_hash(109, strdup(leaver));
							handled=1;
						}
					}
				}
			}

			if ( (orgpkt==0x41) || (orgpkt==0x42) || (orgpkt==0x43) ) { /* chat message */ 	
				char *leaver=NULL;
				ymsg_sess->pkt.type=YMSG_COMMENT;
				leaver=strstr(ymsg_sess->pkt.data, YMSG_SEP);
				if (leaver) {
					leaver += 2;					
						if (strlen(leaver)>1) {
							char *chmsg=NULL;
							char emote[8];
							chmsg=strstr(leaver, YMSG_SEP);
							if (chmsg) {
								*chmsg='\0';
								chmsg += 2;
								if (strcasecmp(leaver, ymsg_sess->user) && 
									(! find_profile_name(leaver) )) {
										snprintf(emote,6, "%d", (orgpkt==0x41)?1:0);
										ych_hash(109, strdup(leaver));
										ych_hash(117, strdup(chmsg));
										ych_hash(124, strdup(emote));
										handled=1;
									}
								}
							}
					}
			}


			if ( orgpkt==0x45) { /* pm, usually dont get this anymore */ 	
				char *leaver=NULL;
				ymsg_sess->pkt.type=YMSG_PM;
				leaver=ymsg_sess->pkt.data;
				if (leaver) {
						if (strlen(leaver)>1) {
							char *chmsg=NULL;
							char *chme=NULL;
							chme=strstr(leaver, YMSG_SEP);
							if (chme) {
								*chme='\0';
								chme += 2;
								chmsg=strstr(chme, YMSG_SEP);
								if (chmsg) {
									if (strcasecmp(leaver, ymsg_sess->user) && 
									  (! find_profile_name(leaver) )) {
										char *stopper=NULL;
										*chmsg='\0';
										chmsg += 2;			
										stopper=strstr(chmsg, YMSG_SEP);	
										if (stopper) {*stopper='\0';}
										ych_hash(1, strdup(leaver));
										ych_hash(4, strdup(leaver));
										ych_hash(14, strdup(chmsg));
										ych_hash(5, strdup(chme));
										handled=1;
										/* pm_in_sep_windows=0; */ 
								}
							}
						}
					}
				}
			}


			if ( orgpkt==0x17) { /* invite */ 	
				char *leaver=NULL;
				ymsg_sess->pkt.type=YMSG_INVITE;
				leaver=ymsg_sess->pkt.data;
				if (leaver) {
						if (strlen(leaver)>1) {
							char *chmsg=NULL;
							char *chme=NULL;
							chme=strstr(leaver, YMSG_SEP);
							if (chme) {
								*chme='\0';
								chme += 2;
								chmsg=strstr(chme, YMSG_SEP);
								if (chmsg) {
									if (strcasecmp(leaver, ymsg_sess->user) && 
									  (! find_profile_name(leaver) )) {
										*chmsg='\0';
										chmsg += 2;				
										ych_hash(117, strdup(leaver));
										ych_hash(119, strdup(chmsg));
										ych_hash(104, strdup(chme));
										handled=1;
								}
							}
						}
					}
				}
			}


			if (orgpkt==0x11) { /* join */ 
				int room_cnt=0;
				int new_room=0;
				int goto_err=0;
				int join_err=0;
				char *roomies=NULL;
				char *rmptr=NULL;
				char *rmend=NULL;
				char *startlist="\x30\xc0\x80";

				ymsg_sess->pkt.type=YMSG_JOIN;
				rmptr= ymsg_sess->pkt.data;
				roomies=malloc(2048);
				if (! roomies) {return;}
				handled=1;
				if (strstr( rmptr,"That room is full.")) {join_err=1;}
				if (strstr( rmptr,"Danger Will Robinson")) {goto_err=1;}
				if (strstr(rmptr, "ad.html")) {new_room=1; ych_hash(126, strdup("0"));}

				sprintf(roomies,"%s","");
				rmend=strstr(rmptr,startlist);
				if (rmend) {
					char *piecest=NULL;
					char *pieceend=NULL;	
					while (strncmp(rmend, startlist, 3 )==0) {
						rmend += 3; 
					}
					snprintf(roomies,2046, "%s", rmend);
					piecest=roomies;
					pieceend=strchr(piecest, 0x02);
					while (pieceend) {
						*pieceend='\0';
						if (strlen(piecest)>2) {ych_hash(109, strdup(piecest));}
						room_cnt++;
						pieceend++;
						piecest=pieceend;
						pieceend=strchr(piecest, 0x01);
						if (pieceend) {
							piecest=pieceend;
							piecest++;
							pieceend=strchr(piecest, 0x02);
						} else {break;}
					}
				}

				snprintf(roomies, 4,  "%d", room_cnt);
				if (room_cnt>0) {ych_hash(108, strdup(roomies));}

				rmend=strstr(rmptr,YMSG_SEP);
				if (rmend) { /* room name */ 
					*rmend='\0';
					ych_hash(104, strdup(rmptr)); 
					rmptr=rmend;
					rmptr += 2;
				}
				rmend=strstr(rmptr,YMSG_SEP);
				if (rmend) {  /* topic */
					*rmend='\0';
					if (new_room) { ych_hash(105, strdup(rmptr)); }
					rmptr=rmend;
					rmptr += 2;
				}
				rmend=NULL;
				rmptr=strstr(rmptr,"vcauth=");  /* voice cookie */ 
				if (rmptr) { rmend=strstr(rmptr+7,"&"); }
				if (rmend) {
					*rmend='\0';
					ych_hash(130, strdup(rmptr+7));
					rmptr=rmend;
					rmptr += 1;
				}
				rmend=NULL;
				if (rmptr) { /* chat serial */ 
					rmptr=strstr(rmptr,"rmspace=");
					if (rmptr) { rmend=strstr(rmptr+8,YMSG_SEP); }
					if (rmend) {
						*rmend='\0';
						ych_hash(129, strdup(rmptr+8));
						rmptr=rmend;
						rmptr+=2;
						}
				}

				if (join_err) {
					sprintf(ycht_buf,"%s","");
					ych_hash(114, strdup("ERR"));
				}
				if (goto_err) {
					sprintf(ycht_buf,"%s","");
					ych_hash(114, strdup("ERR"));
					ymsg_sess->pkt.type=YMSG_GOTO;
				}
				
				if (roomies) {free(roomies);}
			}

			if (! handled) {pm_in_sep_windows=org_pm_setting; return;}
			ymsg_sess->pkt.size=strlen(ycht_buf);
			if (ymsg_sess->pkt.size>2047) {ymsg_sess->pkt.size=2047;}
			memcpy( ymsg_sess->pkt.data, ycht_buf, ymsg_sess->pkt.size );
			ymsg_sess->pkt.data[ ymsg_sess->pkt.size ] = '\0';
			handle_yahoo_packet();
			pm_in_sep_windows=org_pm_setting;
}
