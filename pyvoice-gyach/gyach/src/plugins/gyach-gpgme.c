/*****************************************************************************
 * gyache-gpgme.c, Plugin to use GPG through GPGMe for encryption
 * Sends, unsigned, encrypted (ascii armored) messages
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
#include <gpgme.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* These are machine generated */

#define GPGME_PASSPH0 "ZGIyQ/h-/hyXQtnlgPYWYtjwS@!XCU*zzPtL-^WfHR@vG"
#define GPGME_PASSPH1 "gkrRH~;9Kyh_BM)5i-!.98_vn)&QkrdT^W'(W@?#:)_Z"
#define GPGME_PASSPH2 "pLQH_^P$pN?x@9#_|fdC#[.NwLy<6=O(f~MG]{HOJeHh{"
#define GPGME_PASSPH3 "G?BfU7u?8(,DDK_{Xb#7S.^V*)vVmlE5-dx!xo|2u*S:R"
#define GPGME_PASSPH4 "uWc+OAi32$@0?nEVdZ09k~9Z_)nY_OjV'N_SmIc|J-g"
#define GPGME_PASSPH5 "yHtfQ@3JEQ2Pr)FvIEuX?$yUaFDiUKGOp=7CZGk|vzNj"
#define GPGME_PASSPH6 "N?+6+5j-#rboMX|P[LQ#0_DKy6u6SzK~:S6Ce}Mfp1W?g"
#define GPGME_PASSPH7 "AmGBs.D,6BHBBsMyLH;5Nud(-MTjc+[&u$Fj/hsC,=bK1"
#define GPGME_PASSPH8 "U-uh#t6JoPP;btCDvKI~L>j:)uTwWa0.l&smw+8gYf$"
#define GPGME_PASSPH9 "rs$lh@/QeZt1z_)e{}p{ZcIJvvi!iw#kl_YUB7()nz7iZ"
#define GPGME_PASSPH10 "C~@n_z:z+YkvR>oEB/SP+jBR{F[IEaMeajUcem_nGKfv"
#define GPGME_PASSPH11 "AQqjwS30Id[Wa=Y,P76h8-}EZ;+jIg}h9pS2Dd?kIxC"
#define GPGME_PASSPH12 "LFsglJ{@Qb>q}2_C,}T1:D6$oMr'atJ.<?r)foG9Rb+Qr"
#define GPGME_PASSPH13 "p.B?5?M2z0N96G2ukmkL[m?=iJz@<PK-[jIn*t}&&M<9r"
#define GPGME_PASSPH14 "QG07e9Zu4<L?O&V,k3_L${tW2TqnSr;_:I:|!rrq/@2/C"
#define GPGME_PASSPH15 "FJHOEkGF$H_KUZo-cq?]t*v_6e<enrt6=>b!*+',PdU"
#define GPGME_PASSPH16 "sN$I|MT1_KQ4-hB39pZlei3k#i[4/zE#JGIHr{VRG)cR"
#define GPGME_PASSPH17 "j!haLEumC-lQEeIh)9Y>H@p,)U_++^As?+ViMLXnWE@z+"
#define GPGME_PASSPH18 "yuvq$q4tqi=I!cmM$@8*fj&PQ~2O,Z]'QUuRG+H9qb_RF"
#define GPGME_PASSPH19 "m:ry,uR[lRSA1m&O37+1n7<kvx&zL!X;xL5&Be_/83N"
#define GPGME_PASSPH20 "no{=DMm5Dh]RfzwbkV.s8f2QRF8@Noh=!eW_4E8V/r)rm"
#define GPGME_PASSPH21 "XKk?(9_&R;5&eCNY/A+vwND:#R^+yDASm.orErvt.,wqL"
#define GPGME_PASSPH22 "jOwFf+1o/~QaPHXJK)W[(6y$~;J1U]k@.ddqlsbys5"
#define GPGME_PASSPH23 "iXBOnfA/nUE1(T2^R)J<Ie.Ww:M^hXcR2'#~kB[Zu~h{S"
#define GPGME_PASSPH24 "kFt&X3~&dxR:1@GBq&97AUi~vT7^b>aNbVR;gQ?LKo"
#define GPGME_PASSPH25 "oEj5'yw?:!LB'pQk251{T5Ia-Z/bRvlB=WUBRM^i/,,4y"
#define GPGME_PASSPH26 "mbhGf#'RlD8M0}3;FT@,,CKu])5/3yw}]!FDaJtNl-xy"
#define GPGME_PASSPH27 "V'HkxBx[V0~-sAdFEZZo$>Wur~vwDV1w[VdTv^0Nk0Y"
#define GPGME_PASSPH28 "rZNU:b@:0mFeLa)KZ81{X_CPS{clLP}?+MrC13Z>~}$K"
#define GPGME_PASSPH29 "Ew+89Hq]N/TC#c+l7/m^$dL4)=l&0<*R42hKXZ*'7[G8"
#define GPGME_PASSPH30 "J,F2YFUw>phjf2$<[m6.9lGF&0_]2~IZ+mib5@[P2D;uT"
#define GPGME_PASSPH31 "X8tD/Jl3_?po?9Op?z>eyIJC}-I9o<yHQoi/;V@=s1-3H"
#define GPGME_PASSPH32 "a9I4az_#?uzV!WT.B1k=Wk'Ur0zGv_7Y;]kzXmzucvLEO"
#define GPGME_PASSPH33 "jdr?b}_c7oo}n,<wkdl/RxM]2Bg^oMuZ2iwrhyV|iG{Y"
#define GPGME_PASSPH34 "Xh+,a0vCal]#&QAi4E8h'Ps>E*fs|*AVpJ_SWWt:DR:H$"
#define GPGME_PASSPH35 "Wk^@_77=0#^ji.cqw=kXg}yX<_Tbl:YE&9bbMw}Zx/Fbg"


