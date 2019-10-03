/*****************************************************************************
 * yahoochat.c
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

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

#include "gyach.h"
#include "yahoochat.h"
#include "yahoo_authenticate.h"
#include "users.h"
#include "util.h"
#include "bootprevent.h"
#include "conference.h"
#include "webconnect.h"
#include "plugins.h"
#include "profname.h"
#include "ycht.h"

int conference_counter=128;  /* added PhrozenSmoke */
extern int activate_profnames;
int show_radio_is_on=0;
int show_cam_is_on=0;
int login_invisible=0;
int login_name_changed=1;
int force_busy_icon=0;
int force_idle_icon=0;

char *last_credentials_key=NULL;
char *verify_passphrase=NULL;
int is_bimage_upload=0;
int sharing_bimages=0;

int buddy_image_upload_method=0;  /* 0=normal, 1=reflective */ 

int come_back_from_invisible=0;

int ALTERNATIVE_LOGIN_PLUGIN=0;

/* some protocol and connection settings */ 
int emulate_ymsg6=1;
char *version_emulation="7,5,0333";
int YMSG_PROTOCOL=0x000C0000;
int YMSG_PROTOCOL_STABLE=0x000B0000;

	/* For historical notes, protocol numbers */
	/*  0x000B0000 - current: YMSG-11, Messenger 5.6 */
	/*  0x000C0000 - current: YMSG-12, Messenger 6 Beta */
	/*  0x09000000 -  old, YMSG-9 */
	/*  0x0A000000 - old YMSG10 */ 
	/*  0x0a00c800 - latest  'official' Linux Yahoo client */ 


extern void deny_buddy_all_profile_names(char *who);

int ymsg_conference_leave( YMSG_SESSION *session);
int ymsg_messenger_ping( YMSG_SESSION *session );

u_char yrecvbuf[2048];
u_char ysendbuf[5120];  /* conf invites can require upto 5kb, also safety */
u_char ypingbuf[128];

/* Above, pings and chat pings are sent using gtk_timeout's,
   so its probably a good idea to use a separate buffer */
int without_yreset=0;
void reset_yping_buf() {memset(ypingbuf, 0, sizeof(ypingbuf));}
void reset_yrecv_buf() {memset(yrecvbuf, 0, sizeof(yrecvbuf));}
void reset_ysend_buf() {memset(ysendbuf, 0, sizeof(ysendbuf));}


/* Added PhrozenSmoke, turn proxy use off if configurations seem invalid */

void check_proxy_config() {
	if (use_proxy) {
		if (!proxy_host) {use_proxy=0; return;}
		if (strlen(proxy_host)<2) {use_proxy=0; return;}
		if (proxy_port<1) {use_proxy=0; return;}
		if (proxy_port>65535)  {use_proxy=0; return;}
	}
}

int ymsg_open_socket( YMSG_SESSION *session ) {
	struct sockaddr_in addr;
	struct hostent *hinfo;
	int sock = -1;

	/* initialize the session vars */
	session->sock = -1;
    	memset(&addr ,0 ,sizeof(addr)); 
	
	reset_ysend_buf();
	reset_yrecv_buf();

	check_proxy_config();	

	/* figure out the hosts address */
	if ( session->proxy_host[0] && use_proxy ) {
	    addr.sin_addr.s_addr = inet_addr( session->proxy_host ); 
	} else {
	    addr.sin_addr.s_addr = inet_addr( session->host ); 
	}
 
    if ( addr.sin_addr.s_addr == (u_int)INADDR_NONE ) {
	set_socket_timer(sock, -1);
	set_socket_timer(sock, 12);

		if ( session->proxy_host[0] && use_proxy ) {
			hinfo = gethostbyname( session->proxy_host ); 
		} else {
			hinfo = gethostbyname( session->host ); 
		}

	set_socket_timer(sock, -1);
 
        if (( !hinfo ) ||
			( hinfo->h_addrtype != AF_INET )) {
			if ( session->proxy_host[0] && use_proxy ) {
				snprintf( session->error_msg, 127, 
					"Could not resolve DNS address ( %s ): %d",
					session->proxy_host, errno );
			} else {
				snprintf( session->error_msg,127, 
					"Could not resolve DNS address ( %s ): %d",
					session->host, errno );
			}
            return 0; 
        } 
 
        memset( &addr, 0, sizeof(addr)); 
        memcpy((void*)&addr.sin_addr.s_addr, hinfo->h_addr, hinfo->h_length ); 
    } 

    addr.sin_family = AF_INET; 

	/* set the hosts port */
	if ( session->proxy_host[0] && use_proxy) {
	    addr.sin_port = htons( session->proxy_port ); 
	} else {
	    addr.sin_port = htons( session->port ); 
	}

	/* create a socket() to connect with and connect() */
    sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ); 
    set_socket_timer(sock,10);
    if ( sock == -1 ) {
		snprintf( session->error_msg, 127, "Socket connection failed: %d", errno );
		 set_socket_timer(sock,-1);
		return( 0 );
	} else if ( connect( sock, (struct sockaddr*)&addr, sizeof(addr)) == -1 ) {
		snprintf( session->error_msg, 127, "Socket connection failed: %d", errno );
		 set_socket_timer(sock,-1);
        	close( sock ); 
		return( 0 );
    } 

    set_socket_timer(sock,-1);

	/* if we are using a proxy host, send the CONNECT string */
	if ( session->proxy_host[0] && use_proxy) {		
		char buf[768];
		char tmp[128];

			if ( capture_fp) {	
				time_t time_llnow = time(NULL);
				fprintf(capture_fp, "\n%s\nPROXY SETTINGS (ymsg_open_socket):\nProxy Host: %s\nPort: %d\n\n", ctime(&time_llnow), proxy_host, proxy_port);
				fflush( capture_fp );
			}

		snprintf( buf, 765, "CONNECT %s:%d HTTP/1.1\r\nHost: %s:%d\r\n\r\n", session->host,
			session->port ,session->host, session->port  );

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
			snprintf( session->error_msg,127, 
				"Error connecting via proxy.  Proxy replied: '%s'", tmp );
			close( sock );
			return( 0 );
		}
	}

	/* success so set our session socket */
	session->sock = sock;

	/* sock will be positive here, so return as true */
    return( sock ); 
}

int ymsg_sock_send( YMSG_SESSION *session, u_char *buf, int size ) {
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
		sent = send( session->sock, ptr, size, 0 );
		if ( sent < 1 ) {
			if ((errno != EAGAIN) && (errno != EINTR)) {
				snprintf( session->error_msg, 127, "Error sending to socket: %d",errno );
				if (errno==EPIPE) {
					handle_dead_connection (1,0,0,_("You have been disconnected from Yahoo!: Broken pipe."));
				} else {
					handle_dead_connection (1,0,0,_("You have been disconnected from Yahoo!"));
						 }
			}
			break;
		}
		ptr += sent;
		size -= sent;
	}
	if (!without_yreset) {reset_ysend_buf();}  /* clear the send buffer */

	return( size == 0 );
}


int ymsg_sock_recv( YMSG_SESSION *session, u_char *buf, int size ) {
	int recvd;
	char *ptr = buf;

	while( size ) {
		recvd = recv( session->sock, ptr, size, 0 );
		if ( recvd < 1 ) {
			if ((errno != EAGAIN) && (errno != EINTR)) {
				snprintf(session->error_msg,127,"Error receiving from socket: %d\n", errno );
				if (errno==EPIPE) {
					handle_dead_connection (1,0,0,_("You have been disconnected from Yahoo!: Broken pipe."));
				} else {
					handle_dead_connection (0,0,0,_("You have been disconnected from Yahoo!"));
						 }
			}
			break;
		}
		ptr += recvd;
		size -= recvd;
	}
	return( size == 0 );
}

u_char *ymsg_header( YMSG_SESSION *session, u_char *buf, u_long pkt_type ) {
	u_char *ptr = buf;

	memcpy( buf, "YMSG", 4 );					ptr += 4;  /* msg procotol */

	switch( pkt_type ) {
		case YMSG_AWAY:
		case YMSG_BACK:
				if (using_web_login ) {
					*((u_long *)ptr) = htonl( 0x00650000 );	ptr += 4;    
											   } else {
				*((u_long *)ptr) = htonl( emulate_ymsg6?YMSG_PROTOCOL:YMSG_PROTOCOL_STABLE );	
				ptr += 4;  

					/* Bug fix: PhrozenSmoke, maybe in an earlier version of 
					YMSG we needed this, but the official client now sends
					the regular protocol number instead of this stuff  below */
					/*  *((u_long *)ptr) = htonl( 0x00070000 );	ptr += 4;  */
														}
				break;

		/*  
		case YMSG_ADD_BUDDY:
		case YMSG_REM_BUDDY:
				*((u_long *)ptr) = htonl( 0x06000000 );	ptr += 4;  
				break;
		*/ 
		/* Above: from older YMSG protocols, 
			bug fix to problem that stopped adding of 
			buddies to empty buddy list, PhrozenSmoke:
			The official clients now send regular protocol number */

		/* 
		case YMSG_BIMAGE_SEND:
			*((u_long *)ptr) = htonl( 0x0c000000 );	ptr += 4;    
			break; */ 

		default:
				if (using_web_login ) {
					*((u_long *)ptr) = htonl( 0x00650000 );	ptr += 4;    
											   } else { 
				*((u_long *)ptr) = htonl( emulate_ymsg6?YMSG_PROTOCOL:YMSG_PROTOCOL_STABLE );	
				ptr += 4;   
														 } 				

				break;
	}

	/* next 4 bytes are packet length which we'll */
	/* fill in later, so set to 0 for now         */
	*((u_short *)ptr) = htons( 0x0000 );		ptr += 2;  /* data length  */

	*((u_short *)ptr) = htons( pkt_type );		ptr += 2;  /* packet type  */

	switch (pkt_type) { /* status  */
		case YAHOO_STATUS_WEBLOGIN:
			*((u_long *)ptr) = htonl( YAHOO_STATUS_WEBLOGIN );
			break; 
		case YMSG_PM:
			/* added by PhrozenSmoke, enables offline messaging */
			*((u_long *)ptr) = htonl( 0x5a55aa56 );	
			break; 
		case YMSG_LOGIN:
			if (login_invisible) {*((u_long *)ptr) = htonl( 0x0000000c );} 
		 	else {*((u_long *)ptr) = htonl( 0x00000000 ); }
			break; 
		/* case YMSG_IDACT: */ 
		case YMSG_IDDEACT:
				*((u_long *)ptr) = htonl( 0x00000001 );  
				break;
		default:
			*((u_long *)ptr) = htonl( 0x00000000 );	
			break; 
	}
	ptr += 4; 

	switch( pkt_type ) {
		case YMSG_GET_KEY:
		case YMSG_VERIFY:		
		case YMSG_ADD_BUDDY:
		case YMSG_REM_BUDDY:
				*((u_long *)ptr) =
					htonl( 0x00000000 );	ptr += 4;  /* Session ID   */
				break;
		default:
				if (using_web_login) {
				*((u_long *)ptr) =
					htonl( 0x00000000 );	ptr += 4;  /* Session ID   */
											  } else {
				*((u_long *)ptr) =
					htonl( session->session_id );	ptr += 4;  /* Session ID   */
														 }
				break;
	}

	return( ptr );
}

