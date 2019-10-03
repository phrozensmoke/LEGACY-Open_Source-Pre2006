/*****************************************************************************
 * sounds.c
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
 * Erica Andrews, PhrozenSmoke ['at'] yahoo.com
 * added 8.31.2003, support for playing sound events on the ESound daemon
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
#include <expat.h>
#include <malloc.h>
#include "sounds.h"  /* added, PhrozenSmoke */


/* added, PhrozenSmoke, support for ESound sound events */
int my_system( char *command );
extern int sounds_opening_too_fast();
extern int  enable_sound_events;
extern int  enable_sound_events_pm;
extern FILE *capture_fp;
char *mp3_player=NULL;
int xml_aud_counter=0;


GYAUDIBLE gyache_auds[] = {  /* current capacity is 75 */ 
 { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, 
 { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, 
 { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, 
 { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, 

 { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, 
 { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, 
 { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, 
 { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, 

 { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, 
 { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, 
 { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, 
 { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, 

 { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, 
 { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, 
 { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, 
 { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, 

 { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, 
 { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, 
 { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, 
 { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, 

 { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, 
 { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, 
 { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, 
 { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, 

 { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }, { NULL, NULL, NULL, NULL }
};


int auds_callback_enc(void *enc_data, const XML_Char *name, XML_Encoding *info) {return 0;}
void auds_callback_cdata(void *user_data, const XML_Char *data, int len) {return;}
void auds_callback_end(void *user_data, const XML_Char *name) {	xml_aud_counter++;}

void auds_callback_start(void *user_data, const XML_Char *name, const XML_Char **attrs) {
	if (xml_aud_counter>71) {return;}
	if (strncmp((char*)name,"audible",7)==0) {
		if (!attrs) {return;}
		if (! *attrs) {return;}
		else {
			GYAUDIBLE *sm_ptr;
			XML_Char **cptr=(XML_Char **) attrs;
			gyache_audibles=gyache_auds;
			sm_ptr = &gyache_audibles[xml_aud_counter];
			while (cptr && *cptr) {
				if (strncmp(cptr[0],"yname",5)==0) {sm_ptr->aud_file=strdup(cptr[1]);}
				if (strncmp(cptr[0],"quote",5)==0) {sm_ptr->aud_text=strdup(cptr[1]);}
				if (strncmp(cptr[0],"ahash",5)==0) {sm_ptr->aud_hash=strdup(cptr[1]);}
				if (strncmp(cptr[0],"filename",8)==0) {sm_ptr->aud_disk_name=strdup(cptr[1]);}
				cptr += 2;
				}
		}
	}
}

int load_xml_audibles() {
	char filename[256];
	int fd;
	int bytes;
	XML_Parser p;
	void *buff;
	snprintf(filename,254,"%s/audibles/gyaudibles.xml",  PACKAGE_DATA_DIR);
	fd = open( filename, O_RDONLY, 0600 );
	if ( fd == -1 ) {return( 0 );	}
	p = XML_ParserCreate(NULL);      /* XML_ParserCreate( "iso-8859-1"); */
	XML_SetElementHandler(p, auds_callback_start, auds_callback_end);
	XML_SetCharacterDataHandler(p, auds_callback_cdata);
	XML_SetUnknownEncodingHandler(p, auds_callback_enc, NULL);
	XML_SetUserData(p, "");
		buff = XML_GetBuffer(p, 18432);		
		if (buff == NULL) {	return -1;}
		bytes = read( fd, (char *)buff, 18400 );
		close( fd ); 
		xml_aud_counter=0;
		if (! XML_ParseBuffer(p, bytes, bytes == 0)) { 	return 1;	}
	XML_ParserFree(p);	
	return 1;
}

void init_audibles() {
	GYAUDIBLE *sm_ptr;
	gyache_audibles=gyache_auds;
	sm_ptr = &gyache_audibles[0];
	if (! sm_ptr->aud_file) {load_xml_audibles(); }
}


int check_gy_audible( char *str ) {
	GYAUDIBLE *sm_ptr;
	if (!str) {return 0;}
	gyache_audibles=gyache_auds;
	sm_ptr = &gyache_audibles[0];
	while( sm_ptr->aud_file ) {
		if (!strncmp( str, sm_ptr->aud_file, strlen(sm_ptr->aud_file))) {
			return( 1 );
		}
		if (!strncmp( str, sm_ptr->aud_disk_name, strlen(sm_ptr->aud_disk_name))) {
			return( 1 );
		}
		sm_ptr++;
	}
	return( 0 );
}

char *play_audible(char *aud) {
	char audbuf[256];
	if (sounds_opening_too_fast()) {return NULL;}
	if ( capture_fp ) {	
				fprintf(capture_fp,"\nY! AUDIBLE ANIMATION: %s  [%s]\nUsing MP3 Player  Command for Audibles: '%s'\n", aud?aud:"None", 
				check_gy_audible(aud)?"Available":"Not Available",
				mp3_player?mp3_player:"mplayer");
				fflush( capture_fp );
		}
	if (!aud) {return NULL;}
	if (!check_gy_audible(aud)) {return NULL;}
	else {
		char *audy_file=NULL;
		audy_file=get_gy_audible_disk_name( aud );
		if (! audy_file) {return NULL;}
		snprintf(audbuf,254,"%s %s/audibles/%s.mp3 &", mp3_player?mp3_player:"mplayer", PACKAGE_DATA_DIR, audy_file);
		my_system( audbuf );
		free(audy_file);
	}
	return NULL;
}

char *get_gy_audible_disk_name( char *str ) {
	GYAUDIBLE *sm_ptr;
	gyache_audibles=gyache_auds;
	sm_ptr = &gyache_audibles[0];
	while( sm_ptr->aud_file ) {
		if (!strncmp( str, sm_ptr->aud_file, strlen(sm_ptr->aud_file))) {
			return strdup(sm_ptr->aud_disk_name);
		}
		if (!strncmp( str, sm_ptr->aud_disk_name, strlen(sm_ptr->aud_disk_name))) {
			return strdup(sm_ptr->aud_disk_name);
		}
		sm_ptr++;
	}
	return NULL;
}


char *get_gy_audible_hash( char *str ) {
	GYAUDIBLE *sm_ptr;
	gyache_audibles=gyache_auds;
	sm_ptr = &gyache_audibles[0];
	while( sm_ptr->aud_file ) {
		if (!strncmp( str, sm_ptr->aud_file, strlen(sm_ptr->aud_file))) {
			return strdup(sm_ptr->aud_hash);
		}
		sm_ptr++;
	}
	return NULL;
}

char *get_gy_audible_text( char *str ) {
	GYAUDIBLE *sm_ptr;
	gyache_audibles=gyache_auds;
	sm_ptr = &gyache_audibles[0];
	while( sm_ptr->aud_file ) {
		if (!strncmp( str, sm_ptr->aud_file, strlen(sm_ptr->aud_file))) {
			return strdup(sm_ptr->aud_text);
		}
		sm_ptr++;
	}
	return NULL;
}



#ifdef  SUPPORT_SOUND_EVENTS
	char *sound_events[7]={"buddon.raw","buddoff.raw","yahoomail.raw","pm.raw","buzz.raw","other.raw","reject.raw"};
#endif





/* alot of this code comes from the PyESD ESound library, modified to suit this application */



#ifdef  SUPPORT_SOUND_EVENTS

/* ******************************
  * PyESD
  * Copyright (c) 2003 Erica Andrews
  * <PhrozenSmoke ['at'] yahoo.com>
  *
  * License: GNU General Public License
  *
  * A Python-wrapped library for 
  * playing raw sound over the ESound
  * daemon. This library also facilitates 
  * recording raw sound from  
  * ESound. Also, this library allows you
  * get basic, useful information about 
  * the ESound Daemon being used.
  * for playback over ESD.  This library
  * uses highly modified versions of 
  * MPlayer's 'ao_esd.c' module and 
  * the 'main.c' module from Gnome's 
  * Vumeter application.
  *
****************************** */

/*
 * ao_esd - EsounD audio output driver for MPlayer
 *
 * Juergen Keil <jk@tools.de>
 *
 * This driver is distributed under the terms of the GPL
 *
 * TODO / known problems:
 * - does not work well when the esd daemon has autostandby disabled
 *   (workaround: run esd with option "-as 2" - fortunatelly this is
 *    the default)
 * - plays noise on a linux 2.4.4 kernel with a SB16PCI card, when using
 *   a local tcp connection to the esd daemon; there is no noise when using
 *   a unix domain socket connection.
 *   (there are EIO errors reported by the sound card driver, so this is
 *   most likely a linux sound card driver problem)
 */



#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <malloc.h>
#include <esd.h>

/*  ***************************************  */
/* BEGIN modified version of ao_esd.c from MPlayer */

#define	ESD_CLIENT_NAME	"PyESound"
#define	ESD_MAX_DELAY	(1.0f)	/* max amount of data buffered in esd (#sec) */

int esd_fd = -1;
int esd_play_fd = -1;
int esd_play_fdR = -1;   /*for recording */
int esd_latency;
int esd_bytes_per_sample;
unsigned long esd_samples_written;
int ao_samplerate=1;
int ao_bps;
struct timeval esd_play_start;
char *ao_subdevice="localhost";
int is_blocking=0;
int stop_playback=0;


/*  ***************************************  */
/* END modified version of ao_esd.c from MPlayer */

char PYESD_ERROR_MSG[255]="";

char *get_esd_error_msg() {return PYESD_ERROR_MSG;}

/*
 * open & setup audio device for playback
 * return: 1=success 0=fail
 * rate = 22050, 44100, etc.
 * channels is either 1 or 2
 * format is either 8 or 16
 */
int init_esd_play(int rate_hz, int channels, int format)
{
    esd_format_t esd_fmt;
    int bytes_per_sample;
    int fl;
    char *server = ao_subdevice;  /* NULL for localhost */

		if (esd_play_fd>0)  {return 1;}

    if (esd_fd < 0) {
	esd_fd = esd_open_sound(server);
	if (esd_fd < 0) {
	    snprintf(PYESD_ERROR_MSG, 253, 
		   "AO: [esd] esd_open_sound failed: %d\n",
		   errno);
	    return 0;
	}
    }


    esd_fmt = ESD_STREAM | ESD_PLAY;

    /* EsounD can play mono or stereo */
    ao_samplerate = rate_hz;

    switch (channels) {
    case 1:
	esd_fmt |= ESD_MONO;
	bytes_per_sample = 1;
	break;
    default:
	esd_fmt |= ESD_STEREO;
	bytes_per_sample = 2;
	break;
    }

    /* EsounD can play 8bit unsigned and 16bit signed native */
    switch (format) {
    case 8:
	esd_fmt |= ESD_BITS8;
	break;
    default:
	esd_fmt |= ESD_BITS16;
	bytes_per_sample *= 2;
	break;
    }

    /* modify audio_delay depending on esd_latency
     * latency is number of samples @ 44.1khz stereo 16 bit
     * adjust according to rate_hz & bytes_per_sample
     */
#ifdef HAVE_ESD_LATENCY
    esd_latency = esd_get_latency(esd_fd);
#else
    esd_latency = ((channels == 1 ? 2 : 1) * ESD_DEFAULT_RATE * 
		   (ESD_BUF_SIZE + 64 * (4.0f / bytes_per_sample))
		   ) / rate_hz;  
    esd_latency += ESD_BUF_SIZE * 2; 
#endif
    
    esd_play_fd = esd_play_stream_fallback(esd_fmt, rate_hz,
					   server, ESD_CLIENT_NAME);
    if (esd_play_fd < 0) {
	snprintf(PYESD_ERROR_MSG, 253, 
	       "AO: [esd] failed to open esd playback stream: %d\n",
	       errno);
	return 0;
    }


    /* enable non-blocking i/o on the socket connection to the esd server */
    if ((fl = fcntl(esd_play_fd, F_GETFL)) >= 0)
	fcntl(esd_play_fd, F_SETFL, O_NDELAY|fl);

    ao_bps = bytes_per_sample * rate_hz;
    esd_play_start.tv_sec = 0;
    esd_samples_written = 0;
    esd_bytes_per_sample = bytes_per_sample;

    return 1;
}


/*
 * close audio device
 */
void esd_uninitPlayback()  /* close playback */
{
    if (esd_play_fd >= 0) {
	esd_close(esd_play_fd);
	esd_play_fd = -1;
    }
    if (esd_fd >= 0) {
	esd_close(esd_fd);
	esd_fd = -1;
    }
}


void* esdmalloc(size_t s)
{
  void* p = malloc(s);
  if (!p)
  {
    snprintf(PYESD_ERROR_MSG, 253, "PyESD: Could not allocate memory: out of memory\n");
  }

  return p;
}



/*
 * plays 'len' bytes of 'data'
 * it should round it down to outburst*n
 * return: number of bytes played
 */
int esd_play(void  *_data, int len)
{
    int offs;
    int nwritten;
    int nsamples;
    int remainder, n;
    int saved_fl;

    if (! esd_play_fd) {return -1;}
    if (! _data) {return -1;}

    /* round down buffersize to a multiple of ESD_BUF_SIZE bytes */
    len = len / ESD_BUF_SIZE * ESD_BUF_SIZE;

    if (len <= 0)
	return 0;

#define	SINGLE_WRITE 0
#if	SINGLE_WRITE
    nwritten = write(esd_play_fd, _data, len);
#else
    for (offs = 0; offs + ESD_BUF_SIZE <= len; offs += ESD_BUF_SIZE) {
	/*
	 * note: we're writing to a non-blocking socket here.
	 * A partial write means, that the socket buffer is full.
	 */
	nwritten = write(esd_play_fd, (char*)_data + offs, ESD_BUF_SIZE);
	if (nwritten != ESD_BUF_SIZE) {
	    if (nwritten < 0 && errno != EAGAIN) {
		snprintf(PYESD_ERROR_MSG, 253, "esd play: write failed: %d\n", errno);
	    }
	    break;
	}
    }
    nwritten = offs;
#endif

    if (nwritten > 0 && nwritten % ESD_BUF_SIZE != 0) {

	/*
	 * partial write of an audio block of ESD_BUF_SIZE bytes.
	 *
	 * Send the remainder of that block as well; this avoids a busy
	 * polling loop in the esd daemon, which waits for the rest of
	 * the incomplete block using reads from a non-blocking
	 * socket. This busy polling loop wastes CPU cycles on the
	 * esd server machine, and we're trying to avoid that.
	 * (esd 0.2.28+ has the busy polling read loop, 0.2.22 inserts
	 * 0 samples which is bad as well)
	 *
	 * Let's hope the blocking write does not consume too much time.
	 *
	 * (fortunatelly, this piece of code is not used when playing
	 * sound on the local machine - on solaris at least)
	 */
	remainder = ESD_BUF_SIZE - nwritten % ESD_BUF_SIZE;
	snprintf(PYESD_ERROR_MSG, 253, "esd play: partial audio block written, remainder %d \n",
		remainder);

	/* blocking write of remaining bytes for the partial audio block */
	saved_fl = fcntl(esd_play_fd, F_GETFL);
	fcntl(esd_play_fd, F_SETFL, saved_fl & ~O_NDELAY);
	n = write(esd_play_fd, (char *)_data + nwritten, remainder);
	fcntl(esd_play_fd, F_SETFL, saved_fl);

	if (n != remainder) {
	    snprintf(PYESD_ERROR_MSG, 253, 
		   "AO PLAY: [esd] send remainder of audio block failed, %d/%d\n",
		   n, remainder);
	} else
	    nwritten += n;
    }

    is_blocking=0;

    if (nwritten > 0) {
	if (!esd_play_start.tv_sec)
	    gettimeofday(&esd_play_start, NULL);
	nsamples = nwritten / esd_bytes_per_sample;
	esd_samples_written += nsamples; 
    } else {
	snprintf(PYESD_ERROR_MSG, 253, "esd play: blocked / %lu\n", esd_samples_written);
	is_blocking=1;
    }

    if (nwritten<0) {nwritten=0;}
    return nwritten;
}


int esd_play_sound_file(char *_data,  int rate_hz, int channels, int format) {
	int written=-1;
	void* myptr;
	FILE *myfile;
	int ptrsize=4096;
	int inbytes;
	int mylen=4096;  /* len should always be at least 4096 */

	if (! _data) {snprintf(PYESD_ERROR_MSG, 253, "Cannot open a NULL file."); return written;}
	myfile=fopen(_data,"rb");
	if (! myfile) {snprintf(PYESD_ERROR_MSG, 253, "Could not open file: %s",_data); return written;}

	/* open ESD */
	if (! init_esd_play( rate_hz,  channels,  format)) { return written; }

	myptr=esdmalloc(ptrsize);
	if (! myptr) {return written;}
	written=0;
	stop_playback=0;

	/* open_volume();   */

 	while (1)   {
	mylen=4096;
	if (stop_playback) {break;}
    	inbytes = fread(myptr, 1, ptrsize, myfile);	
	if (inbytes>mylen) {mylen=inbytes;}
	written=written+esd_play(myptr,mylen); 

	if  (is_blocking) { 
		while (is_blocking) {
			if (is_blocking) { sleep(0.02);} 
			written=written+esd_play(myptr,mylen); 
										}
						  }

   	 if (inbytes != ptrsize)      {
    	    if (feof(myfile))       { break;}      
  	    snprintf(PYESD_ERROR_MSG, 253, "File read error.\n");
    	    break;
    										}
				 }

	fclose(myfile);
	esd_uninitPlayback();
	/* close_volume();  */
	free(myptr);
	return written;
}



void play_sound_event(int sound_value) {
	char sound_f[256]="";

	if (sounds_opening_too_fast()) {return;}

	if (sound_value==SOUND_EVENT_PM) { if (!enable_sound_events_pm) {return;}}
	else {
		if (!enable_sound_events) {return;}
		}

	switch(sound_value ) {
		case SOUND_EVENT_BUDDY_ON:
			snprintf(sound_f,254, "%s/sounds/%s",PACKAGE_DATA_DIR,sound_events[SOUND_EVENT_BUDDY_ON]);
			break;
		case SOUND_EVENT_BUDDY_OFF:
			snprintf(sound_f,254, "%s/sounds/%s",PACKAGE_DATA_DIR,sound_events[SOUND_EVENT_BUDDY_OFF]);
			break;
		case SOUND_EVENT_MAIL:
			snprintf(sound_f,254, "%s/sounds/%s",PACKAGE_DATA_DIR,sound_events[SOUND_EVENT_MAIL]);
			break;
		case SOUND_EVENT_PM:
			snprintf(sound_f,254, "%s/sounds/%s",PACKAGE_DATA_DIR,sound_events[SOUND_EVENT_PM]);
			break;
		case SOUND_EVENT_BUZZ:
			snprintf(sound_f,254, "%s/sounds/%s",PACKAGE_DATA_DIR,sound_events[SOUND_EVENT_BUZZ]);
			break;
		case SOUND_EVENT_OTHER:
			snprintf(sound_f,254, "%s/sounds/%s",PACKAGE_DATA_DIR,sound_events[SOUND_EVENT_OTHER]);
			break;
		case SOUND_EVENT_REJECT:
			snprintf(sound_f,254, "%s/sounds/%s",PACKAGE_DATA_DIR,sound_events[SOUND_EVENT_REJECT]);
			break;
		default:
			return;

	}

			if ( capture_fp ) {	
				fprintf(capture_fp,"\nSOUND EVENT (ESound), File: %s\n", sound_f);
				fflush( capture_fp );
				}

	/* printf("sound: %s\n",sound_f); fflush(stdout);  */

	if (strlen(sound_f) >3) { 

		if (sound_value==SOUND_EVENT_BUZZ) 
			   {
			   esd_play_sound_file(sound_f,8000,1,16);
			   }

		else if (sound_value==SOUND_EVENT_REJECT) 
			   {
			   esd_play_sound_file(sound_f,8000,2,8);
			   }
		else {esd_play_sound_file(sound_f,11025,1,8); }
								}

}

#else  /* sound events not compiled int */
	/* a callback that does nothing if sound events aren't supported */
	void play_sound_event(int sound_value) {}


#endif




