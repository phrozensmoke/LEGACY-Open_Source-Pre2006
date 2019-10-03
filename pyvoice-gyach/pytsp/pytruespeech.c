/* ******************************
  * PyTrueSpeech
  * Copyright (c) 2003-2006 Erica Andrews
  * <PhrozenSmoke ['at'] yahoo.com>
  *
  * License: GNU General Public License
  *
  * A Python-wrapped library for 
  * converting between raw sound
  * and DSP's Group's TrueSpeech format
  * and playing TrueSpeech sound over
  * ESound.  This library relies on PyESD
  * for playback over ESD.  This library
  * uses highly modified versions of 
  * code provided by Piotr Karwasz and 
  * Robert Brewer for loading
  * the TrueSpeech codecs with MPlayer's
  * version of 'wine lib'.
  *
****************************** */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include "loader.h"
#include "driver.h"
#include "wine/windef.h"
#include "wineacm.h"
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <esd.h>

#include "pyesd.h"
#include "pytsp.h"


int verbose=5;
int do_debug=0;
char PYTSP_ERROR_MSG[176]="";

  WAVEFORMATEX *ts_wf = 0;
  WAVEFORMATEX pcm_wf;
  WAVEFORMATEX *i_wf = 0;
  WAVEFORMATEX *o_wf = 0;
  HACMSTREAM handle;
  MMRESULT ret;
  DWORD srcsize = 0;
  DWORD destsize = 0;
  long* iptr = 0;
  PWINE_ACMDRIVERID acmDriver1 = 0;
  PWINE_ACMDRIVERID acmDriver2 = 0;
  ACMSTREAMHEADER header;
  FILE* i_file;
  FILE* o_file;

  int driver_loaded=0;
  int recording_driver=0;

  char *tsp_storage_file=NULL;
  char *tsp_storage_file2=NULL;
  char *tsp_storage_file3=NULL;
  char *raw_storage_file=NULL;

char *get_pytsp_error_msg() {return PYTSP_ERROR_MSG;}
char *get_pytsp_version() {return "0.1.1";}
char *get_pytsp_info() { return "Copyright (c) 2003-2005 Erica Andrews <PhrozenSmoke ['at'] yahoo.com>.";}

/* NOTE, don't remove, this is needed, declared extern in registry.c */
/* char* get_path(char* x){  return strdup(x);}   */


char *get_codec_location(){
	char clocat[255]="";
	char *cloc=clocat;
	snprintf(clocat,240, "%s/",WIN32_PATH);
	strcat(clocat,"tssoft32.acm");
	return cloc;
									}

char *get_codec_dll_location(){
	char clocat[255]="";
	char *cloc=clocat;
	snprintf(clocat,240, "%s/",WIN32_PATH);
	strcat(clocat,"tsd32.dll");
	return cloc;
									}


void* tspalloc(size_t s)
{
  void* p = malloc(s);
  if (!p)  {snprintf(PYTSP_ERROR_MSG,174, "PyTSP: tspalloc: out of memory, not enough memory."); }
  return p;
}


int uninit_tsp_driver() {
   if (driver_loaded==1) {
  MSACM_UnregisterDriver(acmDriver1);
  if (do_debug) {printf("acm driver unregistered for truespeech\n"); }
  MSACM_UnregisterDriver(acmDriver2);
  if (do_debug) {printf("acm driver unregistered for pcm\n");  }
  MSACM_UnregisterAllDrivers();  /* just to be safe */
	driver_loaded=0;
							    }
return 1;
}



int init_tsp_driver() {  
  
   if (driver_loaded) {return 1;}

  acmDriver1 = MSACM_RegisterDriver("tssoft32.acm", TRUESPEECH_FORMAT, 0);
  if (!acmDriver1)
  {
    snprintf(PYTSP_ERROR_MSG,174,"Error registering TrueSpeech ACM driver for TrueSpeech: tssoft32.acm");
    return 0;
  }
  
  if (do_debug) {printf("register driver complete for truespeech\n"); }



  acmDriver2 = MSACM_RegisterDriver("tssoft32.acm", WAVE_FORMAT_PCM, 0);
  if (!acmDriver2)
  {
    snprintf(PYTSP_ERROR_MSG,174,"Error registering TrueSpeech ACM driver for PCM:  tssoft32.acm");
    return 0;
  }


   if (do_debug) {printf("register driver complete for tsp\n"); }
driver_loaded=1;
return 1;
}