void ymsg_dump_packet( YMSG_SESSION *session, u_char *pkt ) {
	int len, type;
	int i = 0;
	int x = 0;
	int y = 0;
	char *ptr;

	if ( strncmp( pkt, "YMSG", 4 )) {
		printf( "ymsg_dump_packet(), invalid packet\n" );
		return;
	}

	len = ntohs((u_short)*((u_short*)(pkt+8)));
	type = ntohs((u_short)*((u_short*)(pkt+10)));

	fprintf( stderr, "-----------------------------------------------------"
		"-------------------------\n" );
	if ( capture_fp ) {
		time_t time_llnow;
		time_llnow = time(NULL);
		fprintf( capture_fp,  "\n--------------------------------\n%s\n--------------------------------\n",
		ctime(&time_llnow));
	}
	fprintf( stderr, "sess id   : %u\n", session->session_id );
	if ( capture_fp ) {fprintf( capture_fp, "sess id   : %u\n", session->session_id );}
	fprintf( stderr, "data len  : %d\n", len );
	if ( capture_fp ) {fprintf( capture_fp,"data len  : %d\n", len );}
	fprintf( stderr, "pkt type  : 0x%x\n", type );
	if ( capture_fp ) {fprintf( capture_fp,"pkt type  : 0x%x\n", type );}

	for( i = 0; i < 20; i++ ) {
		fprintf( stderr, "%02x ", *(pkt+i) & 0xFF );
		if ( capture_fp ) {fprintf( capture_fp,"%02x ", *(pkt+i) & 0xFF );}
	}
	fprintf( stderr, "\n" );
	for( i = 0; i < 20; i++ ) {
		if ( isgraph( *(pkt+i) ) || *(pkt+i)  == ' ' ) {
			fprintf( stderr, "%c  ", *(pkt+i) );
			if ( capture_fp ) {fprintf( capture_fp,"%c  ", *(pkt+i) );}
		} else {
			fprintf( stderr, ".  " );
			if ( capture_fp ) {fprintf( capture_fp,".  " );}
		}
	}
	fprintf( stderr, "\n" );
	if ( capture_fp ) {fprintf( capture_fp, "\n" );}

	i = 0;
	ptr = pkt + 20;
	while( i < len ) {
		if ( x < 16 ) {
			fprintf( stderr, "%02x ", *(ptr+i) & 0xFF );
			if ( capture_fp ) {fprintf( capture_fp, "%02x ", *(ptr+i) & 0xFF );}
			x++;
			i++;
		} else {
			fprintf( stderr, "   " );
			for( ; x > 0 ; x-- ) {
				if ( isgraph( *(ptr+i-x) ) || *(ptr+i-x)  == ' ' ) {
					fprintf( stderr, "%c", *(ptr+i-x) );
					if ( capture_fp ) {fprintf( capture_fp, "%c", *(ptr+i-x) );}
				} else {
					fprintf( stderr, "." );
					if ( capture_fp ) {fprintf( capture_fp,".");}
				}
			}

			fprintf( stderr, "\n" );
			if ( capture_fp ) {fprintf( capture_fp,"\n");}
			x = 0;
		}
	}
	for( y = x; y < 16 ; y++ ) {
		fprintf( stderr, "   " );
		if ( capture_fp ) {fprintf( capture_fp, "   ");}
	}
	fprintf( stderr, "   " );
	for( ; x > 0 ; x-- ) {
		if ( isgraph( *(ptr+i-x) ) || *(ptr+i-x)  == ' ' ) {
			fprintf( stderr, "%c", *(ptr+i-x) );
			if ( capture_fp ) {fprintf( capture_fp, "%c", *(ptr+i-x) );}
		} else {
			fprintf( stderr, "." );
			if ( capture_fp ) {fprintf( capture_fp, "." );}
		}
	}

	fprintf( stderr, "\n" );
	if ( capture_fp ) {	fprintf( capture_fp, "\n" ); fflush( capture_fp );}
}


int ymsg_send_packet( YMSG_SESSION *session, u_char *bufp, int size ) {
	/* set data length in packet */
	*((u_short *)(bufp+8)) = htons( size - 20 );

	/* don't try to send if we're not connected */
	if ( session->sock == -1 ) {
		if ( session->debug_packets ) {ymsg_dump_packet( session, bufp );}
		return( 0 );
	}
	if ( session->debug_packets ) {ymsg_dump_packet( session, bufp );}
	/* send the packet */
	return ymsg_sock_send( session, bufp, size );
}

int ymsg_recv_data( YMSG_SESSION *session ) {
	reset_yrecv_buf();

	if ( ! ymsg_sock_recv( session, yrecvbuf, 20 )) {
		return( 0 );
	}

	session->pkt.size = ntohs((u_short)*((u_short*)(yrecvbuf+8)));


	if ( ! ymsg_sock_recv( session, yrecvbuf + 20, session->pkt.size )) {
		return( 0 );
	}

	session->pkt.type = ntohs((u_short)*((u_short*)(yrecvbuf+10)));


	/* some flow control to avoid booting, added PhrozenSmoke */
	sock_sleep=10000;
	if (enable_basic_protection) 	{
		int is_dangerous=1;  /* Medium security by default */
		int very_dangerous=0;

		/* packets that may be big but are sent from Yahoo directly=safe */
		switch (session->pkt.type) {
			case YMSG_VERIFY:
			case YMSG_STEALTH:
			case YMSG_STEALTH_PERM:
			case YMSG_SETTINGS:
			case YMSG_BUDDY_STATUS:
			case YMSG_GET_KEY:
			case  YMSG_JOIN:
			case YMSG_ONLINE:
			case YMSG_LOGIN:
			case YMSG_COOKIE:
			case YMSG_BUDDY_ON:
			case YMSG_BUDDY_OFF:
			case YMSG_MAIL:
			case YMSG_IDDEACT:
			case YMSG_IDACT:
			case YMSG_USERSTAT:
			case YMSG_GROUP_RENAME:
			case YMSG_GOTGROUP_RENAME:
			case YMSG_NOTICIAS:
			case YMSG_CALENDAR:
				/* Stuff coming directly from Yahoo's system should be safe */
				is_dangerous=0;
				break;

			case  YMSG_CONFDECLINE:
			case YMSG_CONFADDINVITE:
			case YMSG_CONFINVITE:
			case YMSG_CONFMSG:
			case YMSG_NOTIFY:
			case YMSG_PM:
			case YMSG_NEW_CONTACT:
			case YMSG_ADD_BUDDY:
			case YMSG_INVITE:
			case YMSG_FILETRANSFER:
			case YMSG_P2P:
			case YMSG_BIMAGE:  /* Can be used to 'bomb' us */
			case YMSG_GAME_INVITE:  /* Can be used to 'bomb' us */
			case YMSG_AUDIBLE:  /* Can be used to 'sound bomb' us */
				very_dangerous=1;
				break;
		}
									

	if (session->pkt.size>800 && is_dangerous) {
		sock_sleep=60000;
		if (very_dangerous) {ymsg_messenger_ping(ymsg_sess); sock_sleep=75000;}
																}
	/* normally these are excessively large conference declines or other 'boot' packets */
	if (session->pkt.size>=950) {
		sock_sleep=75000;
		if (very_dangerous)			 {
			ymsg_messenger_ping(ymsg_sess); 
			sock_sleep=100000;
												}
										}
	if (session->pkt.size>=1200) {
		sock_sleep=100000;
		if (very_dangerous) 		{
			ymsg_messenger_ping(ymsg_sess); 
			sock_sleep=150000;
			push_packet_suppression ();
												}
										}
							}



	if ( session->pkt.type == YMSG_GET_KEY ) {
		session->session_id  = ntohl((u_long)*((u_long *)(yrecvbuf+16)));
	}


	if ( session->pkt.type == YMSG_COOKIE ) {
		if (using_web_login) {
			session->session_id=ntohl((u_long)*((u_long *)(yrecvbuf+16)));
			/* printf("\nnew session id set!   %d\n\n", session->session_id); fflush(stdout); */
									  }
	}
	
	if (session->session_id<1) {
		if ( (ntohl((u_long)*((u_long *)(yrecvbuf+16)))) >1) {
			session->session_id=ntohl((u_long)*((u_long *)(yrecvbuf+16)));
		}
	}

	if ( session->debug_packets )
		ymsg_dump_packet( session, yrecvbuf );

	yrecvbuf[session->pkt.size + 20] = '\0';
	memcpy( session->pkt.data, yrecvbuf + 20, session->pkt.size );
	session->pkt.data[ session->pkt.size ] = '\0';
	check_packet_suppression();

	/* if suppress duplicate consecutive packets is on then check */
	if (( session->suppress_dup_packets ) &&
		( ! memcmp( &session->last_pkt, &session->pkt, sizeof( session->pkt )))) {
		return( 0 );
	}

	memcpy( &session->last_pkt, &session->pkt, sizeof( session->pkt ));

	return( 1 );
}

int ymsg_get_key( YMSG_SESSION *session ) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_GET_KEY );
	int len;
	snprintf( ptr, 106, "1%s%s%s", YMSG_SEP, session->user, YMSG_SEP );
	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );	
	return( 0 );
}

int ymsg_request_login( YMSG_SESSION *session ) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_VERIFY );
	int len;
	snprintf( ptr, 3, "%s", "");
	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );
	return( 0 );
}

int ymsg_request_key( YMSG_SESSION *session ) {
	if (emulate_ymsg6) {return ymsg_request_login(session);	}
	return ymsg_get_key(session);
}


int ymsg_login( YMSG_SESSION *session, char *key ) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_LOGIN );
	char sextra[32]="";
	int len;
	char *get1;
	char *get2;

	if (ALTERNATIVE_LOGIN_PLUGIN) {
			/* 	Use an alternative login plugin when Yahoo locks us out, only available
			on my system and not currently distributed because it is probably 
			a violation of the GPL license to do so - not quite legal */

		yahoo_process_auth_altplug(session->user,  session->password, key);
	}  else {
		/* default login process for the average Joe */
		yahoo_process_auth_0x0b(session->user,  session->password, key);
	}

	/* Make allowances for existing credentials for retry on multiple bad logins */  
	get1=getSHAstr1(); 
	get2=getSHAstr2(); 

	if (! last_credentials_key) {last_credentials_key=strdup(key);}


	/* printf("GET1:  %s\n",get1);
	printf("GET2:  %s\n",get2);
	fflush(stdout);  */

	/* let's try to look as much like the official Windows client as possible, PhrozenSmoke */

	sprintf(sextra,"%s","");
		if (verify_passphrase)  {
			snprintf(sextra, 30 , "%s%s%s%s", 
				 "227", YMSG_SEP, verify_passphrase, YMSG_SEP
				);
			free(verify_passphrase);
			verify_passphrase=NULL;
		}
	
	if (emulate_ymsg6) {
	snprintf( ptr, 1478, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		"6", YMSG_SEP, get1,YMSG_SEP,
		"96", YMSG_SEP,get2,YMSG_SEP,
		sextra, 
		"0", YMSG_SEP, session->user, YMSG_SEP,
		"2", YMSG_SEP, session->user, YMSG_SEP,
		"192", YMSG_SEP, "-1", YMSG_SEP,
		"2", YMSG_SEP, activate_profnames?"1":"0", YMSG_SEP,
		"1", YMSG_SEP, session->user, YMSG_SEP,
		"135", YMSG_SEP, version_emulation , YMSG_SEP, /* impersonate YPager6*/
		"148", YMSG_SEP, "300", YMSG_SEP, 
		/* generic cookies below */
		"59", YMSG_SEP, "B\t9a4k50l00fmia&b=2", YMSG_SEP, 
		"59", YMSG_SEP, "F\ta=.Z6jMDgsvaBtGShSqwPBTq0Ky1SSv3GwNnlt3BTHmMAZ2FO0YmBtuOWyfWKR&b=B0ZL", YMSG_SEP
		);  

	} else {
	snprintf( ptr, 1478, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		"6", YMSG_SEP, get1,YMSG_SEP,
		"96", YMSG_SEP, get2,YMSG_SEP,
		sextra,
		"0", YMSG_SEP, session->user, YMSG_SEP,
		"2", YMSG_SEP, activate_profnames?"1":"0", YMSG_SEP,
		"1", YMSG_SEP, session->user, YMSG_SEP,
		"135", YMSG_SEP, "5, 6, 0, 1358", YMSG_SEP, /* impersonate YPager5.6 latest */
		"148", YMSG_SEP, "300", YMSG_SEP
		);  

	}

	/* more basic, from libyahoo2... */  /* 

	snprintf( ptr, 1478,  "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		"0", YMSG_SEP, session->user, YMSG_SEP,
		"6", YMSG_SEP, get1,YMSG_SEP,
		"96", YMSG_SEP, get2,YMSG_SEP,
		"1", YMSG_SEP, session->user, YMSG_SEP
		);   */


	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );	
	return( 0 );
}

