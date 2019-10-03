/*****************************************************************************
 * gyache-mcrypt.c, Plugin to use MCrypt/libMCrypt for sending
 * encrypted instant messages using various encryption algorithms
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
 * Copyright (C) 2003-2006, Erica Andrews (Phrozensmoke ['a'] yahoo.com)
 * Released under the terms of the GPL.
 * *NO WARRANTY*
 *
 *****************************************************************************/


#include "../plugin_api.h"

#include <mcrypt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char *gmcrypt_block_buffer=NULL;
char *gmcrypt_block_bufferO=NULL;
char gmcrypt_screen_name[14]="";

char *gyache_mcrypt_key_string="SzpEnTGhbwBmQkHjDyKnXRk7bFVcpPzTWIhHgDyKnXRk7bFVcpPzTWIhHghbwBmQ";

  /* xtea,  blowfish, twofish, tripledes
  */
char *gmcrypt_crypt_algo="blowfish";

  int keysize=16; /* 128 bits */
  int addition=0;
  int encoded_limit=0;
  int can_use_special_chars=0;
  int very_light=1; /* some algorithms don't like non-alphanumeric passwords */
  char *algo_method="cfb";


void setup_mcrypt_encryption(int encrypt_type ) {
  keysize=16; 
  addition=0;
  encoded_limit=0;
  can_use_special_chars=0;
  very_light=1; 
  algo_method="cfb";

	switch (encrypt_type) {
		case ENC_TYPE_MCRYPT_XTEA:
			gmcrypt_crypt_algo="xtea";
			keysize=8;
			can_use_special_chars=0;
			very_light=1;
			addition=4;
			break;
		case ENC_TYPE_MCRYPT_BF:
			gmcrypt_crypt_algo="blowfish";
			keysize=48;
			addition=4;
			break;
		case ENC_TYPE_MCRYPT_TF:
			gmcrypt_crypt_algo="twofish";
			keysize=24;
			addition=4;
			break;
		case ENC_TYPE_MCRYPT_TD:
			gmcrypt_crypt_algo="tripledes";
			keysize=16;
			addition=8;
			can_use_special_chars=0;
			break;
		case ENC_TYPE_MCRYPT_LOKI97:
			gmcrypt_crypt_algo="loki97";
			can_use_special_chars=0;
			very_light=1;
			break;
		case ENC_TYPE_MCRYPT_RC6:
			gmcrypt_crypt_algo="rc6";
			can_use_special_chars=0;
			very_light=1;
			keysize=16;
			break;
		case ENC_TYPE_MCRYPT_CAST128:
			gmcrypt_crypt_algo="cast-128";
			very_light=1;
			keysize=8;
			can_use_special_chars=0;
			break;
		case ENC_TYPE_MCRYPT_CAST256:
			gmcrypt_crypt_algo="cast-256";
			very_light=1;
			can_use_special_chars=0;
			break;
		case ENC_TYPE_MCRYPT_GOST:
			gmcrypt_crypt_algo="gost";
			can_use_special_chars=0;
			very_light=1;
			break;
		case ENC_TYPE_MCRYPT_ARCFOUR:
			gmcrypt_crypt_algo="arcfour";
			keysize=56;
			addition=4;
			algo_method="stream";
			can_use_special_chars=0;
			very_light=1;
			break;
		case ENC_TYPE_MCRYPT_RIJNDAEL:
			gmcrypt_crypt_algo="rijndael-256";
			keysize=16;
			addition=4;
			can_use_special_chars=0;
			very_light=1;
			break;
		case ENC_TYPE_MCRYPT_SERPENT:
			gmcrypt_crypt_algo="serpent";
			keysize=16;
			can_use_special_chars=0;
			very_light=1;
			addition=4;
			break;
		case ENC_TYPE_MCRYPT_SAFERPLUS:
			gmcrypt_crypt_algo="saferplus";
			keysize=16;
			can_use_special_chars=0;
			very_light=1;
			addition=4;
			break;

		default:
			gmcrypt_crypt_algo="blowfish";
			break;
	}

}