int convertfile(char* infile, char* outfile, int rate, int bits, int channels, int infilepcm)
{

	int inbytes = 0;


	if  (init_tsp_driver()<1) {return 0;}

  // TrueSpeech format
  if (ts_wf) {
	free(ts_wf);
		   }
  	ts_wf = tspalloc(sizeof(WAVEFORMATEX) + TRUESPEECH_CBSIZE);
	if (! ts_wf) {	return -1;   }
  	memset(ts_wf, 0, sizeof(*ts_wf) + TRUESPEECH_CBSIZE);
  
  ts_wf->wFormatTag      = TRUESPEECH_FORMAT;
  ts_wf->nChannels       = 1;   /* 1 */
  ts_wf->nSamplesPerSec  = 8000;  /* 8000 */
  ts_wf->wBitsPerSample  = 1;  
  ts_wf->nBlockAlign     = 32;
  ts_wf->nAvgBytesPerSec = 1067;
  ts_wf->cbSize          = TRUESPEECH_CBSIZE;

  // write extra data needed by TrueSpeech codec found
  // from examining a TrueSpeech .wav file header
  iptr = (long*)(ts_wf + 1);
  *iptr = 0x00f00001;

//  print_wave_header(in_fmt);

  // Typical PCM format, 16-bit signed samples at 8 KHz.
  memset(&pcm_wf, 0, sizeof(pcm_wf));
  
  pcm_wf.wFormatTag      = WAVE_FORMAT_PCM;
  pcm_wf.nChannels       = 1;   /* 1 */
  pcm_wf.nSamplesPerSec  = 8000;   /* 8000 */
  pcm_wf.wBitsPerSample  = 16;   /* 16 */
  if ( recording_driver==1) {pcm_wf.wBitsPerSample  = 8;}


  pcm_wf.nBlockAlign     = pcm_wf.nChannels * pcm_wf.wBitsPerSample / 8;
  pcm_wf.nAvgBytesPerSec = pcm_wf.nSamplesPerSec * pcm_wf.nBlockAlign;
    

  // decide which way to perform the conversion
  if (infilepcm)
  {
    i_wf = &pcm_wf;
    o_wf = ts_wf;
  }
  else
  {
    i_wf = ts_wf;
    o_wf = &pcm_wf;
  }
  
  // from here on out, everything is pretty generic as long as we
  // use i_wf and o_wf

  if (! init_tsp_driver()) {return -1;}

  // TrueSpeech isn't confident it can do realtime
  // compression.  Tell it we don't care and it's happy.
  ret = acmStreamOpen(&handle, 0, i_wf, o_wf, 0, 0, 0,
                      ACM_STREAMOPENF_NONREALTIME);
  if (ret)
  {
    if (ret == ACMERR_NOTPOSSIBLE)
    {
      snprintf(PYTSP_ERROR_MSG,174,"Invalid codec: mismatched rate, bits, or channels.");
      uninit_tsp_driver(); /*seg-fault watch */
    }
    else
    {
      snprintf(PYTSP_ERROR_MSG,174,"acmStreamOpen error %d", ret);
      uninit_tsp_driver(); /*seg-fault watch */
    }

    return 0;
  }
  
  if (do_debug) {printf("audio codec opened\n"); }

  // we assume that the format with the largest block alignment is
  // the most picky.  And it basically turns out that TrueSpeech has
  // the largest block alignment.  Anyway, we use that block alignment
  // to ask for the preferred size of the "other" format's buffer.
  // Then we turn around and ask for the first one's preferred buffer
  // size based on the other's buffer size, which seems to turn out
  // to be its block alignment.  Or something like that.
  if (i_wf->nBlockAlign > o_wf->nBlockAlign)
  {
    ret = acmStreamSize(handle, i_wf->nBlockAlign, &destsize, ACM_STREAMSIZEF_SOURCE);
    if (ret)
    {
      snprintf(PYTSP_ERROR_MSG,174,"acmStreamSize error %d", ret);
      uninit_tsp_driver(); /*seg-fault watch */
      return 0;
    }
  
    if (do_debug) {printf("Audio ACM output buffer min. size: %ld\n", destsize); }
    if (!destsize)
    {
      snprintf(PYTSP_ERROR_MSG,174,"ACM codec reports destsize=0");
      uninit_tsp_driver(); /*seg-fault watch */
      return 0;
    }

    ret = acmStreamSize(handle, destsize, &srcsize, ACM_STREAMSIZEF_DESTINATION);
    if (ret)
    {
      snprintf(PYTSP_ERROR_MSG,174,"acmStreamSize error");
      uninit_tsp_driver(); /*seg-fault watch */
      return 0;
    }

    if (do_debug) {printf("Audio ACM input buffer min. size: %ld\n", srcsize); }
  }
  else
  {
    ret = acmStreamSize(handle, o_wf->nBlockAlign, &srcsize, ACM_STREAMSIZEF_DESTINATION);
    if (ret)
    {
      snprintf(PYTSP_ERROR_MSG,174,"acmStreamSize error %d", ret);
      uninit_tsp_driver(); /*seg-fault watch */
      return 0;
    }
  
    if (do_debug) {printf("Audio ACM input buffer min. size: %ld\n", srcsize); }
    if (!srcsize)
    {
      snprintf(PYTSP_ERROR_MSG,174,"ACM codec reports srcsize=0");
      uninit_tsp_driver(); /*seg-fault watch */
      return 0;
    }

    ret = acmStreamSize(handle, srcsize, &destsize, ACM_STREAMSIZEF_SOURCE);
    if (ret)
    {
      snprintf(PYTSP_ERROR_MSG,174,"acmStreamSize error");
      uninit_tsp_driver(); /*seg-fault watch */
      return 0;
    }

    if (do_debug) {printf("Audio ACM input buffer min. size: %ld\n", destsize); }
  }


  // set up conversion buffers

  memset(&header, 0, sizeof(header));
  header.cbStruct    = sizeof(header);
  header.cbSrcLength = srcsize;
  if (header.pbSrc) {  	free(header.pbSrc);  }
  header.pbSrc       = tspalloc(header.cbSrcLength);
  if (! header.pbSrc) { return -1; }
  header.cbDstLength = destsize;
  if (header.pbDst) {	free(header.pbDst);	  }
  header.pbDst       = tspalloc(header.cbDstLength);
  if (! header.pbDst) { return -1; }
  
  ret = acmStreamPrepareHeader(handle, &header, 0);
    
  if (ret)
  {
    snprintf(PYTSP_ERROR_MSG,174,"error preparing header");
    uninit_tsp_driver(); /*seg-fault watch */
    return 0;
  }

  if (do_debug) {printf("header prepared %lx\n", header.fdwStatus); }
  
  
  // open files
  i_file = fopen(infile, "rb");
  if (!i_file)
  {
    snprintf(PYTSP_ERROR_MSG,174,"error opening input file %s", infile);
    uninit_tsp_driver(); /*seg-fault watch */
    return 0;
  }

  if (do_debug) {printf("input file opened\n"); }
  
  
  o_file = fopen(outfile, "wb");
  if (!o_file)
  {
    snprintf(PYTSP_ERROR_MSG,174,"error opening output file %s", outfile);
    uninit_tsp_driver(); /*seg-fault watch */
    if (i_file) {fclose(i_file);}
    return 0;
  }


  if (do_debug) {printf("output file opened\n"); }

  
  while (1)
  {
    inbytes = fread(header.pbSrc, 1, srcsize, i_file);

    if (inbytes != srcsize)
    {
      if (feof(i_file))
      {
        if (do_debug) {printf("end of input file, last byte count %d\n", inbytes); }
        break;
      }
      
      snprintf(PYTSP_ERROR_MSG,174,"read error");
      uninit_tsp_driver(); /*seg-fault watch */
      if (i_file) {fclose(i_file);}  if (o_file) {fclose(o_file);}
      return 0;
    }

    ret = acmStreamConvert(handle, &header, ACM_STREAMCONVERTF_BLOCKALIGN);
    if (ret)
    {
      snprintf(PYTSP_ERROR_MSG,174,"conversion error");
      uninit_tsp_driver(); /*seg-fault watch */
      if (i_file) {fclose(i_file);}  if (o_file) {fclose(o_file);}
      return 0;
    }

    if (!(header.fdwStatus & ACMSTREAMHEADER_STATUSF_DONE))
    {
      snprintf(PYTSP_ERROR_MSG,174,"header not marked done %lx", header.fdwStatus);
      uninit_tsp_driver(); /*seg-fault watch */
      if (i_file) {fclose(i_file);}  if (o_file) {fclose(o_file);}
      return 0;
    }

    if (header.cbSrcLengthUsed != header.cbSrcLength)
    {
      snprintf(PYTSP_ERROR_MSG,174,"didn't use all of source data");
      uninit_tsp_driver(); /*seg-fault watch */
      if (i_file) {fclose(i_file);}  if (o_file) {fclose(o_file);}
      return 0;
    }

     if (do_debug) { printf("converted %d bytes to %d bytes\n",
           header.cbSrcLengthUsed,
           header.cbDstLengthUsed); }

    if (fwrite(header.pbDst, 1, header.cbDstLengthUsed, o_file) !=
        header.cbDstLengthUsed)
    {
      snprintf(PYTSP_ERROR_MSG,174,"error writing file");
      uninit_tsp_driver(); /*seg-fault watch */
      if (i_file) {fclose(i_file);}  if (o_file) {fclose(o_file);}
      return 0;
    }      
  }

  // now convert the remaining bit of the file
  // and ask for any leftover stuff

  header.cbSrcLength = inbytes;
  while (1)
  {
    if (header.cbSrcLength)
    {
      // not a full block, but not quite the "end" either
      ret = acmStreamConvert(handle, &header, 0);
    }
    else
    {
      // now we're at the end, let's see if anything is left
      ret = acmStreamConvert(handle, &header, ACM_STREAMCONVERTF_END);
    }
    
    if (ret)
    {
      snprintf(PYTSP_ERROR_MSG,174,"conversion error");
      uninit_tsp_driver(); /*seg-fault watch */
      if (i_file) {fclose(i_file);}  if (o_file) {fclose(o_file);}
      return 0;
    }
    
    if (!(header.fdwStatus & ACMSTREAMHEADER_STATUSF_DONE))
    {
      snprintf(PYTSP_ERROR_MSG,174,"header not marked done %lx", header.fdwStatus);    		 uninit_tsp_driver(); /*seg-fault watch */
      if (i_file) {fclose(i_file);}  if (o_file) {fclose(o_file);}
      return 0;
    }
    
    if (header.cbSrcLengthUsed != header.cbSrcLength)
    {
      snprintf(PYTSP_ERROR_MSG,174,"didn't use all of source data");
       uninit_tsp_driver(); /*seg-fault watch */
      if (i_file) {fclose(i_file);}  if (o_file) {fclose(o_file);}
      return 0;
    }
    
   if (do_debug) { printf("converted %d bytes to %d bytes\n",
           header.cbSrcLengthUsed,
           header.cbDstLengthUsed); }

    if (!header.cbDstLengthUsed)
    {
      if (do_debug) {printf("nothing given to output, must be done\n"); }
      break;
    }
    
    if (fwrite(header.pbDst, 1, header.cbDstLengthUsed, o_file) !=
        header.cbDstLengthUsed)
    {
      snprintf(PYTSP_ERROR_MSG,174,"error writing file");
      uninit_tsp_driver(); /*seg-fault watch */
      if (i_file) {fclose(i_file);}  if (o_file) {fclose(o_file);}
      return 0;
    }

    // ensure we are getting the remaining stuff on the next time around
    header.cbSrcLength = 0;
  }
  
    
  if (do_debug) {printf("finished converting file\n");}
  

  // now close up shop

  fclose(i_file);
  fclose(o_file);

  ret = acmStreamUnprepareHeader(handle, &header, 0);
  if (ret)
  {
    snprintf(PYTSP_ERROR_MSG,174,"error unpreparing header");
    uninit_tsp_driver(); /*seg-fault watch */
    return 1;
  }  
    
  ret = acmStreamClose(handle, 0);
  if (ret)
  {
    snprintf(PYTSP_ERROR_MSG,174,"error closing acm stream");
    uninit_tsp_driver(); /*seg-fault watch */
    return 1;
  }

  if (do_debug) {printf("acm stream closed\n");}
  
  uninit_tsp_driver();

 if (ts_wf) {free(ts_wf);  ts_wf=NULL;}
	if (header.pbSrc)  { free(header.pbSrc); header.pbSrc=NULL;}
	if (header.pbDst)  {  free(header.pbDst); header.pbDst=NULL;}

  return 1;
}