/* added for web login support, Phrozen Smoke */
int ymsg_web_login( YMSG_SESSION *session, char *cookie) {
	u_char *ptr = ymsg_header( session, ysendbuf, YAHOO_SERVICE_WEBLOGIN );
	int len;

	snprintf( ptr, 1000, "%s%s%s%s%s%s%s%s%s%s%s%s",
		"0", YMSG_SEP, session->user, YMSG_SEP,
		"1", YMSG_SEP, session->user, YMSG_SEP,
		"6", YMSG_SEP, cookie, YMSG_SEP
		);

	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );	
	return( 0 );
}


int ymsg_online( YMSG_SESSION *session ) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_ONLINE );
	int len;

	if (using_web_login) {  /* web login connects cant do chat, so fake */
	ymsg_sess->pkt.type = YMSG_ONLINE; 
	show_yahoo_packet();
	return( 0 );
	}

	if (emulate_ymsg6) {
		/* Messenger-B beta style Online packets */
		snprintf( ptr, 350, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		"109", YMSG_SEP, select_profile_name(YMSG_ONLINE, 0), YMSG_SEP,
		"1", YMSG_SEP, select_profile_name(YMSG_ONLINE, 0), YMSG_SEP,
		"6", YMSG_SEP, "abcde", YMSG_SEP, 
		"98", YMSG_SEP, "us", YMSG_SEP,
		"135", YMSG_SEP, "ym",version_emulation, YMSG_SEP  /* app name */
		);

	} else {
		snprintf( ptr, 256, "%s%s%s%s%s%s%s%s%s%s%s%s",
		"109", YMSG_SEP, select_profile_name(YMSG_ONLINE, 0), YMSG_SEP,
		"1", YMSG_SEP, select_profile_name(YMSG_ONLINE, 0), YMSG_SEP,
		"6", YMSG_SEP, "abcde", YMSG_SEP
		);
	}

	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );	
	return( 0 );
}

int ymsg_join( YMSG_SESSION *session ) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_JOIN );
	int len;

	if (using_web_login || ycht_is_running()) { return ycht_join( );}
	
	if (show_cam_is_on) {
		snprintf( ptr, 233, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		"1", YMSG_SEP, select_profile_name(YMSG_JOIN, 0), YMSG_SEP,
		"62", YMSG_SEP, "1", YMSG_SEP,  /* 1=webcam on, 2=webcam off */
		"104", YMSG_SEP, session->req_room, YMSG_SEP,
		"129", YMSG_SEP, "0", YMSG_SEP
		);  
	} else {
		snprintf( ptr, 233, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		"1", YMSG_SEP, select_profile_name(YMSG_JOIN, 0), YMSG_SEP,
		"62", YMSG_SEP, "2", YMSG_SEP,  /* 1=webcam on, 2=webcam off */
		"104", YMSG_SEP, session->req_room, YMSG_SEP,
		"129", YMSG_SEP, "0", YMSG_SEP
		);  
	}

	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );	
	return( 0 );
}

int ymsg_comment( YMSG_SESSION *session, char *text ) {
	if (ycht_is_running()) { return ycht_comment( text, 0 );}

	if (is_conference) {
		return ymsg_conference_msg(session,text);
				  }  

	if (in_a_chat) {	
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_COMMENT );
	int len;

	snprintf( ptr, 1800, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		"1", YMSG_SEP, select_profile_name(YMSG_COMMENT, 0), YMSG_SEP,
		"104", YMSG_SEP, session->room, YMSG_SEP,
		"117", YMSG_SEP, text, YMSG_SEP,
		"124", YMSG_SEP, "1", YMSG_SEP
		);

	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );
				}
	
	return( 0 );
}

int ymsg_emote( YMSG_SESSION *session, char *text ) {
	if (ycht_is_running()) { return ycht_comment( text, 1 );}

	if (is_conference) {
		/* Not sure if conferences can do 'emotes', so we send as a thought */
		char textbuf[736]="";
		snprintf(textbuf,734, "<FADE #00AABB,#3333bb><font face=\"tahoma\" size=\"16\">. o O ( %s )</FADE>", text);
		return ymsg_conference_msg(session,textbuf);
							}

	if (in_a_chat) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_COMMENT );
	int len;

	snprintf( ptr, 1800, "%s%s%s%s%s%s%s%s%s%s<FADE #00AABB,#3333bb><font face=\"tahoma\" size=\"16\">%s</FADE>%s%s%s%s%s",
		"1", YMSG_SEP, select_profile_name(YMSG_COMMENT, 0), YMSG_SEP,
		"104", YMSG_SEP, session->room, YMSG_SEP,
		"117", YMSG_SEP, text, YMSG_SEP,
		"124", YMSG_SEP, "2", YMSG_SEP
		);

	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );
			}
	return( 0 );
}

int ymsg_think( YMSG_SESSION *session, char *text ) {
	if (ycht_is_running()) { return ycht_comment( text, 2 );}

	if (is_conference) {
		char textbuf[736]="";
		snprintf(textbuf,734, "<FADE #00AABB,#3333bb><font face=\"tahoma\" size=\"16\">. o O ( %s )</FADE>", text);
		return ymsg_conference_msg(session,textbuf);
							}

	if (in_a_chat) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_COMMENT );
	int len;

	snprintf( ptr, 1800, "%s%s%s%s%s%s%s%s%s%s<FADE #00AABB,#3333bb><font face=\"tahoma\" size=\"16\">. o O ( %s )</FADE>%s%s%s%s%s",
		"1", YMSG_SEP, select_profile_name(YMSG_COMMENT, 0), YMSG_SEP,
		"104", YMSG_SEP, session->room, YMSG_SEP,
		"117", YMSG_SEP, text, YMSG_SEP,
		"124", YMSG_SEP, "3", YMSG_SEP
		);

	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );
			}
	return( 0 );
}

int ymsg_logout( YMSG_SESSION *session ) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_LOGOUT );
	int len;
	ymsg_conference_leave(session);
	snprintf( ptr, 150, "%s%s%s%s",
		"1", YMSG_SEP, select_profile_name(YMSG_LOGOUT, 0), YMSG_SEP
		);

	if (emulate_ymsg6) {
		strcat(ptr,  "1005");
		strcat(ptr,  YMSG_SEP);
		strcat(ptr, "10837184");
		strcat(ptr,  YMSG_SEP);
	}

	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );
	return( 0 );
}


/* added, PhrozenSmoke, passthrough response to Yahoo ping from their servers */

int ymsg_passthrough( YMSG_SESSION *session ) {
	if (emulate_ymsg6) {
		/* emulate Messenger-6beta and better, lie to Yahoo and
			pretend we are on Windows with the Games tab visible, This 
			is to try to look as much like an 'official' Y! app as possible  */
		u_char *ptr = ymsg_header( session, ysendbuf, YMSG_SETTINGS);	
		int len;	
		snprintf( ptr, 256,  "%s%s%s%s", 
	 		"211", YMSG_SEP, "Tab:YMSGR_Games=1,1\n", YMSG_SEP
			);
		len = 20 + strlen( ptr );
		ymsg_send_packet( session, ysendbuf, len );

	} else { /* old Messenger-5 style passthrough */

	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_PASSTHRU );
	char pthr[70]="";
	int len;
	snprintf(pthr, 69, "C=0%cF=1,B=0,O=0,G=0%cM=1,P=0,C=0,S=0,L=2,D=1,N=0,G=0,F=0,T=0" , 
	'\x01', '\x01'
	);
	snprintf( ptr, 233,  "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", 
	 "1", YMSG_SEP, session->user, YMSG_SEP,
	 "25", YMSG_SEP, pthr, YMSG_SEP,
	 "146", YMSG_SEP, "V2luZG93cyA5OCwgIA==", YMSG_SEP,
	 "145", YMSG_SEP, "SW50ZWwgUGVudGl1bQ==", YMSG_SEP,
	 "147", YMSG_SEP, "RWFzdGVybiBTdGFuZGFyZCBUaW1l", YMSG_SEP
	);
	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );

	}

	return( 0 );
}





/* added, PhrozenSmoke, to log out of Yahoo complete, instead of logging out of chat */

int ymsg_messenger_logout( YMSG_SESSION *session ) {

	if (emulate_ymsg6) {
		u_char *ptr = ymsg_header( session, ysendbuf, YMSG_SIGNOFF);
		int len;
		snprintf( ptr,200, "%s%s%s%s%s%s%s%s", 
	 		"1", YMSG_SEP, session->user, YMSG_SEP,
	 		"5", YMSG_SEP, "", YMSG_SEP	);
		len = 20 + strlen( ptr );
		ymsg_send_packet( session, ysendbuf, len );
	} else {
		u_char *ptr = ymsg_header( session, ysendbuf, YMSG_BUDDY_OFF );
		int len;
		snprintf( ptr, 3, "%s", "");
		len = 20 + strlen( ptr );
		ymsg_send_packet( session, ysendbuf, len );
	}
	return( 0 );
}



/* added, PhrozenSmoke, to keep us from getting booted when we are not
   in a chat - i.e. conference or just 'sitting'  */

int ymsg_messenger_ping( YMSG_SESSION *session ) {
	u_char *ptr = ymsg_header( session, ypingbuf, YMSG_MSGRPING );
	int len;
	snprintf( ptr, 3, "%s", "");
	len = 20 + strlen( ptr );
	without_yreset=1;
	ymsg_send_packet( session, ypingbuf, len );
	reset_yping_buf();
	without_yreset=0;
	return( 0 );
}