char *gpgme_block_buffer=NULL;
char *gpgme_block_bufferO=NULL;
char *gpgme_holder=NULL;
char *gpgme_incoming_user=NULL;

// const char *gpgme_gyache_passphrase="$RgiTkj@(*bNf)mjK+-)Qdm67W19IKm_@qIKMBnF,[";



static const char *passphrase_cb ( void *opaque, const char *desc, void **r_hd )
{
    int sel_pa=get_current_gpgme_passphrase();

    if ( !desc ) {
        /* cleanup by looking at *r_hd */        
        return NULL;
    }

	switch (sel_pa) {
		case 0:
			return GPGME_PASSPH0;
			break;
		case 1:
			return GPGME_PASSPH1;
			break;
		case 2:
			return GPGME_PASSPH2;
			break;
		case 3:
			return GPGME_PASSPH3;
			break;
		case 4:
			return GPGME_PASSPH4;
			break;
		case 5:
			return GPGME_PASSPH5;
			break;
		case 6:
			return GPGME_PASSPH6;
			break;
		case 7:
			return GPGME_PASSPH7;
			break;
		case 8:
			return GPGME_PASSPH8;
			break;
		case 9:
			return GPGME_PASSPH9;
			break;
		case 10:
			return GPGME_PASSPH10;
			break;
		case 11:
			return GPGME_PASSPH11;
			break;
		case 12:
			return GPGME_PASSPH12;
			break;
		case 13:
			return GPGME_PASSPH13;
			break;
		case 14:
			return GPGME_PASSPH14;
			break;
		case 15:
			return GPGME_PASSPH15;
			break;
		case 16:
			return GPGME_PASSPH16;
			break;
		case 17:
			return GPGME_PASSPH17;
			break;
		case 18:
			return GPGME_PASSPH18;
			break;
		case 19:
			return GPGME_PASSPH19;
			break;
		case 20:
			return GPGME_PASSPH20;
			break;
		case 21:
			return GPGME_PASSPH21;
			break;
		case 22:
			return GPGME_PASSPH22;
			break;
		case 23:
			return GPGME_PASSPH23;
			break;
		case 24:
			return GPGME_PASSPH24;
			break;
		case 25:
			return GPGME_PASSPH25;
			break;
		case 26:
			return GPGME_PASSPH26;
			break;
		case 27:
			return GPGME_PASSPH27;
			break;
		case 28:
			return GPGME_PASSPH28;
			break;
		case 29:
			return GPGME_PASSPH29;
			break;
		case 30:
			return GPGME_PASSPH30;
			break;
		case 31:
			return GPGME_PASSPH31;
			break;
		case 32:
			return GPGME_PASSPH32;
			break;
		case 33:
			return GPGME_PASSPH33;
			break;
		case 34:
			return GPGME_PASSPH34;
			break;
		case 35:
			return GPGME_PASSPH35;
			break;

		default:
			return GPGME_PASSPH0;
			break;
	}
	return GPGME_PASSPH0;
}