void tsp_uninit(){
 if (ts_wf) {free(ts_wf);  ts_wf=NULL;}
	if (header.pbSrc)  { free(header.pbSrc); header.pbSrc=NULL;}
	if (header.pbDst)  {  free(header.pbDst); header.pbDst=NULL;}

}


void pytsp_uninit() {
	uninit_tsp_driver();
	tsp_uninit();
	if (tsp_storage_file) {free(tsp_storage_file); tsp_storage_file=NULL;}
	if (tsp_storage_file2) {free(tsp_storage_file2); tsp_storage_file2=NULL;}
	if (tsp_storage_file3) {free(tsp_storage_file3); tsp_storage_file3=NULL;}
	if (raw_storage_file) {free(raw_storage_file); raw_storage_file=NULL;}
 	esd_uninitRecord();
 	esd_uninitPlayback();
	esd_uninit();
}


void pytsp_init() {
	init_tsp_driver();
}



char *get_tsp_storage_file(){
	if (tsp_storage_file) {return tsp_storage_file;}
	else {
	char *user="user";
	char storage[70];
	char *stor=storage;
	if (getenv("USER")) {user=getenv("USER");}
	strcpy(storage,"/tmp/pytsp_tsp_storage_");
	strncat(storage,user,30);
	tsp_storage_file=strdup(stor);
		}
	return tsp_storage_file;
}