/* The ping when we are in chat */
int ymsg_chat_ping( YMSG_SESSION *session ) {
	u_char *ptr = ymsg_header( session, ypingbuf, YMSG_PING );
	int len;

	snprintf( ptr, 106, "%s%s%s%s",
		"109", YMSG_SEP, select_profile_name(YMSG_PING, 0), YMSG_SEP
		);
	len = 20 + strlen( ptr );
	without_yreset=1;
	ymsg_send_packet( session, ypingbuf, len );
	reset_yping_buf();
	without_yreset=0;
	return( 0 );
}

int ymsg_ping( YMSG_SESSION *session ) {
	if (in_a_chat) {
		if (! ycht_is_running()) {	
			ymsg_chat_ping(session);  /* in a ymsg-based chat */ 
		}
	} 
	if (ycht_is_running()) { ycht_ping( );}  /* in a ycht-based chat */ 
	if (! ycht_only) {ymsg_messenger_ping(session);}  /* on web-login or regular ymsg */ 
	return( 0 );
}


int ymsg_set_visbility( YMSG_SESSION *session, int visible ) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_VISIBILITY );
	int len;
		/* In Messenger-6 mode, set visibility:
				0=offline (sign off...i think)
				1=visible
				2=invisible */
	if (! emulate_ymsg6) {return( 0 );}
	snprintf( ptr, 100, "%s%s%d%s",
		"13", YMSG_SEP, visible, YMSG_SEP
		);
	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );
	return( 0 );
}


char *away_msgs[] = {
	"Here",
	"Be Right Back",
	"Busy",
	"Not At Home",
	"Not At My Desk",
	"Not In The Office",
	"On The Phone",
	"On Vacation",
	"Out To Lunch",
	"Stepped Out",
	"",
	"Auto-Away",
	"Invisible",
	NULL
	};

int ymsg_away( YMSG_SESSION *session, char *msg ) {
	u_char *ptr=NULL;
	int len;
	char *mptr;
	char tmp[264];
	char wcamstat[3]="";
	char *mybusy="0";

	if (emulate_ymsg6) {
		if (my_status==12) { /* going invisible */
			ymsg_set_visbility( session, 2 );
			return (0);
		}
		if (come_back_from_invisible) {	ymsg_set_visbility( session, 1 );	}
		ptr= ymsg_header( session, ysendbuf, YMSG_BUDDY_STATUS );

	} else { ptr= ymsg_header( session, ysendbuf, YMSG_AWAY ); }

	strncpy( tmp, msg, emulate_ymsg6?261:71 );
	mptr = strchr( tmp, ':' );
	*mptr = '\0';
	mptr++;

	if ( ! strcmp( tmp, "10" )) {
		strcpy( tmp, "99" );
		if (force_busy_icon) {mybusy="1";}
		if (force_idle_icon) {mybusy="2";}
	}  else  {
		if ( ! strcmp( tmp, "01" )) {mybusy="0"; }  /* available, disable busy/idle icons  */
		else if ( ! strcmp( tmp, "11" )) {mybusy="2"; }  /* auto-away, use 'idle' icon */
		else if ( ! strcmp( tmp, "999" )) {mybusy="2"; }   /* idle, use 'idle' icon */
		else {mybusy="1"; }  /*  other aways, use 'busy' icon */
			}

	if (emulate_ymsg6) {
		char sextra[72]="";
		sprintf(sextra,"%s","");

		if (! strncmp( tmp, "99", 2 ))  {
			snprintf(sextra,54, "%s%s%s%s","187", YMSG_SEP, "0", YMSG_SEP);
			if (show_radio_is_on) {
				snprintf(sextra,70, "%s%s%s%s%s%s%s%s",
				"184",YMSG_SEP,"YSTATUS=2\tu\t1376579776\ts\t3391323\tp\t1\tm\t459\td\t0",YMSG_SEP,
				"187", YMSG_SEP, "2", YMSG_SEP
				);
				}
			if (show_cam_is_on) {
				snprintf(sextra,54, "%s%s%s%s%s%s%s%s",
				"184", YMSG_SEP, "YSTATUS=1", YMSG_SEP,
				"187", YMSG_SEP, "1", YMSG_SEP
				);
				} 
		}

		snprintf( ptr, 768, "%s%s%d%s%s%s%s%s%s%s%s%s%s",
		"10", YMSG_SEP, atoi(tmp), YMSG_SEP,
		"19", YMSG_SEP, mptr, YMSG_SEP,
		"47", YMSG_SEP, mybusy, YMSG_SEP,  sextra
		);

	} else { /* older protocol */
		if (show_cam_is_on && (! strcmp( tmp, "99" ))) {
			/* This is how the YMSG-11 protocol showed a webcam was on */
			wcamstat[0]=0xa0;
			wcamstat[1]=0xa0;
			wcamstat[2]='\0';
		} else {sprintf(wcamstat,"%s","");}

	snprintf( ptr, 512, "%s%s%d%s%s%s%s%s%s%s%s%s%s",
		"10", YMSG_SEP, atoi(tmp), YMSG_SEP,
		"19", YMSG_SEP, mptr, wcamstat, YMSG_SEP,
		"47", YMSG_SEP, mybusy, YMSG_SEP
		);
	}


	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );
	force_busy_icon=0;
	force_idle_icon=0;
	show_radio_is_on=0;

	return( 0 );
}

int ymsg_back( YMSG_SESSION *session ) {
	int len;

	if (emulate_ymsg6) {
		u_char *ptr=NULL;				
		if  ( come_back_from_invisible) { ymsg_set_visbility( session, 1 ); }
		ptr=ymsg_header( session, ysendbuf, YMSG_BUDDY_STATUS );		
		snprintf( ptr, 200, "%s%s%s%s%s%s%s%s",
			"10", YMSG_SEP, "0", YMSG_SEP,
			"19", YMSG_SEP, "", YMSG_SEP
			);
		len = 20 + strlen( ptr );
		ymsg_send_packet( session, ysendbuf, len );		
	} else {  /* The old protocol 'back' packets */
		ymsg_header( session, ysendbuf, YMSG_BACK );
		len = 20;
		ymsg_send_packet( session, ysendbuf, len );

		}
	return( 0 );
}

int ymsg_pm_full( YMSG_SESSION *session, char *remote_user, char *msg, char *imvironment ) {
	char imv[32]="";
	char sextra[16]="";
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_PM );
	int len;

	if (ycht_only && ycht_is_running()) {
		return ycht_pm( remote_user, msg);
	}

	if (strchr(imvironment,';')) {snprintf(imv,24,"%s",imvironment);} 
	else {snprintf(imv,21,"%s",imvironment); strcat(imv,";0"); }

	snprintf(sextra, 12, "%s", "" );
	if (sharing_bimages) {
		snprintf(sextra, 12, "%s%s%d%s", "206", YMSG_SEP, sharing_bimages, YMSG_SEP);
	}

	snprintf( ptr, 1536, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%d%s",
		"1", YMSG_SEP, select_profile_name(YMSG_PM, 1), YMSG_SEP,
		"5", YMSG_SEP, remote_user, YMSG_SEP,
		"14", YMSG_SEP, msg, YMSG_SEP,
		"97", YMSG_SEP, "1", YMSG_SEP,
		"63", YMSG_SEP, imv, YMSG_SEP,
		"64", YMSG_SEP, "0", YMSG_SEP,

		/* New in messenger-6 protocol */
		sextra,   /* buddy image on or off */
		"15", YMSG_SEP, (int) time(NULL), YMSG_SEP  /* time stamp */
		);

	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );
	sharing_bimages=0;
	return( 0 );
}


int ymsg_pm( YMSG_SESSION *session, char *remote_user, char *msg ) {
	return ymsg_pm_full(session,remote_user,msg,";0");
}




int ymsg_add_buddy( YMSG_SESSION *session, char *friend ) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_ADD_BUDDY );
	int len;

	if (emulate_ymsg6) {
	snprintf( ptr, 400, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		"1", YMSG_SEP, session->user, YMSG_SEP,
		"7", YMSG_SEP, friend, YMSG_SEP,
		"14", YMSG_SEP, "I would like to add you to my buddy list.", YMSG_SEP,
		"65", YMSG_SEP, selected_buddy_group?_b2loc(selected_buddy_group):"Buddies", YMSG_SEP,
		"216", YMSG_SEP, "", YMSG_SEP /* dont know what this is */
		);

	} else {
	snprintf( ptr, 400, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		"1", YMSG_SEP, session->user, YMSG_SEP,
		"7", YMSG_SEP, friend, YMSG_SEP,
		"14", YMSG_SEP, "I would like to add you to my buddy list.", YMSG_SEP,
		"65", YMSG_SEP, selected_buddy_group?_b2loc(selected_buddy_group):"Buddies", YMSG_SEP
		);
	}



	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );
	return( 0 );
}

int ymsg_remove_buddy( YMSG_SESSION *session, char *friend ) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_REM_BUDDY );
	int len;

	snprintf( ptr, 233, "%s%s%s%s%s%s%s%s%s%s%s%s",
		"1", YMSG_SEP, session->user, YMSG_SEP,
		"7", YMSG_SEP, friend, YMSG_SEP,
		"65", YMSG_SEP, selected_buddy_group?_b2loc(selected_buddy_group):"Buddies", YMSG_SEP
		);

	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );
	return( 0 );
}

int ymsg_goto( YMSG_SESSION *session, char *friend ) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_GOTO );
	int len;

	if (ycht_is_running()) { return ycht_goto( friend);}

	snprintf( ptr, 233,  "%s%s%s%s%s%s%s%s%s%s%s%s",
		"109", YMSG_SEP, friend, YMSG_SEP,
		"1", YMSG_SEP, select_profile_name(YMSG_GOTO, 0), YMSG_SEP,
		"62", YMSG_SEP, "2", YMSG_SEP
		);

	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );
	return( 0 );
}



int ymsg_typing( YMSG_SESSION *session, char *remote_user, int typing ) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_NOTIFY );
	int len;

	/* set the typing status field */
	*((u_long *)(ysendbuf+12)) = htonl( YMSG_STATUS_TYPING );

	snprintf( ptr, 350,   "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		"49", YMSG_SEP, "TYPING", YMSG_SEP, 
		"1", YMSG_SEP, select_profile_name(YMSG_NOTIFY, 1), YMSG_SEP,
		"14", YMSG_SEP, "", YMSG_SEP,
		"13", YMSG_SEP, typing ? "1" : "0", YMSG_SEP,
		"5", YMSG_SEP, remote_user, YMSG_SEP
		);

	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );
	return( 0 );
}



int ymsg_invite_with_mesg_extended( YMSG_SESSION *session, char *remote_user, char *room, char *msg ) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_INVITE );
	int len;	
	char mess_buf[120]="";
	char *mesg=NULL;
	mesg=msg;

	if (!mesg) {		
		snprintf(mess_buf,118, "%s: '%s'", "You are invited to join the room", room);
				} else {
		snprintf(mess_buf,118, "%s",mesg );
						 }

	snprintf( ptr, 378,   "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		"1", YMSG_SEP, session->user, YMSG_SEP,
		"118", YMSG_SEP, remote_user, YMSG_SEP,
		"104", YMSG_SEP, room, YMSG_SEP,
		"117", YMSG_SEP, mess_buf, YMSG_SEP,
		"129", YMSG_SEP, "0", YMSG_SEP
		);

	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );
	return( 0 );
}