char *decrypt_message (char *inuser, char *inmsg, int encrypt_type ) {
    GpgmeCtx ctx;
    GpgmeError err;
    GpgmeData in, out;
    char gpgbuf[512];
    int nread;
    int datalen=0;

	if (! gpgme_block_bufferO) {gpgme_block_bufferO=malloc(4096);}
	if (! gpgme_block_bufferO) {return inmsg;}
	if (! gpgme_holder) {gpgme_holder=malloc(8192);}
	if (! gpgme_holder) {return inmsg;}

    err = gpgme_new (&ctx);
    if (err) {
	sprintf(gpgme_block_bufferO, "%s", gpgme_strerror(err));
	return gpgme_block_bufferO;
		}

    gpgme_set_passphrase_cb (ctx, passphrase_cb, NULL);

    datalen=snprintf(gpgme_holder,8190,"%s", inmsg);
    err = gpgme_data_new_from_mem ( &in, gpgme_holder, datalen, 0 );
    if (err) {
	sprintf(gpgme_block_bufferO, "%s", gpgme_strerror(err));
	return gpgme_block_bufferO;
		}
    err = gpgme_data_new ( &out );
    if (err) {
	sprintf(gpgme_block_bufferO, "%s", gpgme_strerror(err));
	return gpgme_block_bufferO;
		}
    err = gpgme_op_decrypt (ctx, in, out );
    if (err) {
	sprintf(gpgme_block_bufferO, "%s", gpgme_strerror(err));
    	gpgme_data_release (in);
   	gpgme_data_release (out);
    	gpgme_release (ctx);
	return gpgme_block_bufferO;
	        }

    /* Much of this below borrowed from Ayttm and other Gpgme examples */
    err = gpgme_data_rewind (out);
    if ( !err ) {
		sprintf(gpgme_block_bufferO, "%s", "");
		memset(gpgbuf, 0, sizeof(gpgbuf));
		while (!(err = gpgme_data_read (out, gpgbuf, 512, &nread))) {
			if (nread) {
				strncat(gpgme_block_bufferO, gpgbuf, nread);
				memset(gpgbuf, 0, sizeof(gpgbuf));
							}
																							 			 }
		if (err) {
			if (err != GPGME_EOF) {sprintf(gpgme_block_bufferO, "%s", gpgme_strerror(err));}
				   }
		} else {sprintf(gpgme_block_bufferO, "%s", gpgme_strerror(err));}
    gpgme_data_release (in);
    gpgme_data_release (out);
    gpgme_release (ctx);

    return gpgme_block_bufferO;
}



char *encrypt_message (char *inuser, char *inmsg, int encrypt_type ) {
    GpgmeCtx ctx;
    GpgmeError err;
    GpgmeData in, out;
    char gpgbuf[512];
    int nread;
    int datalen=0;

	if (! gpgme_block_buffer) {gpgme_block_buffer=malloc(8192);}
	if (! gpgme_block_buffer) {return inmsg;}
	if (! gpgme_holder) {gpgme_holder=malloc(8192);}
	if (! gpgme_holder) {return inmsg;}

    err = gpgme_new (&ctx);
    if (err) {
	sprintf(gpgme_block_buffer, "%s", gpgme_strerror(err));
	return gpgme_block_buffer;
		}
    gpgme_set_armor (ctx, 1);
    gpgme_set_passphrase_cb (ctx, passphrase_cb, NULL);

    datalen=snprintf(gpgme_holder,1500,"%s", inmsg);
    err = gpgme_data_new_from_mem ( &in, gpgme_holder, datalen, 0 );
    if (err) {
	sprintf(gpgme_block_buffer, "%s", gpgme_strerror(err));
	return gpgme_block_buffer;
		}
    err = gpgme_data_new ( &out );
    if (err) {
	sprintf(gpgme_block_buffer, "%s", gpgme_strerror(err));
	return gpgme_block_buffer;
		}
    err = gpgme_op_encrypt (ctx, NULL, in, out );
    if (err) {
	sprintf(gpgme_block_buffer, "%s", gpgme_strerror(err));
    	gpgme_data_release (in);
   	gpgme_data_release (out);
    	gpgme_release (ctx);
	return gpgme_block_buffer;
	        }

    /* Much of this below borrowed from Ayttm and other Gpgme examples */
    err = gpgme_data_rewind (out);
    if ( !err ) {
		sprintf(gpgme_block_buffer, "%s", "");
		memset(gpgbuf, 0, sizeof(gpgbuf));
		while (!(err = gpgme_data_read (out, gpgbuf, 512, &nread))) {
			if (nread) {
				strncat(gpgme_block_buffer, gpgbuf, nread);
				memset(gpgbuf, 0, sizeof(gpgbuf));
							}
																							 			 }
		if (err) {
			if (err != GPGME_EOF) {sprintf(gpgme_block_buffer, "%s", gpgme_strerror(err));}
				   }
		} else {sprintf(gpgme_block_buffer, "%s", gpgme_strerror(err));}
    gpgme_data_release (in);
    gpgme_data_release (out);
    gpgme_release (ctx);

    return gpgme_block_buffer;

}


/* #define plugin_info encryption_bf_internal_plugin_info */
int my_init();

PLUGIN_INFO plugin_info = {
	PLUGIN_ENCRYPT, 
	"Gpgme",
	"A plugin to use GPG encryption through Gpgme. Handles unsigned, encrypted (ascii-armored) messages.", 
	"0.1", 
	"3/05/2004",
	"GpgMe (gpgme 0.3.16 or better), GPG 1.0.7 or better",
	"Erica Andrews [PhrozenSmoke ('at') yahoo.com]", my_init};



int my_init() {
		const char *gpgmeversion = gpgme_check_version(NULL);
		if(strncmp("0.3",gpgmeversion , 3)) {			
			return 0;
		}
return 1;
}