char *decrypt_message (char *inuser, char *inmsg, int encrypt_type ) {
  MCRYPT td;
  int i;
  char *key;
  char *IV;
  char mcrpassword[65];

  setup_mcrypt_encryption(encrypt_type);

	if (!gmcrypt_block_bufferO) {gmcrypt_block_bufferO=malloc(4097);}
	if (!gmcrypt_block_bufferO) {return inmsg;}

    key=malloc(keysize+1+addition);
	if (!key)  {return inmsg;}

  /* The password can be up to 20 bytes */
  sprintf(mcrpassword, "%c%c%c%c%c%c%c%c", 
	gyache_mcrypt_key_string[4], gyache_mcrypt_key_string[21], 
	gyache_mcrypt_key_string[7], gyache_mcrypt_key_string[17],
	gyache_mcrypt_key_string[13], gyache_mcrypt_key_string[1], 
	gyache_mcrypt_key_string[8], gyache_mcrypt_key_string[15]
	);

	sprintf(gmcrypt_screen_name,"%s",""); /* clear old name */
	if (very_light) {
		strncpy(gmcrypt_screen_name, "nTGhbwBmQkHjDy", 12);
					 } else {strncpy(gmcrypt_screen_name, inuser, 12);}
	lower_str(gmcrypt_screen_name);  /* have to keep things consistent */
	strncat(mcrpassword,gmcrypt_screen_name,12);

	if (can_use_special_chars) {
	mcrpassword[0]=10;
	mcrpassword[2]=17;
	mcrpassword[strlen(mcrpassword)-3]=(gmcrypt_screen_name[0]+7)<121?(gmcrypt_screen_name[0]+7):121;
	mcrpassword[strlen(mcrpassword)-1]=4; 
	} else  {
	mcrpassword[6]=gyache_mcrypt_key_string[20];
	mcrpassword[strlen(mcrpassword)-3]=gyache_mcrypt_key_string[11];
	mcrpassword[strlen(mcrpassword)-1]=gyache_mcrypt_key_string[5];
	}

	/* printf("de-pass:  %s\n", mcrpassword); fflush(stdout); */

   if (strlen(mcrpassword)<64) {
	strncat(mcrpassword, gyache_mcrypt_key_string, 64-strlen(mcrpassword) );
   }

/* Generate the key using the password */
  memset(key, 0, sizeof(key));
  memcpy(key, mcrpassword, keysize+addition);
  key[keysize+addition]='\0';
  for (i=0; i<strlen(key); i++) { 	if (key[i]=='_') {key[i]='x';}   }
  // snprintf(key, keysize, "%s", mcrpassword);

  td = mcrypt_module_open(gmcrypt_crypt_algo, NULL, algo_method, NULL);

  if (td==MCRYPT_FAILED) {
     free(key);
     return inmsg;
  }

  IV = malloc(mcrypt_enc_get_iv_size(td));
	if (! IV  ) {free(key); return inmsg;}

/* Put random data in IV. Note these are not real random data,
 * consider using /dev/random or /dev/urandom.
 */

	/* 	Normally we would want these to be random, 
		but we have to keep them consistent across
		gyach-e instances, otherwise decrypting will
		fail on the other end, or we would have to come
		up with a way to construct and send this 'key' 
			mcrypt_enc_get_iv_size( td) always equals
			either 8 or 16 for our cases here
	*/

	if ( (encrypt_type==ENC_TYPE_MCRYPT_BF) || 
	(encrypt_type==ENC_TYPE_MCRYPT_TF) ) {
		can_use_special_chars=0;
	}
 
  for (i=0; i< mcrypt_enc_get_iv_size( td); i++) {
   	  /* IV[i]=rand(); */
   	if (can_use_special_chars) { IV[i]=i+11; }
	else {IV[i]=gyache_mcrypt_key_string[i]; }
  }

	if (can_use_special_chars) { IV[0]=key[strlen(key)-2]; }
		else { IV[0]=gyache_mcrypt_key_string[22];} 

	/* printf("de-IV:  %s\n", IV); fflush(stdout); */

  i=mcrypt_generic_init( td, key, keysize+addition, IV);
  if (i<0) {
     /* mcrypt_perror(i); */
     free(key);
     free(IV);
     return inmsg;
  }

  /* Encryption in CFB is performed in bytes */
  i=0;
  encoded_limit=snprintf(gmcrypt_block_bufferO, 2047, "%s", inmsg);
  gmcrypt_block_bufferO[encoded_limit]='\0';
	while (i< encoded_limit) {
		// if (gmcrypt_block_bufferO[i]=='\0') {break;}
		mdecrypt_generic (td, &gmcrypt_block_bufferO[i], 1);
		i++;
					 }
	gmcrypt_block_bufferO[i]='\0';

/* Deinit the encryption thread, and unload the module */
  mcrypt_generic_end(td);

     free(IV);
     free(key);
  return gmcrypt_block_bufferO;
}


