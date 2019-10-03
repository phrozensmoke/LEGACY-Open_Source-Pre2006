/*****************************************************************************
 * prflang.c
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
 *****************************************************************************/

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "prflang.h"


static char *langzmap[13][19]={

	{"en" ,"Yahoo! ID:" , "My Email" , "Real Name:" , "Location:" , "Age:" , "Marital Status:" , "Gender:" , "Occupation:" , "Hobbies:" , "Latest News" , "Favorite Quote" , "Home Page:" , "Cool Link 1:" , "Cool Link 2:" , "Cool Link 3:",  "Last Updated:",  "" , ""}, 

	{  "da","Yahoo! ID:", "Min Email",	"Rigtige navn:","Opholdssted:",	"Alder:","Ægteskabelig status:","Køn:","Erhverv:","Hobbyer:","Sidste nyt:", "Favoritcitat", "Forside:",	"Cool link 1:", 	"Cool link 2:",	"Cool link 3:",	"Opdateret sidste gang&nbsp;" , "",	"" 	},

	{"de" ,"Yahoo!-ID:" , "Meine E-Mail" , "Realer Name:" , "Ort:" , "Alter:" , "Familienstand:" , "Geschlecht:" , "Beruf:" , "Hobbys:" , "Neuste Nachrichten:" , "Mein Lieblingsspruch" ,  "Homepage:" , "Cooler Link 1:" , "Cooler Link 2:" , "Cooler Link 3:","Letzter Update&nbsp;",  "", ""}, 

	{"es_AR" ,"Usuario de Yahoo!:" , "Mi dirección de correo electrónico" , "Nombre real:" , "Ubicación:" , "Edad:" , "Estado civil:" , "Sexo:" , "Ocupación:" , "Pasatiempos:" , "Últimas noticias:" , "Tu cita favorita" ,  "Página de inicio:" , "Enlace genial 1:" , "Enlace genial 2:" , "Enlace genial 3:", "\332ltima actualizaci\363n&nbsp;",  "", ""}, 

	{"es_ES" ,"ID de Yahoo!:" , "Mi correo-e" , "Nombre verdadero:" , "Lugar:" , "Edad:" , "Estado civil:" , "Sexo:" , "Ocupación:" , "Aficiones:" , "Ultimas Noticias:" , "Tu cita Favorita" ,  "Página de Inicio:" , "Enlaces Preferidos 1:" , "Enlaces Preferidos 2:" , "Enlaces Preferidos 3:", "Actualizada el&nbsp;", "es.profiles.yahoo.com", ""}, 

	{"es_MX" ,"ID de Yahoo!:" , "Mi Dirección de correo-e" , "Nombre real:" , "Ubicación:" , "Edad:" , "Estado civil:" , "Sexo:" , "Ocupación:" , "Pasatiempos:" , "Ultimas Noticias:" , "Su cita favorita" ,  "Página web:" , "Enlaces Preferidos 1:" , "Enlaces Preferidos 2:" , "Enlaces Preferidos 3:","Actualizada el &nbsp;", "mx.profiles.yahoo.com", ""}, 

	{"es_US" ,"ID de Yahoo!:" , "Mi Dirección de correo-e" , "Nombre real:" , "Localidad:" , "Edad:" , "Estado civil:" , "Sexo:" , "Ocupación:" , "Pasatiempos:" , "Ultimas Noticias:" , "Su cita Favorita" , "Página de inicio:" , "Enlaces Preferidos 1:" , "Enlaces Preferidos 2:" , "Enlaces Preferidos 3:","Actualizada el &nbsp;", "", ""}, 

	{"fr_CA" ,"Compte Yahoo!:" , "Mon courriel" , "Nom réel:" , "Lieu:" , "Âge:" , "\x89tat civil:" , "Sexe:" , "Profession:" , "Passe-temps:" , "Actualités:" , "Citation préférée" ,  "Page personnelle:" , "Lien préféré 1:" , "Lien préféré 2:","Lien préféré 3:","Derni\xe8re mise \xe0 jour", "cf.profiles.yahoo.com",""}, 

	{"fr_FR" ,"Compte Yahoo!:" , "Mon E-mail","Nom réel:" , "Lieu:" , "Âge:" , "Situation de famille:" , "Sexe:" , "Profession:", "Centres d'intér\xaats:" , "Actualités:" , "Citation préférée" , "Page perso:" , "Lien préféré 1:" , "Lien préféré 2:" , "Lien préféré 3:","Derni\xe8re mise \xe0 jour", "", ""}, 

	{"it" ,"ID Yahoo!:" , "La mia e-mail" , "Nome vero:" , "Località:", "Eta':" , "Stato civile:" , "Sesso:" , "Occupazione:" , "Hobby:" , "Ultime notizie:" , "Citazione preferita" ,  "Inizio:" , "Link Preferiti 1:" , "Link Preferiti 2:" , "Link Preferiti 3:",  "Ultimo aggiornamento&nbsp;", "", ""}, 

	{ "no", "Yahoo! ID:", "Min e-post", "Virkelig navn:", "Sted:", "Alder:", "Sivilstatus:", "Kjønn:",
"Yrke:", "Hobbyer:", "Siste nytt:", "Yndlingssitat",  "Hjemmeside:","Bra lenke 1:",	"Bra lenke 2:",	"Bra lenke 3:",	"Sist oppdatert&nbsp;" , "", "" },

	{"pt" ,"ID Yahoo!:" , "Meu e-mail" , "Nome verdadeiro:" , "Local:" , "Idade:" , "Estado civil:" , "Sexo:" , "Ocupação:" , "Hobbies:" , "Últimas notícias:" , "Frase favorita" ,  "Página pessoal:" , "Site legal 1:" , "Site legal 2:" , "Site legal 3:","\332ltima atualiza\347\343o&nbsp;", "", ""},

	{ "sv", "Yahoo!-ID:","Min mail", "Riktigt namn:" ,"Plats:",  "Ålder:", "Civilstånd:", "Kön:", "Yrke:","Hobby:","Senaste nytt:","Favoritcitat", "Hemsida:",	"Coola länkar 1:",	"Coola länkar 2:",	"Coola länkar 3:",	"Senast uppdaterad&nbsp;" , "","" }

};



