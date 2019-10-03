%module pyesd
    %{
    #include "pyesd.h"
    %}



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


int get_esd_default_port();
int get_esd_default_rate();
char *get_esd_version();
char *get_esd_audio_device();
int get_8bit_stereo();
int get_16bit_mono();
int get_8bit_mono();
int get_16bit_stereo();
int get_default_stream();


char *get_pyesd_error_msg();

char *get_pyesd_version();
char *get_pyesd_info();

char *get_esd_host();
void set_esd_host(char *shost);
char *get_esd_client_name();
void esd_uninitRecord();
void esd_uninitPlayback();
void esd_uninit();



int esd_play_sound(char *_data, int len, int rate_hz, int channels, int format);

int esd_play_sound_file(char *_data,  int rate_hz, int channels, int format);

char *esd_record_sound(int len, int rate_hz, int channels, int format);

int get_record_buffer_size();

float get_esd_volume();
float get_esd_volume_force();

void esd_stop_recording();