char *get_raw_storage_file(){
	if (raw_storage_file) {return raw_storage_file;}
	else {
	char *user="user";
	char storage[70]="";
	char *stor=storage;
	if (getenv("USER")) {user=getenv("USER");}
	strcpy(storage,"/tmp/pytsp_raw_storage_");
	strncat(storage,user,30);
	raw_storage_file=strdup(stor);
		}
	return raw_storage_file;
}

char *get_tsp_record_file(){
	if (tsp_storage_file2) {return tsp_storage_file2;}
	else{
	char *user="user";
	char storage[70];
	char *stor=storage;
	if (getenv("USER")) {user=getenv("USER");}
	strcpy(storage,"/tmp/pytsp_tsp_record_");
	strncat(storage,user,30);
	tsp_storage_file2=strdup(stor);
	    }
	return tsp_storage_file2;
}

char *get_raw_record_file(){
	if (tsp_storage_file3) {return tsp_storage_file3;}
	else {
	char *user="user";
	char storage[70];
	char *stor=storage;
	if (getenv("USER")) {user=getenv("USER");}
	strcpy(storage,"/tmp/pytsp_raw_record_");
	strncat(storage,user,30);
	tsp_storage_file3=strdup(stor);
		}
	return tsp_storage_file3;
}


char *decompress_tsp_to_raw(char *filename, int rate, int bits, int channels) {
	char *to_raw=NULL;
	if (! filename) {return NULL;}
	to_raw=get_raw_storage_file();
	if (! to_raw) {return NULL; }

	 recording_driver=0;

	if (convertfile(filename, to_raw , rate, bits, channels, 0)) 
		{
		return to_raw;
		} else {return NULL;}
	return to_raw;
}