void select_profile_lang_strings(char *content) {
	char *spotter="";
	int langz_lim=13; /* corresponds to the number of languages */ 
	int i=0;


		/* printf("here-1\n"); fflush(stdout); */ 

	gprfl_en_lang=langzmap[0]; /* set fallbacks */
	gprfl_sel_lang=langzmap[0];

		/* in the event of crashing, just 'return' right here */ 

	 	/* printf("here-2\n"); fflush(stdout); */ 


	if (! content) {return;}

	spotter=gprfl_en_lang[GYPRFL_SEARCH];

		/* printf("here-3\n"); fflush(stdout); */ 

	if (strstr(content, spotter)) {return; }

	for (i=0; i<langz_lim; i++)  {
			/* printf("i: %d\n", i); fflush(stdout); */ 
		spotter=langzmap[i][GYPRFL_SEARCH];
			/* printf("spotter:  %s\n", spotter); fflush(stdout); */ 
		if (strstr(content, spotter)) {
			if (langzmap[i][GYPRFL_CONFIRM]) {
			   if (strcmp(langzmap[i][GYPRFL_CONFIRM], "")) {
				if (strstr(content, langzmap[i][GYPRFL_CONFIRM])) {
					gprfl_sel_lang=langzmap[i];
					/* printf("gopt-33\n"); fflush(stdout); */ 
					return; 
				}
			    } else {
				/* printf("gopt-73\n"); fflush(stdout); */ 
				gprfl_sel_lang=langzmap[i];
				return; 
			    }

			} else {
				/* printf("gopt-3\n"); fflush(stdout); */ 
				gprfl_sel_lang=langzmap[i];
				return; 
			}

		 }
	}


}



/* a simple test */ /* 
main() {
	select_profile_lang_strings(strdup("ppppppppppp Ultimo aggiornamento&nbsp; ddddddddddd"));
	printf("\nPASS-OUT:  %s\n\n", gprfl_sel_lang[GYPRFL_LANG]);
	fflush(stdout);
  return 0;
} */  



