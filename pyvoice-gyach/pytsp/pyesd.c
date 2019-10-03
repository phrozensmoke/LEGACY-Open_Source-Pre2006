/* ******************************
  * PyESD
  * Copyright (c) 2003-2006 Erica Andrews
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
char *esd_server_version=NULL;

	int 	    curbuf = 0;
	float last_vol_val=-1;
	int sound=-1;
	int stop_recording=0;

/*  ***************************************  */
/* END modified version of ao_esd.c from MPlayer */

char PYESD_ERROR_MSG[160]="";

char *get_pyesd_error_msg() {return PYESD_ERROR_MSG;}

char *get_pyesd_version() { return "0.1.1"; }

char *get_pyesd_info() { return "Copyright (c) 2003-2005 Erica Andrews <PhrozenSmoke ['at'] yahoo.com>."; }

int get_esd_default_port() {return ESD_DEFAULT_PORT;}

int get_esd_default_rate() {return ESD_DEFAULT_RATE;}

char *get_esd_audio_device() {return esd_audio_device;}

char *get_esd_host() {return ao_subdevice;}
void set_esd_host(char *shost) {if (shost != NULL) {ao_subdevice=shost;}}

char *get_esd_client_name() {return ESD_CLIENT_NAME;}

char *get_esd_version(){

	if (esd_server_version) {return esd_server_version;}
	else {
	char myver[12]="";
	FILE *esdp;
	int i;
	esdp=popen("/usr/bin/esd-config --version","r");
	if (!esdp) {
		esdp=popen("/usr/loca/bin/esd-config --version","r");
		if (!esdp) {
			esd_server_version= strdup("unknown");
			return esd_server_version;
						}
				}
	fread(myver,1,10 , esdp);
	pclose(esdp);
	for (i=0; i<10; i++)  {
		if (myver[i]=='\n') {myver[i]='\0';}
								  }
	esd_server_version=strdup(myver);
		}
	return esd_server_version;
}

int get_8bit_stereo() {
	esd_format_t fm = ESD_BITS8 | ESD_STEREO;
	return fm;
}

int get_8bit_mono() {
	esd_format_t fm = ESD_BITS8;
	return fm;
}

int get_16bit_stereo() {
	esd_format_t fm = ESD_BITS16 | ESD_STEREO;
	return fm;
}

int get_16bit_mono() {
	esd_format_t fm = ESD_BITS16;
	return fm;
}

int get_default_stream() {
	esd_format_t fm = ESD_STREAM | ESD_PLAY | ESD_MONO;
	return fm;
}



void close_volume() {   /* close volume monitor */
	if (sound>-1) {esd_close(sound);  sound=-1;}
}

void open_volume() {  /* open volume monitor */
	if (sound<0) {
		int RATE=44100;
		sound = esd_monitor_stream (ESD_BITS16|ESD_STEREO|ESD_STREAM|ESD_PLAY,
				RATE, ao_subdevice, "PyEsdPlayVol");
					 }	
}

float get_esd_volume() { return last_vol_val; }

/*  ***************************************  */
/* BEGIN modified version of ao_esd.c from MPlayer */


/* The following code was borrowed from the Gnome 'Vumeter' application, from main.c
     It has been modified for the PyESD library.  This basically returns an AVERAGE 
     of the volumes on the left and right channels as reported by ESound */