int ymsg_invite_with_mesg( YMSG_SESSION *session, char *remote_user, char *room, char *msg ) {
	if (is_conference) {
		return ymsg_conference_addinvite( session, remote_user, NULL);
						  }
	return ymsg_invite_with_mesg_extended(session,remote_user,room,msg);
}

int ymsg_invite( YMSG_SESSION *session, char *remote_user, char *room ) {
	return ymsg_invite_with_mesg( session, remote_user, room, NULL);
}


int ymsg_send_file( YMSG_SESSION *session, char *remote_user, char *msg,
		char *filename ) {
	u_char *ptr =NULL;
	int len;
	int sock = -1 ;
	struct sockaddr_in sa;
	struct hostent *hinfo;
	char url[1024];
	struct stat sbuf;
	int fd;
	int bytes;
	int sock_res;
	char neatmsg[256]="";
	char neatfile[256]="";
	char *neater=NULL;

	if (! file_transfer_server) {file_transfer_server=strdup("filetransfer.msg.yahoo.com");}
	reset_ysend_buf();

	if (is_bimage_upload==5) {  /* Turn this off pending bug fix - FIXED!   */
		ptr = ymsg_header( session, ysendbuf, YMSG_BIMAGE_SEND );
	} else {
		ptr = ymsg_header( session, ysendbuf, YMSG_FILETRANSFER );
	}

			if ( capture_fp) {	
				time_t time_llnow = time(NULL);
				fprintf(capture_fp, "\n%s\nFILE-TRANSFER STARTING:\nFile: %s\n\n", ctime(&time_llnow), filename);
				fflush( capture_fp );
			}

	if ( stat( filename, &sbuf )) {
			/* file doesn't exist so exit */  
			/* changed: PhrozenSmoke, use dialog message, not stderr */
		snprintf(url, 500,  "%s:\n%s", _("File does not exist"), filename );
		show_ok_dialog(url);
		reset_ysend_buf();
		/* fprintf( stderr, "File '%s' doesn't exist\n", filename ); fflush( stderr );  */
		return( 0 );
	}

	fd = open( filename, O_RDONLY, 0600 );
	if ( fd == -1 ) {
		/* can't open file */  /* changed: PhrozenSmoke, use dialog message */
		snprintf(url, 500, "%s:\n%s", _("File could not be opened"), filename );
		show_ok_dialog(url);
		reset_ysend_buf();
		/* fprintf( stderr, "Can't open File '%s'\n", filename ); fflush( stderr );  */
		return( 0 );
	}

	/* Limit to files 250kb and under */
	if ( ((int)sbuf.st_size) > 250000) {
		snprintf(url, 500, "%s:\n%s\n\n%s: 250kb", _("File is too large"), filename, 
			_("Maximum Allowed File Size")		);
		show_ok_dialog(url);
		reset_ysend_buf();
		return( 0 );
	}

	snprintf(neatfile, 255,"%s", filename);
	neater =strrchr(neatfile,'/');
	if (neater) {neater++; if (! *neater) {neater=NULL;} } /* cut directory stuff out of file name */
	snprintf(neatmsg, 255, "File Sent: %s", neater?neater:filename);


	if (is_bimage_upload==5) {  /* Turn this off pending bug fix */

	char meme[64];
	snprintf(meme, 62, "%s",  session->user);
	lower_str(meme);
		/* Try to look like a windows file lol */
	snprintf(neatmsg, 255, "%s%s", "C%%3a\\PROGRAM+FILES\\YAHOO!\\MESSENGER\\Media\\FriendIcon\\", neater?neater:filename);

	snprintf( ptr, 768, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%d%s%s%s%s%s%s%s%s%s%s%s",
		"1", YMSG_SEP,select_profile_name(YMSG_FILETRANSFER, 0), YMSG_SEP,
		"38", YMSG_SEP, "604800", YMSG_SEP,
		"0", YMSG_SEP, select_profile_name(YMSG_FILETRANSFER, 0), YMSG_SEP,
		"28", YMSG_SEP, (int)sbuf.st_size, YMSG_SEP,
		"27", YMSG_SEP, neatmsg, YMSG_SEP,
		"14", YMSG_SEP, "", YMSG_SEP,
		"29", YMSG_SEP
		);
	} else {
	snprintf( ptr, 768, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%d%s%s%s",
		"0", YMSG_SEP, select_profile_name(YMSG_FILETRANSFER, 0), YMSG_SEP,
		"5", YMSG_SEP, remote_user, YMSG_SEP,
		"14", YMSG_SEP, neatmsg, YMSG_SEP,

		"27", YMSG_SEP, 		is_bimage_upload?"C%%3a\\PROGRAM+FILES\\YAHOO!\\MESSENGER\\Media\\FriendIcon\\":"", neater?neater:filename, YMSG_SEP,

		"28", YMSG_SEP, (int)sbuf.st_size, YMSG_SEP,
		"29", YMSG_SEP
		);
	}


	len = 20 + strlen( ptr ) + sbuf.st_size;

	*((u_short *)(ysendbuf+8)) = htons( len-20); 
	 if (is_bimage_upload==5) {*((u_short *)(ysendbuf+8)) = htons( strlen( ptr ) +4); }   

	*((u_long *)(ysendbuf+16)) =	htonl( session->session_id );
	check_proxy_config();


	 set_socket_timer(sock,10);

	if ( use_proxy ) {
		hinfo = gethostbyname( proxy_host );
	} else {
		hinfo = gethostbyname( file_transfer_server );
		}

	 set_socket_timer(sock,-1);

	if (!  hinfo)     {  /* added: PhrozenSmoke, to avoid core dumps */
		snprintf(url, 300, "%s:\n%s", _("Could not connect to server"), file_transfer_server );
		show_ok_dialog(url);
		reset_ysend_buf();
		return( 0 );
					   }

	memset( &sa, 0, sizeof(sa));
	memmove((void*)&sa.sin_addr.s_addr, hinfo->h_addr, hinfo->h_length );

	sa.sin_family = AF_INET;

	if ( use_proxy ) {
		sa.sin_port = htons( proxy_port );
	} else {
	sa.sin_port = htons( 80 );
		}


	sock = socket( AF_INET, SOCK_STREAM, 6 );

	if (sock==-1) {  /* added: PhrozenSmoke, to avoid core dumps */
		snprintf(url, 300, "%s:\n%s", _("Could not connect to server"), file_transfer_server );
		show_ok_dialog(url);
		reset_ysend_buf();
		return( 0 );
					   }

	 set_socket_timer(sock,7);
	sock_res=connect( sock, (struct sockaddr*)&sa, sizeof(sa));

	if (sock_res==-1) {  /* added: PhrozenSmoke, to avoid core dumps */
		snprintf(url, 300, "%s:\n%s", _("Could not connect to server"), file_transfer_server );
		show_ok_dialog(url);
		 set_socket_timer(sock,-1);
		close(sock);
		reset_ysend_buf();
		return( 0 );
							   }

	if (is_bimage_upload==5) {  /* Turn this off pending bug fix */
	char *cbreak=NULL;
	char *cookies=NULL;
	char *cookies1=NULL;
	int appended=0;
	char cookbox[720];
	sprintf(cookbox,"%s", "");
	cookies1=strdup(session->cookie);
	cookies=cookies1;
	cbreak = strchr(cookies,' ');
	while (cbreak) {
		*cbreak='\0';
		if ( (! strncmp(cookies, "Y=", 2)) || (! strncmp(cookies, "T=", 2))) {
			if (appended) {strcat(cookbox, " "); strncat(cookbox, cookies, 350 );}
			else {  strncat(cookbox, cookies, 350 );}
			appended=1;
		}
		cbreak++;
		cookies=cbreak;
		cbreak = strchr(cookies,' ');
	}
	if ( (! strncmp(cookies, "Y=", 2)) || (! strncmp(cookies, "T=", 2))) {
			if (appended) {strcat(cookbox, " "); strncat(cookbox, cookies, 350 );}
			else {  strncat(cookbox, cookies, 350 );}
		}
	cbreak=strrchr(cookbox,';');
	if (cbreak) {*cbreak='\0';}

	snprintf (neatmsg, 254, "%s%s/notifyft", 
		use_proxy?"http://":"",
		use_proxy?file_transfer_server:"" );

	snprintf( url, 900,
		"POST %s HTTP/1.0\r\n"
		"Content-length: %d\r\n"
		"Host: %s:80\r\n"
		"Cookie: %s\r\n\r\n",
		neatmsg,
		len, file_transfer_server , cookbox
		  ); 
	if (cookies1) {free(cookies1);}

	} else {
				/* You can also use the old 'notifyt' URL ...
						"http://filetransfer.msg.yahoo.com/notifyt" 
				*/ 
	snprintf (neatmsg, 254, "%s%s/notifyft", 
		use_proxy?"http://":"",
		use_proxy?file_transfer_server:"" );

	snprintf( url, 350,
		"POST %s HTTP/1.0\r\n"
		"Content-length: %d\r\n"
		"Accept: text/html\r\n"
		"Accept: text/plain\r\n"
		"User-Agent: %s\r\n"      /* changed: PhrozenSmoke */
		"Host: %s\r\n",
		neatmsg,
		len,
		GYACH_USER_AGENT, 
		file_transfer_server ); 
	}

	if (is_bimage_upload != 5) { 
		strcat( url, "Cookie: " );
		strncat( url, session->cookie, 625 );
		strcat( url, "\r\n" );
		strcat( url, "\r\n" );
	}

	set_socket_timer(sock,-1);
	set_socket_timer(sock,10);
	write( sock, url, strlen( url ));

			if ( capture_fp) {	
				time_t time_llnow = time(NULL);
				fprintf(capture_fp, "\n%s\nFILE-TRANSFER HTTP Data Sent:\nData: %s\n\n", ctime(&time_llnow), url);
				fflush( capture_fp );
			}

	write( sock, ysendbuf, 20 + strlen( ptr ));

			if ( capture_fp) {	
				time_t time_llnow = time(NULL);
				int pcounter=0;
				int stopsize=20 + strlen( ptr );
				fprintf(capture_fp, "\n%s\nOUTGOING PACKET SENT (FileTransferSystem):\nSize: %d\nData: [", ctime(&time_llnow), stopsize);
				while (pcounter<stopsize){
					if (ysendbuf[pcounter]) {fprintf(capture_fp, "%c", ysendbuf[pcounter]);}
					pcounter++;
				}
				fprintf(capture_fp, "]\n\n");
				fflush( capture_fp );
			}

	/* now read the file in and send it */

	reset_ysend_buf();
		
	bytes = read( fd, ysendbuf, 512 );
	while( bytes ) {
		 set_socket_timer(sock,-1);
		 set_socket_timer(sock,8);
		write( sock, ysendbuf, bytes );
		bytes = read( fd, ysendbuf, 512 );
	}

	 set_socket_timer(sock,-1);

	if (is_bimage_upload==5) { 
		set_socket_timer(sock,8);
		reset_ysend_buf();
		bytes = read( sock, ysendbuf, 512 );
		while( bytes ) {
			/* printf("bytes: %d\n", bytes); fflush(stdout); */ 
		 	set_socket_timer(sock,-1);
		 	set_socket_timer(sock,8);
			bytes = read( sock, ysendbuf, 512 );
		}
	}

	 set_socket_timer(sock,-1);
	 set_socket_timer(sock,6);

	close( fd );
	close( sock );

	set_socket_timer(sock,-1);

	/* added: PhrozenSmoke */
	if (! is_bimage_upload) {
		snprintf(url, 348,  "%s:\n%s", _("File successfully sent"), filename );
		show_ok_dialog(url);
	}

	return( 0 );
}




