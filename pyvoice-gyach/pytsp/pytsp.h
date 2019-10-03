
/* ******************************
  * PyTrueSpeech
  * Copyright (c) 2003-2005 Erica Andrews
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

#define TRUESPEECH_FORMAT 0x22
#define TRUESPEECH_CBSIZE 32

char *get_pytsp_version();
char *get_pytsp_info();
char *get_pytsp_error_msg();
char *get_raw_storage_file();
char *get_tsp_storage_file();
void pytsp_init();
void pytsp_uninit();
char *compress_raw_to_tsp(char *filename, int rate, int bits, int channels);
char *decompress_tsp_to_raw(char *filename, int rate, int bits, int channels);
int decompress_tsp_to_esd(char *filename, int rate, int bits, int channels);
char *tsp_record_sound(int len);

char *get_codec_location();
char *get_codec_dll_location();