char *compress_raw_to_tsp(char *filename, int rate, int bits, int channels) {
	char *to_tsp=NULL;
	if (! filename) {return NULL;}
	to_tsp=get_tsp_storage_file();
	if (! to_tsp) {return NULL; }

	 recording_driver=0;

	if (convertfile(filename, to_tsp , rate, bits, channels, 1)) 
		{
		return to_tsp;
		} else {return NULL;}
	return to_tsp;
}


int decompress_tsp_to_esd(char *filename, int rate, int bits, int channels) {
	char *filen;
	filen=decompress_tsp_to_raw(filename, rate, bits, channels);
	if (!filen) {return 0;}
	return esd_play_sound_file(filen,  rate,  channels, bits);
}



char *tsp_record_sound(int len) {
	char *recorded;
	char *infile;
	char *outfile;
	FILE *ofile;
	int owritten;
	int ilen;

	ilen=len;
	if (ilen<4096) {ilen=4096;}
	infile=get_raw_record_file();
	outfile=get_tsp_record_file();
	recorded=esd_record_sound( ilen, 8000, 1, 16);

	 recording_driver=1;

	if (recorded != NULL) {
		if (strlen(recorded)>=4096) {
			ofile=fopen(infile, "wb");
			if (ofile) {

				owritten=fwrite(recorded, 1, strlen(recorded), ofile);
				fflush(ofile);
				fclose(ofile);

				if (owritten==strlen(recorded)) {

					if (convertfile(infile, outfile, 8000, 16,1, 1)) { return outfile;}
													 			    }

						  }
		
											  		  }

								  }
	return "";
}