/* added: PhrozenSmoke, for creating rooms ourselves */


int ymsg_create_room( YMSG_SESSION *session, char *category, char *room, char *topic, char *attribs ) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_CREATE_ROOM);
	int len;

	snprintf( ptr, 768, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		"1", YMSG_SEP, select_profile_name(YMSG_CREATE_ROOM, 0), YMSG_SEP,
		"128", YMSG_SEP, category, YMSG_SEP,
		"104", YMSG_SEP, room, YMSG_SEP,
		"129", YMSG_SEP, category, YMSG_SEP, 
		"105", YMSG_SEP, topic, YMSG_SEP,
		"126", YMSG_SEP, attribs, YMSG_SEP,
		"62", YMSG_SEP, "2", YMSG_SEP
		);

	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );	
	return( 0 );
}



/* added: PhrozenSmoke, just sents a basic response to YMSG_P2PASK telling Yahoo we can accept 
     a peer-to-peer transfer.  All that really happens is the other user's Yahoo program starts 
     an HTTP server on their end, places the file on their http server, and sends us the link on the http 
     server to download the file from. When a peer-to-peer file is sent to us, we will send Yahoo a 
     packet basically telling Yahoo we are behind a firewall and need Yahoo to use the file transfer 
     system instead - clicking directly on peer-to-peer links doesnt seem to work, and also using 
     peer to peer reveals our IP address to the other user - it safer to force the user to send the file 
     to the file transfer system and we can download it from a Yahoo web server  */

int ymsg_p2p_ack( YMSG_SESSION *session, char *who ) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_P2PASK);
	int len;
	snprintf( ptr, 255, "%s%s%s%s%s%s%s%s%s%s%s%s",
		"5", YMSG_SEP, who, YMSG_SEP,
		"4", YMSG_SEP, select_profile_name(YMSG_P2PASK, 1), YMSG_SEP,
		"11", YMSG_SEP, "-198274379", YMSG_SEP
		);
	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );	
	return( 0 );
}



int ymsg_p2p_redirect( YMSG_SESSION *session, char *who, char *short_file, char *long_file ) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_P2P);
	int len;

	snprintf( ptr, 428, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		"5", YMSG_SEP, who , YMSG_SEP,
		"49", YMSG_SEP, "FILEXFER", YMSG_SEP,
		"1", YMSG_SEP, select_profile_name(YMSG_P2P, 1) , YMSG_SEP,
		"4", YMSG_SEP, select_profile_name(YMSG_P2P, 1) , YMSG_SEP,
		"13", YMSG_SEP, "3", YMSG_SEP,
		"27", YMSG_SEP, short_file, YMSG_SEP,
		"53", YMSG_SEP, long_file, YMSG_SEP,
		"11", YMSG_SEP, "3", YMSG_SEP
		);

	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );	
	return( 0 );
}


/* added: PhrozenSmoke - reject a person trying to add us as a buddy */

int ymsg_reject_buddy_cb( YMSG_SESSION *session, char *who ) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_REJECTBUDDY);
	int len;

	snprintf( ptr, 768, "%s%s%s%s%s%s%s%s%s%s%s%s",
		"1", YMSG_SEP, select_profile_name(YMSG_REJECTBUDDY, 0) , YMSG_SEP,
		"7", YMSG_SEP, who, YMSG_SEP,
		"14", YMSG_SEP, "Thanks, but no thanks.\nIf you are receiving this message, it means either\nI do NOT wish to be added to your buddy list, or I\nam REMOVING my name from your buddy list.\nPlease do not add or re-add my name to\nyour buddy list without my permission. Thank you.", YMSG_SEP
		);

	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );	
	return( 0 );
}


int ymsg_reject_buddy( YMSG_SESSION *session, char *who ) {
	deny_buddy_all_profile_names(who);
	return( 0 );
}



/*  ADDED: PhrozenSmoke
      Conferences Stuff
*/

/* for sending voice chat invitations to PM users */

int ymsg_voice_invite( YMSG_SESSION *session, char *who, char *room ) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_VOICECHAT);
	int len;

	snprintf( ptr, 333,  "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		"5", YMSG_SEP, who, YMSG_SEP,
		"4", YMSG_SEP, select_profile_name(YMSG_VOICECHAT, 1), YMSG_SEP,
		"57", YMSG_SEP, room, YMSG_SEP,
		"13", YMSG_SEP, "1", YMSG_SEP
		);

	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );	
	return( 0 );
}


char *get_random_conference_name( YMSG_SESSION *session) {
		char rroom[64]="";
		char *randroom=rroom;
		conference_counter++;  /* create a new conference */
		snprintf(rroom, 63, "%s-%d", get_current_chat_profile_name(), conference_counter);
		return randroom;
}


int ymsg_conference_join( YMSG_SESSION *session) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_CONFLOGON);
	int len;

	ymsg_conference_leave(session);

	snprintf( ptr,128, "%s%s%s%s",
		"1", YMSG_SEP, select_profile_name(YMSG_CONFLOGON, 0), YMSG_SEP
		);

	strncat(ptr,  get_conference_member_list(YMSG_SEP, "3"), 2700);

	if (session->room && (strlen(session->room)>0))  {
	strcat(ptr,  "57");
	strcat(ptr,  YMSG_SEP);
	strncat(ptr,session->room, 64);
	strcat(ptr,  YMSG_SEP);
											}  else   {
		char *rroom=get_random_conference_name(session);
		strcat(ptr,  "57");
		strcat(ptr,  YMSG_SEP);
		strncat(ptr, rroom, 64);
		strcat(ptr,  YMSG_SEP);
		strncpy( session->room, rroom,62 );
							  						}

	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );	
	return( 0 );
}



int ymsg_conference_addinvite( YMSG_SESSION *session, char *who, char *msg) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_CONFADDINVITE);
	int len;
	char *mesg=NULL;
	mesg=msg;
	if (!mesg) {
		char mess_buf[225]="";
		snprintf(mess_buf, 223, "%s   [%s]", "You are invited to join my conference.", _("You are invited to join my conference."));
		mesg=mess_buf;
				}

	 snprintf( ptr, 255, "%s%s%s%s%s%s%s%s",
		"1", YMSG_SEP, select_profile_name(YMSG_CONFADDINVITE, 0), YMSG_SEP,
		"51", YMSG_SEP, who, YMSG_SEP
		);  

	//strcat(ptr,  get_conference_invite_list(YMSG_SEP, "51"));

	strcat(ptr,  "57");
	strcat(ptr,  YMSG_SEP);
	strncat(ptr, session->room, 70);
	strcat(ptr,  YMSG_SEP);

	strncat(ptr,  get_conference_member_list(YMSG_SEP, "52"), 1400);
	strncat(ptr,  get_conference_member_list(YMSG_SEP, "53"), 1400);

	/* check for problems */
	strncat(ptr,  get_conference_invite_list(YMSG_SEP, "52"), 600);
	strncat(ptr,  get_conference_invite_list(YMSG_SEP, "53"), 600);

	strcat(ptr,  "58");
	strcat(ptr,  YMSG_SEP);
	strncat(ptr, mesg,256);
	strcat(ptr,  YMSG_SEP);

	strcat(ptr,  "13");
	strcat(ptr,  YMSG_SEP);
	strcat(ptr, "0");
	strcat(ptr,  YMSG_SEP);


	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );	

	return( 0 );
}



int ymsg_conference_invite( YMSG_SESSION *session, char *who, char *msg) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_CONFINVITE);
	int len;
	char *rroom;
	char *mesg=NULL;
	mesg=msg;

	if (is_conference && session->room) {
		ymsg_conference_addinvite(session,who,msg);
		return 0;
							      }
	
	if (!mesg) {
		char mess_buf[225]="";
		snprintf(mess_buf,223, "%s   [%s]", "You are invited to join my conference.", 
			_("You are invited to join my conference."));
		mesg=mess_buf;
				}

	snprintf( ptr, 255, "%s%s%s%s%s%s%s%s",
		"50", YMSG_SEP, select_profile_name(YMSG_CONFINVITE, 0), YMSG_SEP,
		"1", YMSG_SEP, who, YMSG_SEP
		);

		rroom=get_random_conference_name(session);
		strcat(ptr,  "57");
		strcat(ptr,  YMSG_SEP);
		strncat(ptr, rroom, 70);
		strcat(ptr,  YMSG_SEP);
		strncpy(session->room, rroom , 62);


	strncat(ptr,  get_conference_invite_list(YMSG_SEP, "52"), 2500);

	strcat(ptr,  "58");
	strcat(ptr,  YMSG_SEP);
	strncat(ptr, mesg, 256);
	strcat(ptr,  YMSG_SEP);

	strcat(ptr,  "13");
	strcat(ptr,  YMSG_SEP);
	strcat(ptr, "0");
	strcat(ptr,  YMSG_SEP);

	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );	

	return( 0 );
}


int ymsg_conference_decline( YMSG_SESSION *session, char *room, char *who_list, char *msg) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_CONFDECLINE);
	int len;
	int i=0;
	char mess_buf[255]="";
	char *end;
	char *ptrr;
	int last;
	char buff[80]="";
	char *mesg=NULL;
	mesg=msg;
	if (!room) {return 1;}
	if (!who_list) {return 1 ;}

	if (!mesg) {
		snprintf(mess_buf,253, "%s   [%s]", "Thanks, but no thanks.", 
			_("Thanks, but no thanks."));
		mesg=mess_buf;
				}

	snprintf( ptr, 128, "%s%s%s%s",
		"1", YMSG_SEP, select_profile_name(YMSG_CONFDECLINE, 0), YMSG_SEP
		);


					ptrr = who_list;
					last = 0;
					while( ptrr ) 	{
						end = strchr( ptrr, ',' );
						if ( end ) {
							*end = '\0';
						} else {
							last = 1;
								  }
						strncpy(buff,ptrr, 78);
						strcat(ptr,  "3");
						strcat(ptr,  YMSG_SEP);
						strcat(ptr, buff);
						strcat(ptr,  YMSG_SEP);	
						i++;
						if ( last ) {
							break;
						} else {
							ptrr = end + 1;
								  }
										}
	

	if (i==0) {   /* create a new conference */
		strcat(ptr,  "3");
		strcat(ptr,  YMSG_SEP);
		strncat(ptr, select_profile_name(YMSG_CONFDECLINE, 0), 66);
		strcat(ptr,  YMSG_SEP);	
				}

	strcat(ptr,  "57");
	strcat(ptr,  YMSG_SEP);
	strncat(ptr, room, 101);
	strcat(ptr,  YMSG_SEP);

	strcat(ptr,  "14");
	strcat(ptr,  YMSG_SEP);
	strncat(ptr, mesg, 512);
	strcat(ptr,  YMSG_SEP);

	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );	

	return( 0 );
}