char *encrypt_message (char *inuser, char *inmsg, int encrypt_type) {
  MCRYPT td;
  int i;
  char *key;
  char mcrpassword[65];
  char *IV;
 
  setup_mcrypt_encryption(encrypt_type);

	if (!gmcrypt_block_buffer) {gmcrypt_block_buffer=malloc(4097);}
	if (!gmcrypt_block_buffer) {return inmsg;}

  key=malloc(keysize+1+addition);
	if (!key)  {return inmsg;}

  /* The password can be up to 20 bytes */
  sprintf(mcrpassword, "%c%c%c%c%c%c%c%c", 
	gyache_mcrypt_key_string[4], gyache_mcrypt_key_string[21], 
	gyache_mcrypt_key_string[7], gyache_mcrypt_key_string[17],
	gyache_mcrypt_key_string[13], gyache_mcrypt_key_string[1], 
	gyache_mcrypt_key_string[8], gyache_mcrypt_key_string[15]
	);

	sprintf(gmcrypt_screen_name,"%s",""); /* clear old name */
	if (very_light) {
		strncpy(gmcrypt_screen_name, "nTGhbwBmQkHjDy", 12);
					 } else {strncpy(gmcrypt_screen_name, inuser, 12);}
	lower_str(gmcrypt_screen_name);  /* have to keep things consistent */
	strncat(mcrpassword,gmcrypt_screen_name,12);

	if (can_use_special_chars) {
	mcrpassword[0]=10;
	mcrpassword[2]=17;
	mcrpassword[strlen(mcrpassword)-3]=(gmcrypt_screen_name[0]+7)<121?(gmcrypt_screen_name[0]+7):121;
	mcrpassword[strlen(mcrpassword)-1]=4; 
	} else  {
	mcrpassword[6]=gyache_mcrypt_key_string[20];
	mcrpassword[strlen(mcrpassword)-3]=gyache_mcrypt_key_string[11];
	mcrpassword[strlen(mcrpassword)-1]=gyache_mcrypt_key_string[5];
	}

   if (strlen(mcrpassword)<64) {
	strncat(mcrpassword, gyache_mcrypt_key_string, 64-strlen(mcrpassword) );
   }

/* Generate the key using the password */
  memset(key, 0, sizeof(key));
  memcpy(key, mcrpassword, keysize+addition);
  key[keysize+addition]='\0';
  for (i=0; i<strlen(key); i++) { 	if (key[i]=='_') {key[i]='x';}   }
  // snprintf(key, keysize, "%s", mcrpassword);

  td = mcrypt_module_open(gmcrypt_crypt_algo, NULL, algo_method, NULL);


	 printf("keysize: %d  key is: %d \n", mcrypt_enc_get_key_size(td), strlen(key));  /* */

  if (td==MCRYPT_FAILED) {
     free(key);
     return inmsg;
  }

  IV = malloc(mcrypt_enc_get_iv_size(td));
	if (! IV  ) {free(key); return inmsg;}


/* Put random data in IV. Note these are not real random data,
 * consider using /dev/random or /dev/urandom.
 */

	/* 	Normally we would want these to be random, 
		but we have to keep them consistent across
		gyach-e instances, otherwise decrypting will
		fail on the other end, or we would have to come
		up with a way to construct and send this 'key' 
			mcrypt_enc_get_iv_size( td) always equals
			either 8 or 16 for our cases here
	*/
 
	if ( (encrypt_type==ENC_TYPE_MCRYPT_BF) || 
	(encrypt_type==ENC_TYPE_MCRYPT_TF) ) {
		can_use_special_chars=0;
	}
 
  for (i=0; i< mcrypt_enc_get_iv_size( td); i++) {
   	/* IV[i]=rand();  */
   	if (can_use_special_chars) { IV[i]=i+11; }
	else {IV[i]=gyache_mcrypt_key_string[i]; }
  }

	if (can_use_special_chars) { IV[0]=key[strlen(key)-2]; }
		else { IV[0]=gyache_mcrypt_key_string[22];} 

	/* printf("en-IV:  %s\n", IV); fflush(stdout); */

  i=mcrypt_generic_init( td, key, keysize+addition, IV);
  if (i<0) {
     /* mcrypt_perror(i); */
     free(IV);
     free(key);
     return inmsg;
  }

  /* Encryption in CFB is performed in bytes */
  i=0;
  encoded_limit=snprintf(gmcrypt_block_buffer, 2047, "%s", inmsg);
  gmcrypt_block_buffer[encoded_limit]='\0';

	while (i< encoded_limit) {
		if (gmcrypt_block_buffer[i]=='\0') {break;}
		mcrypt_generic (td, &gmcrypt_block_buffer[i], 1);
		i++;
					 }
	//gmcrypt_block_buffer[i]='\0';


/* Deinit the encryption thread, and unload the module */
  mcrypt_generic_end(td);
     free(IV);
     free(key);
  return gmcrypt_block_buffer;
}



/* #define plugin_info encryption_bf_internal_plugin_info */
int my_init();

PLUGIN_INFO plugin_info = {
	PLUGIN_ENCRYPT, 
	"MCrypt",
	"A plugin to use the following encryption ciphers through Mcrypt: XTea, Blowfish, Twofish, Tripledes, Gost, Cast-128, Cast-256, RC6, Rijndael-256, ArcFour/RC4, Serpent, Safer+, and Loki97", 
	"0.2", 
	"3/11/2004",
	"Mcrypt (mcrypt 2.5.2-22 or better)",
	"Erica Andrews [PhrozenSmoke ('at') yahoo.com]", my_init};



int my_init() {
	return 1;
}