float get_esd_volume_force() {
	int NSAMP = 2048;
	int BUFS=8;
	short        aubuf[BUFS][NSAMP];

	int 	    lag = 2;
	int 	    locount = 0;
	int pos = 0;
    	int to_get = NSAMP*2;
    	int count;
    	int buf;
    	int i;
    	short val_l, val_r;
    	unsigned short bigl, bigr;
	int fl;


	int open_required=0;


    if (sound<0) {
    	open_volume();	
	open_required=1;
				 }


   if (sound < 0) {  /* printf("sound open fail\n"); fflush(stdout); */  return -1;}

    /* enable non-blocking i/o on the socket connection to the esd server */

	/* fcntl(sound, F_SETFL, O_NONBLOCK); */ /*  This is useless */

    while (to_get>0){
    count = read(sound, aubuf[curbuf] + pos, to_get);
    if (count <0)      
	{ if (open_required)  {close_volume();}  return last_vol_val;  }	/* no data */
    else
    {
	pos += count;
	to_get -= count;
    }
    if (to_get == 0)
    {
	to_get = NSAMP*2;
	pos = 0;
	break;
    }
						}


    curbuf++;
    curbuf++;
    if(curbuf >= BUFS)
	curbuf = 0;
    buf = ((BUFS*2)+curbuf-lag)%BUFS;
    if((curbuf%2)>0)
	{ if (open_required)  {close_volume();}   return last_vol_val;} 
    
    bigl = bigr = 0;
    for (i = 0; i < NSAMP/2;i++)
    {
	val_l = abs (aubuf[curbuf][i]);
	i++;
	val_r = abs (aubuf[curbuf][i]);
	bigl = (val_l > bigl) ? val_l : bigl;
	bigr = (val_r > bigr) ? val_r : bigr;
    }
    bigl /= (NSAMP/8);
    bigr /= (NSAMP/8);

	if (open_required)  {close_volume();}
	last_vol_val= ((bigl + bigr)/2);
	if (last_vol_val>10) {last_vol_val=10.000;}
	return last_vol_val;

}



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
	    snprintf(PYESD_ERROR_MSG, 158, 
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
	snprintf(PYESD_ERROR_MSG, 158, 
	       "AO: [esd] failed to open esd playback stream: %d\n",
	       errno);
	return 0;
    }

	/* printf("RATE:  %d\n",rate_hz);
	fflush(stdout);
	printf("CHANNELS:  %d\n",channels);
	fflush(stdout);
	printf("FORMAT:  %d\n",esd_fmt);
	fflush(stdout); */

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
 * open & setup audio device for recording
 * return: 1=success 0=fail
 * rate = 22050, 44100, etc.
 * channels is either 1 or 2
 * format is either 8 or 16
 */
int init_esd_record(int rate_hz, int channelsd, int formatd)
{
	int rate = ESD_DEFAULT_RATE;
    	int bits = ESD_BITS16, channels = ESD_STEREO;
    	int mode = ESD_STREAM, func = ESD_RECORD ;
    	esd_format_t format = 0;

	rate=rate_hz;
	if (channelsd==1)  {channels = ESD_MONO; }
	if (formatd==8) {bits = ESD_BITS8;}
	bits=formatd;
   	format = bits | channels | mode | func;

	if (esd_play_fdR>0)  {return 1;}

    /* enable non-blocking i/o on the socket connection to the esd server */

	/* fcntl(esd_play_fdR, F_SETFL, O_NONBLOCK); */


	esd_play_fdR=esd_record_stream_fallback(format,rate,NULL, ESD_CLIENT_NAME );

    if (esd_play_fdR < 0) {
	snprintf(PYESD_ERROR_MSG, 158, 
	       "AO: [esd] failed to open esd record stream: %d\n",
	       errno);
	return 0;
    							}

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
	if (sound>=0) {esd_close(sound); sound=-1;}
}

void esd_uninitRecord()  /* close record */
{
    if (esd_play_fdR >= 0) {
	esd_close(esd_play_fdR);
	esd_play_fdR = -1;
    }

}

void esd_uninit()   /* free any other resources */
{
   if (esd_server_version) { free(esd_server_version); esd_server_version=NULL;}
   if (sound>=0) {esd_close(sound); sound=-1;}
}


/*
 * plays 'len' bytes of 'data'
 * it should round it down to outburst*n
 * return: number of bytes played
 */
int play(void  *_data, int len)
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
		snprintf(PYESD_ERROR_MSG, 158, "esd play: write failed: %d\n", errno);
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
	snprintf(PYESD_ERROR_MSG, 158, "esd play: partial audio block written, remainder %d \n",
		remainder);

	/* blocking write of remaining bytes for the partial audio block */
	saved_fl = fcntl(esd_play_fd, F_GETFL);
	fcntl(esd_play_fd, F_SETFL, saved_fl & ~O_NDELAY);
	n = write(esd_play_fd, (char *)_data + nwritten, remainder);
	fcntl(esd_play_fd, F_SETFL, saved_fl);

	if (n != remainder) {
	    snprintf(PYESD_ERROR_MSG, 158, 
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
	snprintf(PYESD_ERROR_MSG, 158, "esd play: blocked / %lu\n", esd_samples_written);
	is_blocking=1;
    }

    if (nwritten<0) {nwritten=0;}
    return nwritten;
}