int ymsg_conference_msg( YMSG_SESSION *session, char *msg) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_CONFMSG);
	int len;

	if (!session->room) {return 1;}
	if (strlen(session->room)==0) {return 1;}
	if (!msg) {return 1;}

	snprintf( ptr, 96, "%s%s%s%s",
		"1", YMSG_SEP, select_profile_name(YMSG_CONFMSG, 0), YMSG_SEP
		);

	strncat(ptr,  get_conference_member_list(YMSG_SEP, "53"), 2450);

	strcat(ptr,  "57");
	strcat(ptr,  YMSG_SEP);
	strncat(ptr, session->room,70);
	strcat(ptr,  YMSG_SEP);

	strcat(ptr,  "14");
	strcat(ptr,  YMSG_SEP);
	strncat(ptr, msg, 1536);
	strcat(ptr,  YMSG_SEP);

	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );	
	return( 0 );
}


/* called back automatically */

int ymsg_conference_leave( YMSG_SESSION *session) {
	if (is_conference && last_conference) {

	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_CONFLOGOFF);
	int len;

	if (strlen(last_conference)==0) {return 1;}

	snprintf( ptr, 128, "%s%s%s%s",
		"1", YMSG_SEP, select_profile_name(YMSG_CONFLOGOFF, 0), YMSG_SEP
		);

	strncat(ptr,  get_conference_member_list(YMSG_SEP, "3"), 2525);

	strcat(ptr,  "57");
	strcat(ptr,  YMSG_SEP);
	strncat(ptr, last_conference, 70);
	strcat(ptr,  YMSG_SEP);

	if (emulate_ymsg6) {
		strcat(ptr,  "1005");
		strcat(ptr,  YMSG_SEP);
		strcat(ptr, "10887872");
		strcat(ptr,  YMSG_SEP);
	}

	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );	
	if (last_conference) {free(last_conference); last_conference=NULL;}
											}  
	return( 0 );
}


/* Send our User Stat (value 0xa - 10) when refreshing the buddy list, added PhrozenSmoke */
/* Refresh the buddy list and cookie by requesting a new cookie, added PhrozenSmoke */

int ymsg_refresh( YMSG_SESSION *session) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_USERSTAT);
	int len;
	snprintf( ptr, 3, "%s","");
	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );	
	return( 0 );
}


/* added PhrozenSmoke, code for leaving a chat room and literally just being 'nowhere' */

int ymsg_leave_chat( YMSG_SESSION *session ) {

	if (ycht_is_running()) { return ycht_logout(0 );}

	if (is_conference && last_conference) {
		ymsg_conference_leave(session);
		is_conference=0;
		in_a_chat=0;
		chatter_list_remove(session->user);
		session->pkt.type=YMSG_GYE_CONFEXIT;
		show_yahoo_packet();
		return (0);
						                } 

	if (in_a_chat) {
		u_char *ptr = ymsg_header( session, ysendbuf, YMSG_EXIT);
		int len;

	snprintf( ptr, 233, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		"1", YMSG_SEP, select_profile_name(YMSG_EXIT, 0), YMSG_SEP,
		"104", YMSG_SEP, session->room, YMSG_SEP,
		"109", YMSG_SEP, select_profile_name(YMSG_EXIT, 0), YMSG_SEP,
		"62", YMSG_SEP, "0", YMSG_SEP
		);

		len = 20 + strlen( ptr );
		ymsg_send_packet( session, ysendbuf, len );	
		chatter_list_remove(session->user);
		ymsg_logout(session);
		is_conference=0;
		in_a_chat=0;
		return (0);
			    }

	//strcpy(session->room,_("NONE"));
	return( 0 );
}



int ymsg_contact_info( YMSG_SESSION *session, char *remote_user, char *info ) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_NOTIFY );
	int len;

	/* set the typing status field */
	*((u_long *)(ysendbuf+12)) = htonl( YMSG_STATUS_TYPING );

	snprintf( ptr, 1500, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		"1", YMSG_SEP, select_profile_name(YMSG_NOTIFY, 0), YMSG_SEP,
		"5", YMSG_SEP, remote_user, YMSG_SEP,
		"13", YMSG_SEP, "0", YMSG_SEP,
		"14", YMSG_SEP,info, YMSG_SEP,
		"49", YMSG_SEP, "CONTACTINFO", YMSG_SEP,
		"1002", YMSG_SEP, "1", YMSG_SEP
		);

	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );
	return( 0 );
}


/* added PhrozenSmoke: handling of Yahoo's permenant ignore list */

int ymsg_perm_ignore( YMSG_SESSION *session, char *who, int remove_ignore ) {
	char which_igg[4]="";
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_TOGGLEIGG );
	int len;

	if (remove_ignore) {strcpy(which_igg,"2");} else {strcpy(which_igg,"1");}

	snprintf( ptr, 233, "%s%s%s%s%s%s%s%s%s%s%s%s",
		"1", YMSG_SEP, session->user, YMSG_SEP,
		"7", YMSG_SEP, who, YMSG_SEP,
		"13", YMSG_SEP, which_igg, YMSG_SEP
		);

	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );
	return( 0 );
}

/* send notify of IMVironment setting: not yet used except for pre-emptive strikes: PhrozenSmoke */

int ymsg_imvironment( YMSG_SESSION *session, char *who, char *imvironment) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_P2P);
	int len;

	if (!strcmp(imvironment,"gotbootZ")) {
	snprintf( ptr, 500, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
	"5", YMSG_SEP,who, YMSG_SEP,
	"4", YMSG_SEP,session->user, YMSG_SEP, 
	"0" , YMSG_SEP,session->user, YMSG_SEP,
	"14", YMSG_SEP,"SCR|PT |N H3R3", YMSG_SEP,
	"49", YMSG_SEP, "IMVIRONMENT", YMSG_SEP,
	"14", YMSG_SEP, "SCR|PT |N H3R3", YMSG_SEP,
	"97" , YMSG_SEP, "1", YMSG_SEP,
	"63" , YMSG_SEP,"" , YMSG_SEP, 
	"64", YMSG_SEP, "6", YMSG_SEP,
	"0" , YMSG_SEP, "202", YMSG_SEP, 
	"140" , YMSG_SEP, "1", YMSG_SEP,
	"202" , YMSG_SEP,"140", YMSG_SEP,
	"0" , YMSG_SEP,"140", YMSG_SEP,
	"140", YMSG_SEP, "1", YMSG_SEP,"", YMSG_SEP
			); 

	} else {
	snprintf( ptr, 300, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		"49", YMSG_SEP, "IMVIRONMENT", YMSG_SEP,
		"1", YMSG_SEP, select_profile_name(YMSG_P2P, 1), YMSG_SEP,
		"14", YMSG_SEP, " ", YMSG_SEP,
		"13", YMSG_SEP, "0", YMSG_SEP,
		"5", YMSG_SEP, who, YMSG_SEP,
		"63", YMSG_SEP, imvironment, YMSG_SEP,
		"64", YMSG_SEP, "0", YMSG_SEP
		);
	}

	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );
	return( 0 );
}

int ymsg_photo_imvironment( YMSG_SESSION *session, char *who, char *msg, int ptype) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_P2P);
	int len;
	snprintf( ptr, 600, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%d%s%s%s%s%s%s%s%s%s%s%s%s%s",
		"49", YMSG_SEP, "IMVIRONMENT", YMSG_SEP,
		"1", YMSG_SEP, select_profile_name(YMSG_P2P, 1), YMSG_SEP,
		"14", YMSG_SEP, msg?msg:"null", YMSG_SEP,
		"13", YMSG_SEP, ptype, YMSG_SEP,
		"5", YMSG_SEP, who, YMSG_SEP,
		"63", YMSG_SEP, "photos;20", YMSG_SEP,
		"64", YMSG_SEP, "1", YMSG_SEP
		);
	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );
	return( 0 );
}


int ymsg_get_webcam( YMSG_SESSION *session, char *who) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_WEBCAM);
	int len;

	if (who != NULL) {
	snprintf( ptr, 168, "%s%s%s%s%s%s%s%s",
		"1", YMSG_SEP, select_profile_name(YMSG_WEBCAM, 0), YMSG_SEP,
		"5", YMSG_SEP, who, YMSG_SEP
		);
						} else {
	snprintf( ptr, 233, "%s%s%s%s",
		"1", YMSG_SEP, select_profile_name(YMSG_WEBCAM, 0), YMSG_SEP
		);
								  }

	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );	
	return( 0 );
}


int ymsg_webcam_invite_packet( YMSG_SESSION *session, char *who, int which) {
	u_char *ptr = ymsg_header( session, ysendbuf,YMSG_NOTIFY);
	int len;
	char *invtype=" ";  /* send invite */
	if (which==1) {invtype="1";}  /* accept */
	if (which==2) {invtype="-1";}  /* reject invite */

	/* set the typing status field */
	*((u_long *)(ysendbuf+12)) = htonl( YMSG_STATUS_TYPING );

	snprintf( ptr, 233, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		"49", YMSG_SEP,"WEBCAMINVITE", YMSG_SEP,
		"14", YMSG_SEP, invtype, YMSG_SEP,
		"13", YMSG_SEP, "0", YMSG_SEP,
		"1", YMSG_SEP, select_profile_name(YMSG_NOTIFY, 0), YMSG_SEP,
		"5", YMSG_SEP, who, YMSG_SEP
		);

	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );	
	return( 0 );
}


int ymsg_webcam_invite( YMSG_SESSION *session, char *who) {
	return ymsg_webcam_invite_packet( session, who, 0);
}

int ymsg_webcam_invite_accept( YMSG_SESSION *session, char *who) {
	return ymsg_webcam_invite_packet( session, who, 1);
}

int ymsg_webcam_invite_reject( YMSG_SESSION *session, char *who) {
	return ymsg_webcam_invite_packet( session, who, 2);
}



/* This helps us not get booted SOMETIMES when the boot method is conference messages */

int ymsg_conference_msg_echo( YMSG_SESSION *session) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_CONFMSG);
	int len;
	/* This always returns a conference error message */
	snprintf( ptr, 233, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		"1", YMSG_SEP, session->user, YMSG_SEP,
		"53", YMSG_SEP, "YahooMessengerTeam", YMSG_SEP,
		"57", YMSG_SEP, "confbpstayon", YMSG_SEP,
		"14", YMSG_SEP, "None", YMSG_SEP
		);
	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );	
	return( 0 );
}


/* added, PhrozenSmoke, notification packets to start/stop encrypted sessions */

int ymsg_encryption_notify( YMSG_SESSION *session, char *who, int which) {
	u_char *ptr = ymsg_header( session, ysendbuf,YMSG_NOTIFY);
	int len;
	char notstr[30]="";
	char invtype[6]="";

	sprintf(invtype, "%s", " ");
	if (which>0) {snprintf(invtype, 4, "%d",which);} 

	snprintf(notstr,28,"%s", ENCRYPTION_OFF);
	if (which<0) {snprintf(notstr,28,"%s", ENCRYPTION_UNAVAIL);}
	if (which>0) {snprintf(notstr,28,"%s", ENCRYPTION_START_TAG);}

	*((u_long *)(ysendbuf+12)) = htonl( YMSG_STATUS_TYPING );

	snprintf( ptr, 233, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		"49", YMSG_SEP,notstr, YMSG_SEP,
		"14", YMSG_SEP, invtype, YMSG_SEP,
		"13", YMSG_SEP, "0", YMSG_SEP,
		"1", YMSG_SEP, select_profile_name(YMSG_NOTIFY, 1), YMSG_SEP,
		"5", YMSG_SEP, who, YMSG_SEP
		);

	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );	
	return( 0 );
}