/* END modified version of ao_esd.c from MPlayer */

int record(char *_data, int len)
{
    int nread;
    if (! esd_play_fdR) {return 0;}
    if (! _data) {return 0;}
   
    nread=read( esd_play_fdR, _data, len );
    return nread;
}


int esd_play_sound(void*_data, int len, int rate_hz, int channels, int format) {
	int written=-1;
	int mylen=4096;  /* len should always be at least 4096 */
	if (len>mylen) {mylen=len;}
	if (init_esd_play( rate_hz,  channels,  format)) {
		written=play(_data,mylen);
		/* esd_uninitPlayback();  */
																		   }
	return written;
}


void* esdmalloc(size_t s)
{
  void* p = malloc(s);
  if (!p)
  {
    snprintf(PYESD_ERROR_MSG,158,  "PyESD: Could not allocate memory: out of memory\n");
  }

  return p;
}



int get_record_buffer_size() {return ESD_BUF_SIZE;}

void esd_stop_recording() {stop_recording=1;}

char *esd_record_sound(int len, int rate_hz, int channelsd, int formatd) {
	int nread=-1;
	char rbuf[ESD_BUF_SIZE];   /* ESD_BUF_SIZE */
	char *rbuf2=rbuf;
	char buffer[len+1];
	char *buffer_d=buffer;
	int cursor=1;
	int copyi;
	int space_left; 

	/* FILE *target; */
	space_left=len-cursor;

	if (init_esd_record( rate_hz,  channelsd,  formatd)) {

		/* target = fopen( "aggy.raw", "w" );  */
		sprintf(buffer,"%s","");
		stop_recording=0;

		while (space_left>0) {
			nread=record(rbuf,ESD_BUF_SIZE);
			if (! nread) {break;}
			copyi=nread;			
			if (copyi>space_left) {copyi=space_left;}

			/* fwrite( buf2, 1, nread, target );  */
			strncat(buffer,rbuf,copyi);
			cursor=cursor+copyi;
			space_left=len-cursor;  
			sleep(0.007);
			if (stop_recording==1) {break;}
									      }
		/* fclose(target);  */

												
									   												} else {return NULL; }

	return buffer_d;
}



int esd_play_sound_file(char *_data,  int rate_hz, int channels, int format) {
	int written=-1;
	void* myptr;
	FILE *myfile;
	int ptrsize=4096;
	int inbytes;
	int mylen=4096;  /* len should always be at least 4096 */
	int skipit=0;

	if (! _data) {snprintf(PYESD_ERROR_MSG, 158, "Cannot open a NULL file."); return written;}

	/* open ESD */
	if (! init_esd_play( rate_hz,  channels,  format)) { return written; }

	myfile=fopen(_data,"rb");
	if (! myfile) {snprintf(PYESD_ERROR_MSG, 158, "Could not open file: %s",_data); return written;}

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
	written=written+play(myptr,mylen); 

	/* we really only want to do volume updates when we KNOW sound is playing or 
	     the sound volume connection to ESD will hang until sound is played */
	  if (written>=8192) { 
		if (! is_blocking) {
			if (!skipit) {get_esd_volume_force(); skipit=1;}
			else {skipit=0;}
									} 
								   }  

	if  (is_blocking) { 
		while (is_blocking) {
			if (is_blocking) { sleep(0.02);} 
			written=written+play(myptr,mylen); 
										}
						  }

   	 if (inbytes != ptrsize)      {
    	    if (feof(myfile))       { break;}      
  	    snprintf(PYESD_ERROR_MSG,158,  "File read error.\n");
    	    break;
    										}
				 }

	fclose(myfile);
	/* close_volume();  */
	free(myptr);
	return written;
}