int ymsg_encryption_gpgme_notify( YMSG_SESSION *session, char *who, int which) {
	u_char *ptr = ymsg_header( session, ysendbuf,YMSG_NOTIFY);
	int len;
	char invtype[6]="";
	snprintf(invtype, 4, "%d",which);

	*((u_long *)(ysendbuf+12)) = htonl( YMSG_STATUS_TYPING );

	snprintf( ptr, 233, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		"49", YMSG_SEP,ENCRYPTION_START_GPGME, YMSG_SEP,
		"14", YMSG_SEP, invtype, YMSG_SEP,
		"13", YMSG_SEP, "0", YMSG_SEP,
		"1", YMSG_SEP, select_profile_name(YMSG_NOTIFY, 1), YMSG_SEP,
		"5", YMSG_SEP, who, YMSG_SEP
		);

	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );	
	return( 0 );
}


int ymsg_idact( YMSG_SESSION *session, char *yahid) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_IDACT);
	int len;
	snprintf( ptr, 104, "%s%s%s%s",	"3", YMSG_SEP, yahid, YMSG_SEP);
	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );	
	return( 0 );
}
int ymsg_iddeact( YMSG_SESSION *session, char *yahid) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_IDDEACT);
	int len;
	snprintf( ptr, 104, "%s%s%s%s",	"3", YMSG_SEP, yahid, YMSG_SEP);
	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );	
	return( 0 );
}


int ymsg_group_rename( YMSG_SESSION *session, char *ngroup ) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_GROUP_RENAME);
	char *newgrp=NULL;
	int len;
	if (!selected_buddy_group) {return 0;}
	if (!ngroup) {return 0 ;}
	newgrp=g_strdup(_b2loc(ngroup));
	snprintf( ptr, 384,  "%s%s%s%s%s%s%s%s%s%s%s%s",
		"1", YMSG_SEP, ymsg_sess->user, YMSG_SEP,
		"65", YMSG_SEP, _b2loc(selected_buddy_group), YMSG_SEP,
		"67", YMSG_SEP, newgrp, YMSG_SEP
		);

	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );	
	g_free(newgrp);
	return( 0 );
}

int ymsg_stealth( YMSG_SESSION *session, char *who, int visible ) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_STEALTH );
	int len;
	if (!emulate_ymsg6) {return (0);}
	snprintf( ptr, 400, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", 
		"1", YMSG_SEP, ymsg_sess->user, YMSG_SEP,
		"31", YMSG_SEP, visible?"1":"2", YMSG_SEP,
		"13", YMSG_SEP, "1", YMSG_SEP,
		"7", YMSG_SEP, who, YMSG_SEP
	);
	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );
	return( 0 );
}

int ymsg_stealth_perm( YMSG_SESSION *session, char *who, int visible ) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_STEALTH_PERM );
	int len;
	
	if (!emulate_ymsg6) {return (0);}
	snprintf( ptr, 400, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", 
		"1", YMSG_SEP, ymsg_sess->user, YMSG_SEP,
		"31", YMSG_SEP,visible?"2":"1" , YMSG_SEP,
		"13", YMSG_SEP, "2", YMSG_SEP,
		"7", YMSG_SEP, who, YMSG_SEP
	);
	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );
	return( 0 );
}


int ymsg_audible(YMSG_SESSION *session, char *pm_user, char *aud_file, char *aud_text, char  *aud_hash) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_AUDIBLE );
	int len;
	
	if (!emulate_ymsg6) {return (0);}
	snprintf( ptr, 768, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", 
		"1", YMSG_SEP, select_profile_name(YMSG_AUDIBLE, 1), YMSG_SEP,
		"5", YMSG_SEP,pm_user , YMSG_SEP,
		"230", YMSG_SEP, aud_file, YMSG_SEP,
		"231", YMSG_SEP, aud_text, YMSG_SEP,
		"232", YMSG_SEP, aud_hash, YMSG_SEP
	);

	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );
	return( 0 );
}


int ymsg_game_invite(YMSG_SESSION *session, char *who, char *game, int invite) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_GAME_INVITE );
	int len;
	
	if (!emulate_ymsg6)  {return (0);}
	if (invite<1) {return (0);}
	if (invite>4) {return (0);}
	/* Invite types:
		1=Invite, cookie will be created
		2=Accept invite
		3=Decline invite
		4=Invitation cancelled (by the original sender)
	*/

	if (invite==1) { /* send invitation, creating cookie */
	char cookie[28]="";
		/* I think these 'cookies' are 24 chars or random junk that always 
		end with two hyphens  */
	conference_counter++;
	snprintf(cookie, 22, "%c%s%d%c%s", who[0], "rf1hzMqlbav",conference_counter ,
		who[0] ,  "taV1pmDsgjiFtws7gtlq");
	strcat(cookie,"--");
	snprintf( ptr, 512, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", 
		"1", YMSG_SEP, select_profile_name(YMSG_GAME_INVITE, 1) , YMSG_SEP,
		"5", YMSG_SEP,who, YMSG_SEP,
		"180", YMSG_SEP, game, YMSG_SEP,
		"11", YMSG_SEP, cookie, YMSG_SEP, 
		"13", YMSG_SEP, "1", YMSG_SEP
		);

	} else {
	snprintf( ptr, 512, "%s%s%d%s%s%s%s%s%s%s%s%s%s%s%s%s", 
		"13", YMSG_SEP, invite, YMSG_SEP,
		"1", YMSG_SEP,select_profile_name(YMSG_GAME_INVITE, 1), YMSG_SEP,
		"5", YMSG_SEP, who, YMSG_SEP,
		"180", YMSG_SEP, game, YMSG_SEP
		);
	}

	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );
	return( 0 );
}


int ymsg_avatar_toggle(YMSG_SESSION *session, int avatar_on) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_AVATAR_TOGGLE);
	int len;	
	if (!emulate_ymsg6) {return (0);}
	snprintf( ptr, 200, "%s%s%s%s%s%s%s%s", 
		"1", YMSG_SEP, get_primary_screen_name(), YMSG_SEP,
		"213", YMSG_SEP,avatar_on?"1":"0" , YMSG_SEP
	);
	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );
	return( 0 );
}


int ymsg_bimage_toggle(YMSG_SESSION *session, char *who, int bimage) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_BIMAGE_TOGGLE);
	int len;	
	if (!emulate_ymsg6) {return (0);}
	if (bimage<0) {return (0);}
	if (bimage>2) {return (0);}
	/*  field 206:  0= no image, 1=avatar, 2=buddy image */
	snprintf( ptr, 200, "%s%s%s%s%s%s%s%s%s%s%d%s", 
		"1", YMSG_SEP, select_profile_name(YMSG_PM, 1), YMSG_SEP,
		"5", YMSG_SEP, who , YMSG_SEP,
		"206", YMSG_SEP,bimage, YMSG_SEP
	);
	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );
	return( 0 );
}

int ymsg_bimage_accept(YMSG_SESSION *session, char *who, int bimage) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_BIMAGE);
	int len;	
	if (!emulate_ymsg6) {return (0);}
	/* This method also used for REQUESTING buddy images */

	snprintf( ptr, 200, "%s%s%s%s%s%s%s%s%s%s%s%s", 
		"1", YMSG_SEP, select_profile_name(YMSG_PM, 1), YMSG_SEP,
		"5", YMSG_SEP, who , YMSG_SEP,
		"13", YMSG_SEP,bimage?"1":"0", YMSG_SEP
	);
	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );
	return( 0 );
}

int ymsg_bimage_notify(YMSG_SESSION *session, char *who, char *b_url, char *b_hash) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_BIMAGE);
	int len;	
	if (!emulate_ymsg6) {return (0);}
	/* Notify users that a buddy image is now available on Yahoo's servers */

	snprintf( ptr, 640, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", 
		"1", YMSG_SEP, select_profile_name(YMSG_PM, 1), YMSG_SEP,
		"5", YMSG_SEP, who, YMSG_SEP,
		"13", YMSG_SEP, "2", YMSG_SEP,
		"20", YMSG_SEP, b_url , YMSG_SEP,
		"192", YMSG_SEP,b_hash, YMSG_SEP
	);
	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );
	return( 0 );
}


int ymsg_bimage_update(YMSG_SESSION *session, char *who,char *b_hash) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_BIMAGE_UPDATED);
	int len;	
	if (!emulate_ymsg6) {return (0);}
	/* Notify  users and Yahoo that our buddy image has changed */

	if (!who) { /* broadcast change to yahoo */
	snprintf( ptr, 400, "%s%s%s%s%s%s%s%s%s%s%s%s", 
		"1", YMSG_SEP, get_primary_screen_name(), YMSG_SEP,
		"212", YMSG_SEP, "1", YMSG_SEP,
		"192", YMSG_SEP,b_hash, YMSG_SEP
		);
	} else { /* tell a specific user */
	snprintf( ptr, 400, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", 
		"1", YMSG_SEP, select_profile_name(YMSG_PM, 1), YMSG_SEP,
		"5", YMSG_SEP, who, YMSG_SEP,
		"212", YMSG_SEP, "1", YMSG_SEP,
		"192", YMSG_SEP,b_hash, YMSG_SEP
		);
	}

	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );
	return( 0 );
}

int ymsg_upload_bimage(YMSG_SESSION *session, char *filename) {
	/*  Buddy image upload method 1: uses regular filetransfer service
	     by sending a file to ourselves then sharing it with PM friends */ 
	is_bimage_upload=1;      

	/* Buddy image upload method 2: Broken: uses special buddy 
	    image filetransfer service */ 
	if (buddy_image_upload_method<1) { is_bimage_upload=5; }  

	ymsg_send_file( session, session->user, session->user, filename );
	is_bimage_upload=0;
	return( 0 );
}



int ymsg_p2p_ask( YMSG_SESSION *session, char *yahid, int probe) {
	u_char *ptr = ymsg_header( session, ysendbuf, YMSG_P2PASK);
	int len;
	snprintf( ptr, 700, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",	
		"5", YMSG_SEP, yahid, YMSG_SEP,
		 "4", YMSG_SEP, ymsg_sess->user, YMSG_SEP,  
		"1", YMSG_SEP, ymsg_sess->user, YMSG_SEP,
		/* "4", YMSG_SEP, ymsg_sess->user, YMSG_SEP, */
		"12", YMSG_SEP, probe?"P33RtoPE3Rchck==":"KP67PjtxGHY1Ww==", YMSG_SEP,
		"61", YMSG_SEP, "0", YMSG_SEP,
		"2", YMSG_SEP, "", YMSG_SEP,
		"13", YMSG_SEP, "0", YMSG_SEP,
		"49", YMSG_SEP, "PEERTOPEER", YMSG_SEP,
		"140", YMSG_SEP, "1", YMSG_SEP,
		"11", YMSG_SEP, "1487248657", YMSG_SEP
		);
	len = 20 + strlen( ptr );
	ymsg_send_packet( session, ysendbuf, len );	
	return( 0 );
}




