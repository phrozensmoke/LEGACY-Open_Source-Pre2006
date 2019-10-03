#! /usr/bin/python
# -*- coding: ISO-8859-1 -*-

##########################
#  Pythoñol - PyBabelPhish Module
#
#  Copyright (c) 2002-2004
#  Erica Andrews
#  PhrozenSmoke ['at'] yahoo.com
#  http://pythonol.sourceforge.net/
#
#  This software is distributed 
#  free-of-charge and open source 
#  under the terms of the Pythoñol 
#  Free Education Initiative License.
#  
#  You should have received a copy 
#  of this license with your copy of    
#  this software. If you did not, 		
#  you may get a copy of the 	 	 
#  license at:								 
#									   
#  http://pythonol.sourceforge.net   
##########################

#####################################
#  PyBabelPhish 
#
#  PyBabelPhish is a simple gtk-based
#  frontend for the Babelfish and Google
#  translation services 
#  available on the web. PyBabelPhish
#  is written in 100% Python, does NOT
#  require Gnome, but requires PyGtk. 
#  Some languages have been removed 
#  from the list due to an inability to 
#  display certain foreign characters. 
#  PyBabelPhish is in NO way affiliated 
#  with Systran, Altavista, Babelfish, or 
#  Google and should NOT be considered 
#  "official" Babelfish software. NO
#  support is provided by the author for 
#  this software.  
#
#  Copyright (c) 2002-2004
#  Erica Andrews
#  PhrozenSmoke ['at'] yahoo.com
#  
#####################################


from pyfilter import *
import pytheme,pythonol_festival,pyfilter
if wine_friendly==0: import URLRead
from symbolbar import symbolbar
global with_window
with_window=0
global search_engine
search_engine=3  # default to SystranBox, 1=Altavista,  2=Google, 3-SystranBox

global enable_auto_engine_select  # Automatically choose appropriate translation service?
enable_auto_engine_select=1


def get_service_name(servno):
	if servno==1: return "BabelFish - Altavista"
	if servno==2: return "Google"
	if servno==3: return "SysTranBox"
	if servno==4: return "Lingua-Phile [experimental]"
	if servno==5: return "FreeTranslations"
	if servno==6: return "interNOSTRUM [Catalan/Spanish]"
	if servno==7: return "Reverso"
	if servno==8: return "Traductor Universia [Spanish/Portuguese]"
	return "SysTranBox"

def get_service_name_short(servno):
	serv=get_service_name(servno)
	if serv.find("[")>-1: serv=serv[:serv.find("[")].strip()
	return serv

banned_for_kids=['fuck',' ass',' asses',' damn','shit','bastard','asshole','jackass','dumbass','whore','whoring','bitch','slut',' tramp', 'tramps',' hoe ','hoes',' ho ','pussy','dick','penis','vagina','sodom',' rape', ' raped', ' raping ','orgasm','ejaculate','ejaculating','ejaculation',' cum',' cums','homosexual','transsexual','transexual','bisexual','sexual','sexy','erotic','have sex','condom','abortion','prostitute','prostitution','sleazy','testicles','cunt','breast',' tit',' tits','masturbate','masturbated','masturbation','maturbating','jerk off','jack off','whack off','bestiality','pedophile','child molester','pedophilia',' org ',' orgie ','make love','blowjob','blow job','handjob', 'hand job',' anal ','anally',' piss ','pissed off','pissing',' pissed','go to hell','pussies','faggot','fags',' fa ',' sissy ','pussy','clit','clitoris','labia','scrotum','felatio','fellatio','cunilingus','incest']


class babelwin :
    def __init__(self,bwin=None,help_func=None) :
	global with_window

	# Altavista supports a subset of what SysTranBox does
	self.lp_values={"English to French":"en_fr","English to German":"en_de","English to Italian":"en_it","English to Portuguese":"en_pt","English to Spanish":"en_es","French to English":"fr_en","French to German":"fr_de","German to English":"de_en","German to French":"de_fr","Italian to English":"it_en","Portuguese to English":"pt_en","Spanish to English":"es_en"}

	# SysTranbox
	self.lp_values3={"English to French":"en_fr","English to German":"en_de","English to Italian":"en_it","English to Portuguese":"en_pt","English to Spanish":"en_es","French to English":"fr_en","French to German":"fr_de","German to English":"de_en","German to French":"de_fr","Italian to English":"it_en","Portuguese to English":"pt_en","Spanish to English":"es_en", "English to Dutch":"en_nl", "Dutch to English":"nl_en", "French to Dutch":"fr_nl", "Dutch to French":"nl_fr", "French to Spanish":"fr_es", "Spanish to French":"es_fr", "French to Portuguese":"fr_pt", "Portuguese to French":"pt_fr", "French to Italian":"fr_it", "Italian to French":"it_fr"}

	# Google
	self.lp_values2={"English to French":"en|fr","English to German":"en|de","English to Italian":"en|it","English to Portuguese":"en|pt","English to Spanish":"en|es","French to English":"fr|en","French to German":"fr|de","German to English":"de|en","German to French":"de|fr","Italian to English":"it|en","Portuguese to English":"pt|en","Spanish to English":"es|en"}

	# lingua-phile, a nice GPL ad-free translation service, many languages to English
	self.lp_values4={
	"Afrikaans to English":"af"  ,
	"Alawa to English": "ALH",
	"Albanian to English":"sq" ,
	"Arrernte to English": "AER" ,
	"Basque to English": "eu" ,
	"Belarusian to English": "be",
	"Bulgarian to English": "bg",
	"Catalan to English": "ca" ,
	"Croatian to English": "hr" ,
	"Czech to English":"cs" ,
	"Danish to English": "da" ,
	"Dutch to English":"nl" ,
	"Esperanto to English": "eo" ,
	"Estonian to English": "et",
	"Finnish to English": "fi",
	"French to English": "fr",
	"Galician to English":"gl"  ,
	"German to English":"de"  ,
	"Greek to English": "el",
	"Hawaiian to English": "haw",
	"Hungarian to English":"hu" ,
	"Icelandic to English": "is",
	"Indonesian to English": "id",
	"Interlingua to English": "ia",
	"Gaelic to English": "ga",
	"Italian to English": "it",
	"Kala Lagaw Ya to English":"MWP" ,
	"Korean to English": "ko",
	"Kriol to English":"ROP" ,
	"Latvian to English":"lv" ,
	"Lithuanian to English":"lt"  ,
	"Malay to English": "ms",
	"Maltese to English": "mt",
	"Maori to English": "mi" ,
	"Marshallese to English": "mh" ,
	"Norwegian to English": "no" ,
	"Pitjantjatjara to English": "PJT",
	"Polish to English": "pl",
	#  "Portuguese to English": "pt" ,  # doesn't work 
	"Romanian to English": "ro",
	"Russian to English":"ru" ,
	"Samoan to English": "sm",
	"Serbian to English": "sr",
	"Slovakian to English": "sk",
	"Slovenian to English": "sl",
	"Spanish to English": "es",
	"Swahili to English": "sw",
	"Swedish to English": "sv",
	"Tagalog to English": "tl" ,
	"Thai to English": "th" ,
	"Tok Pisin to English":"tpi" ,
	"Turkish to English":"tr" ,
	"Ukrainian to English": "uk",
	"Warlpiri to English": "WBP",
	"Welsh to English": "cy",
		}

	self.lp_values5={  # FreeTranslations.com, English to Norwegian!
	"English to Spanish":  "English/Spanish",
	"English to French": "English/French" ,
	"English to German": "English/German" ,
	"English to Italian": "English/Italian" ,
	"English to Dutch": "English/Dutch" ,
	"English to Portuguese":  "English/Portuguese",
	"English to Norwegian": "English/Norwegian" ,
	"Spanish to English": "Spanish/English" ,
	"French to English": "French/English" ,
	"German to English": "German/English" ,
	"Italian to English":  "Italian/English",
	"Dutch to English": "Dutch/English" ,
	"Portuguese to English":"Portuguese/English"
			}

	# interNOSTRUM, specialized Spanish-Catalan
	self.lp_values6 = {"Catalan to Spanish":"ca-es", "Spanish to Catalan":"es-ca"}

	# Reverso
	self.lp_values7={
		"English to French":"524289",
		"German to French":"524292", 
		"French to English":"65544",
		"French to German":"262152",
		"German to English":"65540",
		"English to German":"262145",
		"Spanish to French":"65568",
		"French to Spanish":"2097160", 
		"Spanish to English":"65568", 
		"English to Spanish":"2097153",
					}

	# Traductor Universia, specialized Spanish-Portuguese
	self.lp_values8 = {"Portuguese to Spanish":"pt-es", "Spanish to Portuguese":"es-pt"}

	self.full_quit=0
	self.babel_version=PYBABELPHISH_VERSION
	mainvbox=VBox(0,3)
	pytheme.installTheme(mainvbox)
	pytheme.setWindowForUpdate(mainvbox)

	if with_window==1:
		babelwin=Window(WINDOW_TOPLEVEL)
		babelwin.set_title('PyBabelPhish '+self.babel_version)
		babelwin.set_position(WIN_POS_CENTER)
		set_window_icon(babelwin,pybabel_icon)
		babelwin.set_wmclass("pybabelphish","PyBabelPhish")
		babelwin.realize()
		pytheme.setWindowForUpdate(babelwin)

		self.babelwin=babelwin

		menubar1=MenuBar()
		pytheme.setWindowForUpdate(menubar1)

        	ag=AccelGroup()
		self.accel_group=ag
        	itemf=ItemFactory(MenuBar,"<main>",ag)
		self.item_factory=itemf

		# Using 'StockItem' below for menu icons
		# instead of 'ImageItem' because ImageItem
		# causes 'gtk warnings' on at least Gtk 2.2.4
		# on Windows
		
        	itemf.create_items([
            	# path              key           callback    action#  type
 		 ("/ _File", "<alt>F", None, 0,"<Branch>"),
 		 ("/ _File/_Save Translation...","<control>S",self.saveTranslation,9,"<StockItem>", STOCK_SAVE ),
 		 ("/ _File/_sep1",None,None,8,"<Separator>"),
 		 ("/ _Translation Service", "<alt>T", None, 20,"<Branch>"),
 		 ("/ _Translation Service/_Change Translation Service...","<alt>S",self.askService,99,"<StockItem>", STOCK_PREFERENCES),
                 ("/ _File/_Settings...","<alt>A",pytheme.showThemeSelector,904,"<StockItem>", STOCK_SELECT_COLOR),
 		 ("/ _File/_sep2",None,None,8,"<Separator>"),
 		 ("/ _File/_Quit","<control>Q",self.doShutdown,10,"<StockItem>", STOCK_QUIT),
 		 ("/_Help", "<alt>H", None,16,"<LastBranch>"),
		  ("/_Help/_About PyBabelPhish...","F2",self.showAbout,20,"<StockItem>", STOCK_DIALOG_INFO),
        	])


        	babelwin.add_accel_group(self.accel_group)
        	menubar1=self.item_factory.get_widget("<main>")
        	menubar1.show()
		self.menubar=menubar1
		mainvbox.pack_start(menubar1,0,0,0)
		bwin=self.babelwin

	widgetbox=VBox(0,0)
	widgetbox.set_border_width(5)
	babtitle=loadPixmap(getPixDir()+"babtitle.gif",bwin)
	if not babtitle: babtitle=Label('PyBabelPhish '+self.babel_version)
	widgetbox.pack_start(babtitle,0,0,4)
	tranbox=HBox(0,0)
	tranbox.set_spacing(4)
	label2=Label('Text to translate:')
	label2.set_alignment(0,0.5)
	tranbox.pack_start(label2,1,1,0)
	widgetbox.pack_start(tranbox,0,0,0)
	babscroll1=ScrolledWindow()
	babscroll1.set_size_request(-1,100)
	babtext1=TextView()
	addDragSupport(babtext1,setDrag)
	babtext1.set_editable(1)
	self.babtext1=babtext1
	babscroll1.add(babtext1)
	widgetbox.pack_start(babscroll1,1,1,0)
	symbar=symbolbar(self.babtext1,bwin)
	symbar.text_type=2
	#symbar.pack_start(Label(""),1,1,0)
	widgetbox.pack_start(symbar,0,0,0)
	hseparator1=HSeparator()
	widgetbox.pack_start(hseparator1,0,0,5)
	hbox1=HBox(0,7)
	label4=Label('Language:')
	self.label4=label4
	hbox1.pack_start(label4,0,0,0)
	babcombo=Combo()
	bablp={}
	bablp.update(self.lp_values)
	bablp.update(self.lp_values2)
	bablp.update(self.lp_values3)
	bablp.update(self.lp_values4)
	bablp.update(self.lp_values5)
	bablp.update(self.lp_values6)
	bablp.update(self.lp_values7)
	bablp.update(self.lp_values8)
	bablist=bablp.keys()
	bablist.sort()
	babcombo.set_popdown_strings(bablist)
	babcombo.set_size_request(235,-1)
	TIPS.set_tip(babcombo,"Select the Languages")
	babentry=babcombo.entry
	babentry.set_text("English to Spanish")
	babentry.set_editable(0)
	self.babentry=babentry
	hbox1.pack_start(babcombo,1,1,0)
	transbutton=getIconButton (None,STOCK_REFRESH,'Translate!',1)
	TIPS.set_tip(transbutton,"Translate")
	transbutton.connect("clicked",self.launchTranslate)
	self.transbutton=transbutton
	hbox1.pack_start(transbutton,0,0,0)
	if not with_window==1:
		hbox1.pack_start(Label("  "),0,0,0)
		savebutton=getIconButton (None,STOCK_SAVE,"Save",1)
		TIPS.set_tip(savebutton,"Save")
		self.savebutton=savebutton
		savebutton.connect("clicked",self.saveTranslation)
		hbox1.pack_start(savebutton,0,0,0)

		helpbutton=getIconButton (None,STOCK_DIALOG_QUESTION,"Help",1)
		TIPS.set_tip(helpbutton,"Help")
		self.helpbutton=helpbutton
		helpbutton.connect("clicked",help_func)
		hbox1.pack_start(helpbutton,0,0,0)

		aboutbutton=getIconButton (None,STOCK_DIALOG_INFO,"About",1)
		TIPS.set_tip(aboutbutton,"About PyBabelPhish")
		self.aboutbutton=aboutbutton
		aboutbutton.connect("clicked",self.showAbout)
		hbox1.pack_start(aboutbutton,0,0,0)
	widgetbox.pack_start(hbox1,0,0,0)
	hseparator2=HSeparator()
	widgetbox.pack_start(hseparator2,0,0,5)
	label3=Label('Translation:')
	label3.set_alignment(0,0.5)
	self.label3=label3
	widgetbox.pack_start(label3,0,0,0)
	babscroll2=ScrolledWindow()
	babscroll2.set_size_request(-1,85)
	babtext2=TextView()
	babtext2.set_wrap_mode(WRAP_WORD)
	babtext1.set_wrap_mode(WRAP_WORD)
	babtext2.set_editable(1)
	self.babtext2=babtext2
	babscroll2.add(babtext2)

	babscroll2box=HBox(0,0)
	babscroll2box.pack_start(pythonol_festival.FestivalBar(bwin,babtext2,2),0,0,0)
	babscroll2box.pack_start(babscroll2,1,1,0)
	widgetbox.pack_start(babscroll2box,1,1,0)

	label5=Label('PyBabelPhish requires an internet connection.')
	widgetbox.pack_start(label5,0,0,1)
	mainvbox.pack_start(widgetbox,1,1,0)
	self.mainvbox=mainvbox
	pytheme.installTheme(mainvbox)
	pytheme.installTheme(widgetbox)
	if with_window==1:
		babelwin.add(mainvbox)
		pytheme.installTheme(babelwin)
		babelwin.connect("destroy",self.doShutdown)
		babelwin.show_all()
		#self.setText(self.babtext2,str(sys.argv)+"  "+os.getcwd())

    def showMessage(self,message_text):
	return msg_info("PyBabelPhish "+self.babel_version, to_utf8(str(message_text)))

    def showConfirm(self,message_text):
	return msg_confirm("PyBabelPhish "+self.babel_version, to_utf8(str(message_text)))

    def saveTranslation(self,*args):
	if (not self.getText(self.babtext1).strip()) and (not self.getText(self.babtext2).strip()):
		self.showMessage("Error:  There is no translation to save!")
		return
	SELECT_A_FILE(self.saveTranslationCB,FILE_SELECTOR_TITLE,"pybabelphish","PyBabelPhish",None,"Save","Cancel")

    def saveTranslationCB(self,*args):
	ff=GET_SELECTED_FILE()
	if ff==None: return
	if ff=="": return
	if ff.endswith("/"): return # a directory
	try:
		if wine_friendly==0:
			if os.path.exists(ff):
				if not self.showConfirm("The file '"+ff+"' already exists.\nReplace It?")==1: return
		fbab=open(ff,"w")
		fbab.write(unfilter(self.getText(self.babtext1).strip()+"\n\n\n"+self.getText(self.babtext2).strip()))
		fbab.flush()
		fbab.close()
		self.showMessage("Translation successfully saved as plain text to the file:\n'"+str(ff)+"'")
	except:
		self.showMessage("There was an ERROR saving the translation to the file:\n'"+str(ff)+"'")

    def doShutdown(self,*args):
	self.babelwin.unmap()
	if self.full_quit==1: gtk.mainquit()


    def who_supports_it(self,langcombination):
	services=[]
	if self.lp_values2.has_key(langcombination): services.append(get_service_name(2))
	if self.lp_values.has_key(langcombination): services.append(get_service_name(1))
	if self.lp_values3.has_key(langcombination): services.append(get_service_name(3))
	if self.lp_values5.has_key(langcombination): services.append(get_service_name(5))
	if self.lp_values7.has_key(langcombination): services.append(get_service_name(7))
	# specialized or expermental engines suggested last
	if self.lp_values8.has_key(langcombination): services.append(get_service_name(8))
	if self.lp_values6.has_key(langcombination): services.append(get_service_name(6))
	if self.lp_values4.has_key(langcombination): services.append(get_service_name(4))
	if len(services)>0:
		ss="\n\nHowever, this combination of languages is supported by the services(s):\n"
		for ii in services:
			ss=ss+"\n    "+ii
		return ss
	else: return ""

    def who_supports_it_num(self,langcombination):
	if self.lp_values3.has_key(langcombination): return 3  # SysTranBox
	if self.lp_values2.has_key(langcombination): return 2  # Google
	if self.lp_values.has_key(langcombination): return 1  # Altavista
	if self.lp_values5.has_key(langcombination): return 5  # FreeTranslations
	if self.lp_values7.has_key(langcombination): return 7  # Reverso
	if self.lp_values6.has_key(langcombination): return 6  # interNOSTRUM
	if self.lp_values8.has_key(langcombination): return 8  # Traductor Universia
	if self.lp_values4.has_key(langcombination): return 4  # Lingua-Phile
	return 3

    def select_language_names(self):
	global search_engine
	mydict=self.lp_values3
	if search_engine==1:
		mydict=self.lp_values
	if search_engine==2: 
		mydict=self.lp_values2
	if search_engine==3:
		mydict=self.lp_values3
	if search_engine==4:
		mydict=self.lp_values4
	if search_engine==5:
		mydict=self.lp_values5
	if search_engine==6:
		mydict=self.lp_values6
	if search_engine==7:
		mydict=self.lp_values7
	if search_engine==8:
		mydict=self.lp_values8
	return mydict

    def launchTranslate(self,*args):
	if not self.babentry.get_text(): return
	if not self.getText(self.babtext1).strip(): return
	global search_engine
	searchtext=self.getText(self.babtext1).strip()
	mylp="en_es"
	mydict=self.select_language_names()
	service=get_service_name(search_engine)

	if mydict.has_key(self.babentry.get_text()): mylp=mydict[self.babentry.get_text()]
	else: 
		if enable_auto_engine_select:
			search_org=search_engine
			search_engine=self.who_supports_it_num(self.babentry.get_text())
			mydict=self.select_language_names()
			if mydict.has_key(self.babentry.get_text()): 
				mylp=mydict[self.babentry.get_text()]
				babdata2=doTranslate(searchtext,mylp)
				self.setText(self.babtext2,babdata2)
				self.label3.set_text("Translation:    ("+get_service_name_short(search_engine)+")")
				search_engine=search_org
				return
			search_engine=search_org
		self.showMessage("Sorry, the translation service '"+service+"' is not capable of translating this combination of languages."+self.who_supports_it(self.babentry.get_text()))
		return
	babdata2=doTranslate(searchtext,mylp)
	self.setText(self.babtext2,babdata2)
	self.label3.set_text("Translation:    ("+get_service_name_short(search_engine)+")")

    def showAbout(self,*args):
	self.showMessage("PyBabelPhish "+self.babel_version+"\n\nPyBabelPhish is a simple GTK-based frontend for several web-based translation services.  PyBabelPhish is written in 100% Python, does NOT require Gnome,  but requires PyGtk-2.  Many languages from Russia, the Baltic region, Asia, and the Middle East are not fully supported due to difficulty displaying certain foreign characters.  This program focuses on translating European-based languages which use Latin character sets.  PyBabelPhish is in NO way affiliated with or endorsed by the translation services the program accesses and should NOT be considered 'official' software from said translation services.  NO support is provided by the author for this software. \n\nThis software is free and open source under the Free Education Initiative License.\n\nCopyright (c) 2002-2004\nErica Andrews\nPhrozenSmoke@yahoo.com\nhttp://pythonol.sourceforge.net\n")

    def askService(self,*args):
	swin=Window(WINDOW_TOPLEVEL)
	set_window_icon(swin,pybabel_icon)
	swin.set_wmclass("pybabelphish","PyBabelPhish")
	swin.realize()
	swin.set_title('PyBabelPhish '+self.babel_version)
	swin.set_position(WIN_POS_CENTER)
	theshbox=VBox(0,0)
	theshbox.set_border_width(6)
	theshbox.pack_start(Label(" Select the translation service you prefer to use: "),0,0,5)	
	thesradio1=RadioButton(None," "+get_service_name(2)+" ")	
	TIPS.set_tip(thesradio1," "+get_service_name(2)+" ")
	theshbox.pack_start(thesradio1,0,0,2)
	thesradio2=RadioButton(thesradio1," "+get_service_name(1)+" ")
	TIPS.set_tip(thesradio2," "+get_service_name(1)+" ")
	theshbox.pack_start(thesradio2,0,0,2)
	thesradio3=RadioButton(thesradio1," "+get_service_name(3)+" ")
	TIPS.set_tip(thesradio3," "+get_service_name(3)+" ")
	theshbox.pack_start(thesradio3,0,0,2)
	thesradio4=RadioButton(thesradio1," "+get_service_name(4)+" ")
	TIPS.set_tip(thesradio4," "+get_service_name(4)+" ")
	theshbox.pack_start(thesradio4,0,0,2)
	thesradio5=RadioButton(thesradio1," "+get_service_name(5)+" ")
	TIPS.set_tip(thesradio5," "+get_service_name(5)+" ")
	theshbox.pack_start(thesradio5,0,0,2)
	thesradio6=RadioButton(thesradio1," "+get_service_name(6)+" ")
	TIPS.set_tip(thesradio6," "+get_service_name(6)+" ")
	theshbox.pack_start(thesradio6,0,0,2)
	thesradio7=RadioButton(thesradio1," "+get_service_name(7)+" ")
	TIPS.set_tip(thesradio7," "+get_service_name(7)+" ")
	theshbox.pack_start(thesradio7,0,0,2)
	thesradio8=RadioButton(thesradio1," "+get_service_name(8)+" ")
	TIPS.set_tip(thesradio8," "+get_service_name(8)+" ")
	theshbox.pack_start(thesradio8,0,0,2)
	theshbox.pack_start(HSeparator(),0,0,2)
	autosel=CheckButton("If the selected translation service does not support\nthe languages I am translating, automatically\nretrieve a translation from a service that does.")
	TIPS.set_tip(autosel,"If the selected translation service does not support\nthe languages I am translating, automatically\nretrieve a translation from a service that does.")
	theshbox.pack_start(autosel,0,0,2)
	global ENGINESEL
	ENGINESEL=autosel
	global MYRAD1
	global MYRAD2
	global MYRAD3
	global MYRAD4
	global MYRAD5
	global MYRAD6
	global MYRAD7
	global MYRAD8
	global enable_auto_engine_select
	if search_engine==1:
		thesradio2.set_active(1)
	if search_engine==2:
		thesradio1.set_active(1)
	if search_engine==3:
		thesradio3.set_active(1)
	if search_engine==4:
		thesradio4.set_active(1)
	if search_engine==5:
		thesradio5.set_active(1)
	if search_engine==6:
		thesradio6.set_active(1)
	if search_engine==7:
		thesradio7.set_active(1)
	if search_engine==8:
		thesradio7.set_active(1)
	autosel.set_active(enable_auto_engine_select)
	MYRAD1=thesradio1
	MYRAD2=thesradio2
	MYRAD3=thesradio3
	MYRAD4=thesradio4
	MYRAD5=thesradio5
	MYRAD6=thesradio6
	MYRAD7=thesradio7
	MYRAD8=thesradio8
	okbutton=getIconButton (None,STOCK_YES,  DIALOG_OK,1)
	TIPS.set_tip(okbutton,"Select 'OK' when you have chosen your preferred translation service.")
	okbutton.connect("clicked",self.setService,swin)
	theshbox.pack_start(okbutton,0,0,4)
	swin.add(theshbox)
	swin.set_modal(1)
	swin.connect("destroy",self.setService,swin)
	swin.show_all()

    def setService(self,*args):
	global MYRAD1
	global MYRAD2
	global MYRAD3
	global MYRAD4
	global MYRAD5
	global MYRAD6
	global MYRAD7
	global MYRAD8
	global ENGINESEL
	global enable_auto_engine_select
	global search_engine
	search_engine=3
	enable_auto_engine_select=ENGINESEL.get_active()
	if MYRAD1.get_active(): search_engine=2
	if MYRAD2.get_active(): search_engine=1
	if MYRAD3.get_active(): search_engine=3
	if MYRAD4.get_active(): search_engine=4
	if MYRAD5.get_active(): search_engine=5
	if MYRAD6.get_active(): search_engine=6
	if MYRAD7.get_active(): search_engine=7
	if MYRAD8.get_active(): search_engine=8
	args[1].hide()
	args[1].destroy()
	args[1].unmap()

    def setText(self,temptext,textdata):
	text_buffer_set_text(temptext, str(textdata))

    def getText(self,text_widget) :
	return text_buffer_get_text(text_widget)

## shared methods
def doTranslate(searchtext,mylang=None):
	global search_engine
	#http://babelfish.altavista.com/tr?doit=done&tt=urltext&lp=en_es&urltext=cuando%20tu%20me%20besas&intl=1
	#http://babelfish.altavista.com/babelfish/tr?doit=done&tt=urltext&intl=1&urltext=in+the+evening&lp=en_es

	if child_friendly==1:
		for gg in banned_for_kids:
			searchtext=searchtext.lower().replace(gg," ")
		searchtext=searchtext.replace("  "," ").replace("  "," ").replace("  "," ").replace("  "," ").replace("  "," ").strip()
	myparams={}
	mylp="en_es"
	if search_engine==2: mylp="en|es"
	if search_engine==3: mylp="en_es"
	if search_engine==4: mylp="es"
	if search_engine==5: mylp= "English/Spanish"
	if search_engine==6: mylp= "ca-es"
	if search_engine==7: mylp= "2097153"
	if search_engine==8: mylp= "es-pt"

	by_pass=0
	if search_engine==4: # Lingua-Phile doesnt do English-Spanish, so redirect to SysTran
		by_pass=1
	if search_engine==6: # interNOSTRUM doesnt do English-Spanish, so redirect to SysTran
		by_pass=1
	if search_engine==8: # Traductor Universia doesnt do English-Spanish, so redirect to SysTran
		by_pass=1

	if by_pass==1:
		search_org=search_engine
		if mylang==None:
			search_engine=3
			bdata=doTranslate(searchtext,None)
			search_engine=search_org
			return bdata

	if not mylang==None: mylp=mylang
	if search_engine==1:
		myparams['doit']='done'
		myparams['tt']='urltext'
		myparams['intl']='1'
		myparams['lp']=mylp
		myparams['urltext']=searchtext
	if search_engine==2:
		myparams['text']=searchtext
		myparams['langpair']=mylp
		myparams['hl']='en'
		myparams['ie']='ASCII'
		myparams['oe']='ASCII'
	if search_engine==3:
		myparams['systran_text']=searchtext
		myparams['systran_lp']=mylp
		myparams['ttype']='text'
		myparams['systran_url']='http://www.'
		myparams['x']='59'
		myparams['y']='10'
		myparams['systran_charset']='utf-8'
		myparams['systran_id']='SystranSoft-en'
	if search_engine==4:  # Lingua-Phile
		myparams['txt']=searchtext
		myparams['txt_src_lang']=mylp
		myparams['txt_dst_lang']='en'
		myparams['txt_submit']='Submit'
	if search_engine==5:  # ets freetranslations
		myparams['srctext']=searchtext
		myparams['language']=mylp
		myparams['sequence']='core'
		myparams['mode']='html'
		myparams['template']='results_en-us.htm'
	if search_engine==6:  # interNOSTRUM
		myparams['quadretext']=searchtext
		myparams['direccio']=mylp
		myparams['tipus']='txtf'
	if search_engine==7:  # Reverso
		myparams['source']=searchtext
		myparams['directions']=mylp
		myparams['status']='translate'
		myparams['templates']='General'
		myparams['detour']=''
		myparams['accent1']='à'
		myparams['accent2']='é'
		myparams['accent3']='î'
		myparams['accent4']='ô'
		myparams['accent5']='ù'
		myparams['accent6']='ç'
		myparams['status']='translate'
	if search_engine==8:  # Traductor Universia
		myparams['quadretext']=searchtext
		myparams['direccio']=mylp
		myparams['tipus']='txtf'
	babdata2=filter(getTranslation(myparams))
	return babdata2

def accentReplace(bdata):
	bdata=convert_to_unicode(bdata)
	bdata=bdata.replace("Ã¡","á").replace("Ã±","ñ").replace("Ã©","é").replace("Ã¼","ü").replace("Ã­","í").replace("Ã³","ó").replace("Ãº","ú").replace("Â¡","¡").replace("Â¿","¿").replace("\xc3\x89","É").replace("\xc3\x9a","Ú").replace("<pr>","").replace("#","") 
	return bdata

def getTranslation(myparamss):
	if wine_friendly==1: return "Sorry, translations cannot be retrieved will running under Wine."
	murl="http://babelfish.altavista.com/babelfish/tr"
	mref="http://babelfish.altavista.com/babelfish/tr"
	global search_engine
	if search_engine==2:
		murl="http://translate.google.com/translate_t"
		mref="http://translate.google.com/translate_t"
	if search_engine==3:
		murl="http://www.systranbox.com/systran/box"
		mref="http://www.systranbox.com/systran/box"
	if search_engine==4:
		murl="http://linguaphile.sourceforge.net/cgi-bin/translator.pl"
		mref="http://linguaphile.sourceforge.net/cgi-bin/translator.pl"
	if search_engine==5:
		murl="http://ets.freetranslation.com/"
		mref="http://ets.freetranslation.com/"
	if search_engine==6:
		murl="http://www.internostrum.com/tradquadtext.php"
		mref="http://www.internostrum.com/tradquadtext.php"
	if search_engine==7:
		murl="http://www.reverso.net/textonly/text_translation.asp"
		mref="http://www.reverso.net/textonly/text_translation.asp"
	if search_engine==8:
		murl="http://copacabana.dlsi.ua.es/es/tradquadtext.php"
		mref="http://copacabana.dlsi.ua.es/es/tradquadtext.php"
	#print "STUFF:  "+murl
	#print "STUFF:  "+mref
	#print "STUFF:  "+str(myparamss)
	babdata=str(URLRead.openUrl(murl,None,0,myparamss,mref,0))
	if babdata.lower().find("this text cannot be translated") >-1: babdata=""
	if len(babdata)>5:

	  if search_engine==1:  # Altavista
		if (babdata.find("<Div style=padding:10px; lang=")> -1) and (babdata.find("</div></td>")> babdata.find("<Div style=padding:10px; lang=")):
			babdata=babdata[babdata.find("<Div style=padding:10px; lang=")+len("<Div style=padding:10px; lang="):len(babdata)]
			babdata=babdata[babdata.find(">")+1:babdata.find("</div></td>")]

	  if search_engine==2:  # Google
		if (babdata.find("wrap=PHYSICAL>")> -1) and (babdata.find("</textarea>")> babdata.find("wrap=PHYSICAL>")):
			babdata=babdata[babdata.find("wrap=PHYSICAL>")+len("wrap=PHYSICAL>"):len(babdata)]
			babdata=babdata[0:babdata.find("</textarea>")]

	  if search_engine==3:  # Systranbox
		if (babdata.find("<textarea name=\"translation\"")> -1) and (babdata.find("</textarea>")> babdata.find("<textarea name=\"translation\"")):
			babdata=babdata[babdata.find("<textarea name=\"translation\"")+len("<textarea name=\"translation\""):len(babdata)]	
			babdata=babdata[babdata.find(">")+1:babdata.find("</textarea>")]

	  if search_engine==4:  # Lingua-Phile
		if (babdata.find("</FORM>")> -1) and (babdata.find("</BODY>")> babdata.find("</FORM>")):
			babdata=babdata[babdata.find("</FORM>")+len("</FORM>"):len(babdata)]
			babdata=babdata[0:babdata.find("</BODY>")]
			if babdata.find(">")>-1: babdata=babdata[babdata.find(">")+1:]
			babdata=babdata.replace("</span>","").strip()

	  if search_engine==5:  # FreeTranslations.com
		if (babdata.find("<textarea name=\"dsttext\"")> -1) and (babdata.find("</textarea>")> babdata.find("<textarea name=\"dsttext\"")):
			babdata=babdata[babdata.find("<textarea name=\"dsttext\"")+len("<textarea name=\"dsttext\""):len(babdata)]
			babdata=babdata[0:babdata.find("</textarea>")]
			if babdata.find(">")>-1: babdata=babdata[babdata.find(">")+1:]

	  if search_engine==6:  # interNOSTRUM
		if (babdata.find("<p class=\"textonormal\">")> -1) :
			babdata=babdata[babdata.find("<p class=\"textonormal\">")+len("<p class=\"textonormal\">"):len(babdata)]
			if babdata.find("</p>")>0: babdata=babdata[0:babdata.find("</p>")]

	  if search_engine==7:  # Reverso
	    if (babdata.find("translation is here")> -1) :
		babdata=babdata[babdata.find("translation is here")+len("translation is here"):len(babdata)]
		if (babdata.find("<TEXTAREA")> -1) :
			babdata=babdata[babdata.find("<TEXTAREA")+len("<TEXTAREA"):len(babdata)]
			if babdata.find("</TEXTAREA>")>0: babdata=babdata[0:babdata.find("</TEXTAREA>")]
			if babdata.find(">")>-1: babdata=babdata[babdata.find(">")+1:]

	  if search_engine==8:  # Traductor Universia
		if (babdata.find("del texto original</font>")> -1) :
			babdata=babdata[babdata.find("del texto original</font>")+len("del texto original</font>"):len(babdata)]
			if (babdata.find("size=\"2\">")> -1) :
				babdata=babdata[babdata.find("size=\"2\">")+len("size=\"2\">"):len(babdata)]
				if babdata.find("</font>")>0: babdata=babdata[0:babdata.find("</font>")]

	  babdata=babdata.replace("<br>","").replace("<BR>","").replace("\r","").strip()
	  babdata=accentReplace(""+babdata).replace("\t"," ").strip()
	  if pyfilter.child_friendly==1:
		for gg in banned_for_kids:
			babdata=babdata.lower().replace(gg,"[censored]")
		babdata=babdata.replace("  "," ").replace("  "," ").replace("  "," ").replace("  "," ").strip()
	  if len(babdata.strip())==0: 
		babdata="The translation service could not translate this text."
	else: babdata="Sorry, the data returned from the translation server could not be read. Translation failed. The translation server may be temporarily unavailable or you might not be connected to the internet at this time. PyBabelPhish REQUIRES a working internet connection to function and retrieve translations. You may want to try selecting a different 'Translation Service'. Please try again.\n________\n"+babdata
	return babdata


def getIconButton (iwin,picon,lab="?",both=0) :  
	return getPixButton(iwin,picon,lab,both)

####################################


iso_unicode = {
'Aacgr': 0x0386,
'aacgr': 0x03AC,
'Aacute': 0x00C1,
'aacute': 0x00E1,
'Abreve': 0x0102,
'abreve': 0x0103,
'Acirc': 0x00C2,
'acirc': 0x00E2,
'acute': 0x00B4,
'Acy': 0x0410,
'acy': 0x0430,
'AElig': 0x00C6,
'aelig': 0x00E6,
'Agr': 0x0391,
'agr': 0x03B1,
'Agrave': 0x00C0,
'agrave': 0x00E0,
'alefsym': 0x2135,
'aleph': 0x2135,
'Alpha': 0x0391,
'alpha': 0x03B1,
'Amacr': 0x0100,
'amacr': 0x0101,
'amalg': 0x2210,
'amp': 0x0026,
'and': 0x2227,
'ang': 0x2220,
'ang90': 0x221F,
'angmsd': 0x2221,
'angsph': 0x2222,
'angst': 0x212B,
'Aogon': 0x0104,
'aogon': 0x0105,
'ap': 0x2248,
'ape': 0x224A,
'apos': 0x02BC,
'Aring': 0x00C5,
'aring': 0x00E5,
'ast': 0x002A,
'asymp': 0x2248,
'Atilde': 0x00C3,
'atilde': 0x00E3,
'Auml': 0x00C4,
'auml': 0x00E4,
'b.alpha': 0x03B1,
'barwed': 0x22BC,
'Barwed': 0x2306,
'b.beta': 0x03B2,
'b.chi': 0x03C7,
'bcong': 0x224C,
'Bcy': 0x0411,
'bcy': 0x0431,
'b.Delta': 0x0394,
'b.delta': 0x03B3,
'bdquo': 0x201E,
'becaus': 0x2235,
'bepsi': 0x220D,
'b.epsi': 0x03B5,
'b.epsis': 0x03B5,
'b.epsiv': 0x03B5,
'bernou': 0x212C,
'Beta': 0x0392,
'beta': 0x03B2,
'b.eta': 0x03B7,
'beth': 0x2136,
'b.Gamma': 0x0393,
'b.gamma': 0x03B3,
'b.gammad': 0x03DC,
'Bgr': 0x0392,
'bgr': 0x03B2,
'b.iota': 0x03B9,
'b.kappa': 0x03BA,
'b.kappav': 0x03F0,
'b.Lambda': 0x039B,
'b.lambda': 0x03BB,
'blank': 0x2423,
'blk12': 0x2592,
'blk14': 0x2591,
'blk34': 0x2593,
'block': 0x2588,
'b.mu': 0x03BC,
'b.nu': 0x03BD,
'b.Omega': 0x03A9,
'b.omega': 0x03CE,
'bottom': 0x22A5,
'bowtie': 0x22C8,
'boxdl': 0x2510,
'boxdL': 0x2555,
'boxDl': 0x2556,
'boxDL': 0x2557,
'boxdr': 0x250C,
'boxdR': 0x2552,
'boxDr': 0x2553,
'boxDR': 0x2554,
'boxh': 0x2500,
'boxH': 0x2550,
'boxhd': 0x252C,
'boxHd': 0x2564,
'boxhD': 0x2565,
'boxHD': 0x2566,
'boxhu': 0x2534,
'boxHu': 0x2567,
'boxhU': 0x2568,
'boxHU': 0x2569,
'boxul': 0x2518,
'boxuL': 0x255B,
'boxUl': 0x255C,
'boxUL': 0x255D,
'boxur': 0x2514,
'boxuR': 0x2558,
'boxUr': 0x2559,
'boxUR': 0x255A,
'boxv': 0x2502,
'boxV': 0x2551,
'boxvh': 0x253C,
'boxvH': 0x256A,
'boxVh': 0x256B,
'boxVH': 0x256C,
'boxvl': 0x2524,
'boxvL': 0x2561,
'boxVl': 0x2562,
'boxVL': 0x2563,
'boxvr': 0x251C,
'boxvR': 0x255E,
'boxVr': 0x255F,
'boxVR': 0x2560,
'b.Phi': 0x03A6,
'b.phis': 0x03C6,
'b.phiv': 0x03D5,
'b.Pi': 0x03A0,
'b.pi': 0x03C0,
'b.piv': 0x03D6,
'bprime': 0x2035,
'b.Psi': 0x03A8,
'b.psi': 0x03C8,
'breve': 0x02D8,
'b.rho': 0x03C1,
'b.rhov': 0x03F1,
'brvbar': 0x00A6,
'b.Sigma': 0x03A3,
'b.sigma': 0x03C3,
'b.sigmav': 0x03C2,
'bsim': 0x223D,
'bsime': 0x22CD,
'bsol': 0x005C,
'b.tau': 0x03C4,
'b.Theta': 0x0398,
'b.thetas': 0x03B8,
'b.thetav': 0x03D1,
'bull': 0x2022,
'bump': 0x224E,
'bumpe': 0x224F,
'b.Upsi': 0x03A5,
'b.upsi': 0x03C5,
'b.Xi': 0x039E,
'b.xi': 0x03BE,
'b.zeta': 0x03B6,
'Cacute': 0x0106,
'cacute': 0x0107,
'Cap': 0x22D2,
'cap': 0x2229,
'caret': 0x2041,
'caron': 0x02C7,
'Ccaron': 0x010C,
'ccaron': 0x010D,
'Ccedil': 0x00C7,
'ccedil': 0x00E7,
'Ccirc': 0x0108,
'ccirc': 0x0109,
'Cdot': 0x010A,
'cdot': 0x010B,
'cedil': 0x00B8,
'cent': 0x00A2,
'CHcy': 0x0427,
'chcy': 0x0447,
'check': 0x2713,
'Chi': 0x03A7,
'chi': 0x03C7,
'cir': 0x25CB,
'circ': 0x02C6,
'cire': 0x2257,
'clubs': 0x2663,
'colon': 0x003A,
'colone': 0x2254,
'comma': 0x002C,
'commat': 0x0040,
'comp': 0x2201,
'compfn': 0x2218,
'cong': 0x2245,
'conint': 0x222E,
'coprod': 0x2210,
'copy': 0x00A9,
'copysr': 0x2117,
'crarr': 0x21B5,
'cross': 0x2717,
'cuepr': 0x22DE,
'cuesc': 0x22DF,
'cularr': 0x21B6,
'Cup': 0x22D3,
'cup': 0x222A,
'cupre': 0x227C,
'curarr': 0x21B7,
'curren': 0x00A4,
'cuvee': 0x22CE,
'cuwed': 0x22CF,
'dagger': 0x2020,
'Dagger': 0x2021,
'daleth': 0x2138,
'dArr': 0x21D3,
'darr': 0x2193,
'darr2': 0x21CA,
'dash': 0x2010,
'dashv': 0x22A3,
'dblac': 0x02DD,
'Dcaron': 0x010E,
'dcaron': 0x010F,
'Dcy': 0x0414,
'dcy': 0x0434,
'deg': 0x00B0,
'Delta': 0x0394,
'delta': 0x03B4,
'Dgr': 0x0394,
'dgr': 0x03B4,
'dharl': 0x21C3,
'dharr': 0x21C2,
'diam': 0x22C4,
'diams': 0x2666,
'die': 0x00A8,
'divide': 0x00F7,
'divonx': 0x22C7,
'DJcy': 0x0402,
'djcy': 0x0452,
'dlarr': 0x2199,
'dlcorn': 0x231E,
'dlcrop': 0x230D,
'dollar': 0x0024,
'dot': 0x02D9,
'Dot': 0x00A8,
'DotDot': 0x20DC,
'drarr': 0x2198,
'drcorn': 0x231F,
'drcrop': 0x230C,
'DScy': 0x0405,
'dscy': 0x0455,
'Dstrok': 0x0110,
'dstrok': 0x0111,
'dtri': 0x25BF,
'dtrif': 0x25BE,
'DZcy': 0x040F,
'dzcy': 0x045F,
'Eacgr': 0x0388,
'eacgr': 0x03AD,
'Eacute': 0x00C9,
'eacute': 0x00E9,
'Ecaron': 0x011A,
'ecaron': 0x011B,
'ecir': 0x2256,
'Ecirc': 0x00CA,
'ecirc': 0x00EA,
'ecolon': 0x2255,
'Ecy': 0x042D,
'ecy': 0x044D,
'eDot': 0x2251,
'Edot': 0x0116,
'edot': 0x0117,
'EEacgr': 0x0389,
'eeacgr': 0x03AE,
'EEgr': 0x0397,
'eegr': 0x03B7,
'efDot': 0x2252,
'Egr': 0x0395,
'egr': 0x03B5,
'Egrave': 0x00C8,
'egrave': 0x00E8,
'egs': 0x22DD,
'ell': 0x2113,
'els': 0x22DC,
'Emacr': 0x0112,
'emacr': 0x0113,
'empty': 0x2205,
'emsp': 0x2003,
'emsp13': 0x2004,
'emsp14': 0x2005,
'ENG': 0x014A,
'eng': 0x014B,
'ensp': 0x2002,
'Eogon': 0x0118,
'eogon': 0x0119,
'epsi': 0x03B5,
'Epsilon': 0x0395,
'epsilon': 0x03B5,
'epsis': 0x220A,
'equals': 0x003D,
'equiv': 0x2261,
'erDot': 0x2253,
'esdot': 0x2250,
'Eta': 0x0397,
'eta': 0x03B7,
'ETH': 0x00D0,
'eth': 0x00F0,
'Euml': 0x00CB,
'euml': 0x00EB,
'excl': 0x0021,
'exist': 0x2203,
'Fcy': 0x0424,
'fcy': 0x0444,
'female': 0x2640,
'ffilig': 0xFB03,
'fflig': 0xFB00,
'ffllig': 0xFB04,
'filig': 0xFB01,
'flat': 0x266D,
'fllig': 0xFB02,
'fnof': 0x0192,
'forall': 0x2200,
'fork': 0x22D4,
'frac12': 0x00BD,
'frac13': 0x2153,
'frac14': 0x00BC,
'frac15': 0x2155,
'frac16': 0x2159,
'frac18': 0x215B,
'frac23': 0x2154,
'frac25': 0x2156,
'frac34': 0x00BE,
'frac35': 0x2157,
'frac38': 0x215C,
'frac45': 0x2158,
'frac56': 0x215A,
'frac58': 0x215D,
'frac78': 0x215E,
'frasl': 0x2044,
'frown': 0x2322,
'gacute': 0x01F5,
'Gamma': 0x0393,
'gamma': 0x03B3,
'gammad': 0x03DC,
'Gbreve': 0x011E,
'gbreve': 0x011F,
'Gcedil': 0x0122,
'gcedil': 0x0123,
'Gcirc': 0x011C,
'gcirc': 0x011D,
'Gcy': 0x0413,
'gcy': 0x0433,
'Gdot': 0x0120,
'gdot': 0x0121,
'gE': 0x2267,
'ge': 0x2265,
'gel': 0x22DB,
'ges': 0x2265,
'Gg': 0x22D9,
'Ggr': 0x0393,
'ggr': 0x03B3,
'gimel': 0x2137,
'GJcy': 0x0403,
'gjcy': 0x0453,
'gl': 0x2277,
'gne': 0x2269,
'gnE': 0x2269,
'gnsim': 0x22E7,
'grave': 0x0060,
'gsdot': 0x22D7,
'gsim': 0x2273,
'Gt': 0x226B,
'gt': 0x003E,
'gvnE': 0x2269,
'hairsp': 0x200A,
'half': 0x00BD,
'hamilt': 0x210B,
'HARDcy': 0x042A,
'hardcy': 0x044A,
'harr': 0x2194,
'hArr': 0x21D4,
'harrw': 0x21AD,
'Hcirc': 0x0124,
'hcirc': 0x0125,
'hearts': 0x2665,
'hellip': 0x2026,
'horbar': 0x2015,
'Hstrok': 0x0126,
'hstrok': 0x0127,
'hybull': 0x2043,
'hyphen': 0x002D,
'Iacgr': 0x038A,
'iacgr': 0x03AF,
'Iacute': 0x00CD,
'iacute': 0x00ED,
'Icirc': 0x00CE,
'icirc': 0x00EE,
'Icy': 0x0418,
'icy': 0x0438,
'idiagr': 0x0390,
'Idigr': 0x03AA,
'idigr': 0x03CA,
'Idot': 0x0130,
'IEcy': 0x0415,
'iecy': 0x0435,
'iexcl': 0x00A1,
'iff': 0x21D4,
'Igr': 0x0399,
'igr': 0x03B9,
'Igrave': 0x00CC,
'igrave': 0x00EC,
'IJlig': 0x0132,
'ijlig': 0x0133,
'Imacr': 0x012A,
'imacr': 0x012B,
'image': 0x2111,
'incare': 0x2105,
'infin': 0x221E,
'inodot': 0x0131,
'inodot': 0x0131,
'int': 0x222B,
'intcal': 0x22BA,
'IOcy': 0x0401,
'iocy': 0x0451,
'Iogon': 0x012E,
'iogon': 0x012F,
'Iota': 0x0399,
'iota': 0x03B9,
'iquest': 0x00BF,
'isin': 0x2208,
'Itilde': 0x0128,
'itilde': 0x0129,
'Iukcy': 0x0406,
'iukcy': 0x0456,
'Iuml': 0x00CF,
'iuml': 0x00EF,
'Jcirc': 0x0134,
'jcirc': 0x0135,
'Jcy': 0x0419,
'jcy': 0x0439,
'Jsercy': 0x0408,
'jsercy': 0x0458,
'Jukcy': 0x0404,
'jukcy': 0x0454,
'Kappa': 0x039A,
'kappa': 0x03BA,
'kappav': 0x03F0,
'Kcedil': 0x0136,
'kcedil': 0x0137,
'Kcy': 0x041A,
'kcy': 0x043A,
'Kgr': 0x039A,
'kgr': 0x03BA,
'kgreen': 0x0138,
'KHcy': 0x0425,
'khcy': 0x0445,
'KHgr': 0x03A7,
'khgr': 0x03C7,
'KJcy': 0x040C,
'kjcy': 0x045C,
'lAarr': 0x21DA,
'Lacute': 0x0139,
'lacute': 0x013A,
'lagran': 0x2112,
'Lambda': 0x039B,
'lambda': 0x03BB,
'lang': 0x2329,
'laquo': 0x00AB,
'Larr': 0x219E,
'larr': 0x2190,
'lArr': 0x21D0,
'larr2': 0x21C7,
'larrhk': 0x21A9,
'larrlp': 0x21AB,
'larrtl': 0x21A2,
'Lcaron': 0x013D,
'lcaron': 0x013E,
'Lcedil': 0x013B,
'lcedil': 0x013C,
'lceil': 0x2308,
'lcub': 0x007B,
'Lcy': 0x041B,
'lcy': 0x043B,
'ldot': 0x22D6,
'ldquo': 0x201C,
'ldquor': 0x201E,
'lE': 0x2266,
'le': 0x2264,
'leg': 0x22DA,
'les': 0x2264,
'lfloor': 0x230A,
'lg': 0x2276,
'Lgr': 0x039B,
'lgr': 0x03BB,
'lhard': 0x21BD,
'lharu': 0x21BC,
'lhblk': 0x2584,
'LJcy': 0x0409,
'ljcy': 0x0459,
'Ll': 0x22D8,
'Lmidot': 0x013F,
'lmidot': 0x0140,
'lnE': 0x2268,
'lne': 0x2268,
'lnsim': 0x22E6,
'lowast': 0x2217,
'lowbar': 0x005F,
'loz': 0x25CA,
'loz': 0x2727,
'lozf': 0x2726,
'lpar': 0x0028,
'lrarr2': 0x21C6,
'lrhar2': 0x21CB,
'lrm': 0x200E,
'lsaquo': 0x2039,
'lsh': 0x21B0,
'lsim': 0x2272,
'lsqb': 0x005B,
'lsquo': 0x2018,
'lsquor': 0x201A,
'Lstrok': 0x0141,
'lstrok': 0x0142,
'Lt': 0x226A,
'lt': 0x003C,
'lthree': 0x22CB,
'ltimes': 0x22C9,
'ltri': 0x25C3,
'ltrie': 0x22B4,
'ltrif': 0x25C2,
'lvnE': 0x2268,
'macr': 0x00AF,
'male': 0x2642,
'malt': 0x2720,
'map': 0x21A6,
'marker': 0x25AE,
'Mcy': 0x041C,
'mcy': 0x043C,
'mdash': 0x2014,
'Mgr': 0x039C,
'mgr': 0x03BC,
'micro': 0x00B5,
'mid': 0x2223,
'middot': 0x00B7,
'minus': 0x2212,
'minusb': 0x229F,
'mldr': 0x2026,
'mnplus': 0x2213,
'models': 0x22A7,
'Mu': 0x039C,
'mu': 0x03BC,
'mumap': 0x22B8,
'nabla': 0x2207,
'Nacute': 0x0143,
'nacute': 0x0144,
'nap': 0x2249,
'napos': 0x0149,
'natur': 0x266E,
'nbsp': 0x00A0,
'Ncaron': 0x0147,
'ncaron': 0x0148,
'Ncedil': 0x0145,
'ncedil': 0x0146,
'ncong': 0x2247,
'Ncy': 0x041D,
'ncy': 0x043D,
'ndash': 0x2013,
'ne': 0x2260,
'nearr': 0x2197,
'nequiv': 0x2262,
'nexist': 0x2204,
'nge': 0x2271,
'nges': 0x2271,
'Ngr': 0x039D,
'ngr': 0x03BD,
'ngt': 0x226F,
'nharr': 0x21AE,
'nhArr': 0x21CE,
'ni': 0x220B,
'NJcy': 0x040A,
'njcy': 0x045A,
'nlarr': 0x219A,
'nlArr': 0x21CD,
'nldr': 0x2025,
'nle': 0x2270,
'nles': 0x2270,
'nlt': 0x226E,
'nltri': 0x22EA,
'nltrie': 0x22EC,
'nmid': 0x2224,
'not': 0x00AC,
'notin': 0x2209,
'npar': 0x2226,
'npr': 0x2280,
'npre': 0x22E0,
'nrarr': 0x219B,
'nrArr': 0x21CF,
'nrtri': 0x22EB,
'nrtrie': 0x22ED,
'nsc': 0x2281,
'nsce': 0x22E1,
'nsim': 0x2241,
'nsime': 0x2244,
'nspar': 0x2226,
'nsub': 0x2284,
'nsubE': 0x2288,
'nsube': 0x2288,
'nsup': 0x2285,
'nsupE': 0x2289,
'nsupe': 0x2289,
'Ntilde': 0x00D1,
'ntilde': 0x00F1,
'Nu': 0x039D,
'nu': 0x03BD,
'num': 0x0023,
'numero': 0x2116,
'numsp': 0x2007,
'nvdash': 0x22AC,
'nvDash': 0x22AD,
'nVdash': 0x22AE,
'nVDash': 0x22AF,
'nwarr': 0x2196,
'Oacgr': 0x038C,
'oacgr': 0x03CC,
'Oacute': 0x00D3,
'oacute': 0x00F3,
'oast': 0x229B,
'ocir': 0x229A,
'Ocirc': 0x00D4,
'ocirc': 0x00F4,
'Ocy': 0x041E,
'ocy': 0x043E,
'odash': 0x229D,
'Odblac': 0x0150,
'odblac': 0x0151,
'odot': 0x2299,
'OElig': 0x0152,
'oelig': 0x0153,
'ogon': 0x02DB,
'Ogr': 0x039F,
'ogr': 0x03BF,
'Ograve': 0x00D2,
'ograve': 0x00F2,
'OHacgr': 0x038F,
'ohacgr': 0x03CE,
'OHgr': 0x03A9,
'ohgr': 0x03C9,
'ohm': 0x2126,
'olarr': 0x21BA,
'oline': 0x203E,
'Omacr': 0x014C,
'omacr': 0x014D,
'Omega': 0x03A9,
'omega': 0x03C9,
'Omicron': 0x039F,
'omicron': 0x03BF,
'ominus': 0x2296,
'oplus': 0x2295,
'or': 0x2228,
'orarr': 0x21BB,
'order': 0x2134,
'ordf': 0x00AA,
'ordm': 0x00BA,
'oS': 0x24C8,
'Oslash': 0x00D8,
'oslash': 0x00F8,
'osol': 0x2298,
'Otilde': 0x00D5,
'otilde': 0x00F5,
'otimes': 0x2297,
'Ouml': 0x00D6,
'ouml': 0x00F6,
'par': 0x2225,
'para': 0x00B6,
'part': 0x2202,
'Pcy': 0x041F,
'pcy': 0x043F,
'percnt': 0x0025,
'period': 0x002E,
'permil': 0x2030,
'perp': 0x22A5,
'Pgr': 0x03A0,
'pgr': 0x03C0,
'PHgr': 0x03A6,
'phgr': 0x03C6,
'phi': 0x03C6,
'Phi': 0x03A6,
'phis': 0x03C6,
'phiv': 0x03D5,
'phmmat': 0x2133,
'phone': 0x260E,
'Pi': 0x03A0,
'pi': 0x03C0,
'piv': 0x03D6,
'planck': 0x210F,
'plus': 0x002B,
'plusb': 0x229E,
'plusdo': 0x2214,
'plusmn': 0x00B1,
'pound': 0x00A3,
'pr': 0x227A,
'pre': 0x227C,
'prime': 0x2032,
'Prime': 0x2033,
'prnsim': 0x22E8,
'prod': 0x220F,
'prop': 0x221D,
'prsim': 0x227E,
'PSgr': 0x03A8,
'psgr': 0x03C8,
'Psi': 0x03A8,
'psi': 0x03C8,
'puncsp': 0x2008,
'quest': 0x003F,
'quot': 0x0022,
'rAarr': 0x21DB,
'Racute': 0x0154,
'racute': 0x0155,
'radic': 0x221A,
'rang': 0x232A,
'raquo': 0x00BB,
'Rarr': 0x21A0,
'rarr': 0x2192,
'rArr': 0x21D2,
'rarr2': 0x21C9,
'rarrhk': 0x21AA,
'rarrlp': 0x21AC,
'rarrtl': 0x21A3,
'rarrw': 0x219D,
'Rcaron': 0x0158,
'rcaron': 0x0159,
'Rcedil': 0x0156,
'rcedil': 0x0157,
'rceil': 0x2309,
'rcub': 0x007D,
'Rcy': 0x0420,
'rcy': 0x0440,
'rdquo': 0x201D,
'rdquor': 0x201C,
'real': 0x211C,
'rect': 0x25AD,
'reg': 0x00AE,
'rfloor': 0x230B,
'Rgr': 0x03A1,
'rgr': 0x03C1,
'rhard': 0x21C1,
'rharu': 0x21C0,
'Rho': 0x03A1,
'rho': 0x03C1,
'rhov': 0x03F1,
'ring': 0x02DA,
'rlarr2': 0x21C4,
'rlhar2': 0x21CC,
'rlm': 0x200F,
'rpar': 0x0029,
'rsaquo': 0x203A,
'rsh': 0x21B1,
'rsqb': 0x005D,
'rsquo': 0x2019,
'rsquor': 0x2018,
'rthree': 0x22CC,
'rtimes': 0x22CA,
'rtri': 0x25B9,
'rtrie': 0x22B5,
'rtrif': 0x25B8,
'rx': 0x211E,
'Sacute': 0x015A,
'sacute': 0x015B,
'samalg': 0x2210,
'sbquo': 0x201A,
'sbsol': 0x005C,
'sc': 0x227B,
'Scaron': 0x0160,
'scaron': 0x0161,
'sccue': 0x227D,
'sce': 0x227D,
'Scedil': 0x015E,
'scedil': 0x015F,
'Scirc': 0x015C,
'scirc': 0x015D,
'scnsim': 0x22E9,
'scsim': 0x227F,
'Scy': 0x0421,
'scy': 0x0441,
'sdot': 0x22C5,
'sdotb': 0x22A1,
'sect': 0x00A7,
'semi': 0x003B,
'setmn': 0x2216,
'sext': 0x2736,
'sfgr': 0x03C2,
'sfrown': 0x2322,
'Sgr': 0x03A3,
'sgr': 0x03C3,
'sharp': 0x266F,
'SHCHcy': 0x0429,
'shchcy': 0x0449,
'SHcy': 0x0428,
'shcy': 0x0448,
'shy': 0x00AD,
'Sigma': 0x03A3,
'sigma': 0x03C3,
'sigmaf': 0x03C2,
'sigmav': 0x03C2,
'sim': 0x223C,
'sime': 0x2243,
'smile': 0x2323,
'SOFTcy': 0x042C,
'softcy': 0x044C,
'sol': 0x002F,
'spades': 0x2660,
'spar': 0x2225,
'sqcap': 0x2293,
'sqcup': 0x2294,
'sqsub': 0x228F,
'sqsube': 0x2291,
'sqsup': 0x2290,
'sqsupe': 0x2292,
'squ': 0x25A1,
'square': 0x25A1,
'squf': 0x25AA,
'ssetmn': 0x2216,
'ssmile': 0x2323,
'sstarf': 0x22C6,
'star': 0x2606,
'starf': 0x2605,
'Sub': 0x22D0,
'sub': 0x2282,
'subE': 0x2286,
'sube': 0x2286,
'subnE': 0x228A,
'subne': 0x228A,
'sum': 0x2211,
'sung': 0x266A,
'Sup': 0x22D1,
'sup': 0x2283,
'sup1': 0x00B9,
'sup2': 0x00B2,
'sup3': 0x00B3,
'supE': 0x2287,
'supe': 0x2287,
'supnE': 0x228B,
'supne': 0x228B,
'szlig': 0x00DF,
'target': 0x2316,
'Tau': 0x03A4,
'tau': 0x03C4,
'Tcaron': 0x0164,
'tcaron': 0x0165,
'Tcedil': 0x0162,
'tcedil': 0x0163,
'Tcy': 0x0422,
'tcy': 0x0442,
'tdot': 0x20DB,
'telrec': 0x2315,
'Tgr': 0x03A4,
'tgr': 0x03C4,
'there4': 0x2234,
'theta': 0x03B8,
'Theta': 0x0398,
'thetas': 0x03B8,
'thetasym': 0x03D1,
'thetav': 0x03D1,
'THgr': 0x0398,
'thgr': 0x03B8,
'thinsp': 0x2009,
'thkap': 0x2248,
'thksim': 0x223C,
'THORN': 0x00DE,
'thorn': 0x00FE,
'tilde': 0x02DC,
'times': 0x00D7,
'timesb': 0x22A0,
'top': 0x22A4,
'tprime': 0x2034,
'trade': 0x2122,
'trie': 0x225C,
'TScy': 0x0426,
'tscy': 0x0446,
'TSHcy': 0x040B,
'tshcy': 0x045B,
'Tstrok': 0x0166,
'tstrok': 0x0167,
'twixt': 0x226C,
'Uacgr': 0x038E,
'uacgr': 0x03CD,
'Uacute': 0x00DA,
'uacute': 0x00FA,
'uArr': 0x21D1,
'uarr': 0x2191,
'uarr2': 0x21C8,
'Ubrcy': 0x040E,
'ubrcy': 0x045E,
'Ubreve': 0x016C,
'ubreve': 0x016D,
'Ucirc': 0x00DB,
'ucirc': 0x00FB,
'Ucy': 0x0423,
'ucy': 0x0443,
'Udblac': 0x0170,
'udblac': 0x0171,
'udiagr': 0x03B0,
'Udigr': 0x03AB,
'udigr': 0x03CB,
'Ugr': 0x03A5,
'ugr': 0x03C5,
'Ugrave': 0x00D9,
'ugrave': 0x00F9,
'uharl': 0x21BF,
'uharr': 0x21BE,
'uhblk': 0x2580,
'ulcorn': 0x231C,
'ulcrop': 0x230F,
'Umacr': 0x016A,
'umacr': 0x016B,
'uml': 0x00A8,
'Uogon': 0x0172,
'uogon': 0x0173,
'uplus': 0x228E,
'Upsi': 0x03A5,
'upsi': 0x03C5,
'upsih': 0x03D2,
'Upsilon': 0x03A5,
'upsilon': 0x03C5,
'urcorn': 0x231D,
'urcrop': 0x230E,
'Uring': 0x016E,
'uring': 0x016F,
'Utilde': 0x0168,
'utilde': 0x0169,
'utri': 0x25B5,
'utrif': 0x25B4,
'Uuml': 0x00DC,
'uuml': 0x00FC,
'varr': 0x2195,
'vArr': 0x21D5,
'Vcy': 0x0412,
'vcy': 0x0432,
'vdash': 0x22A2,
'vDash': 0x22A8,
'Vdash': 0x22A9,
'veebar': 0x22BB,
'vellip': 0x22EE,
'verbar': 0x007C,
'Verbar': 0x2016,
'vltri': 0x22B2,
'vprime': 0x2032,
'vprop': 0x221D,
'vrtri': 0x22B3,
'vsubnE': 0x228A,
'vsubne': 0x228A,
'vsupne': 0x228B,
'vsupnE': 0x228B,
'Vvdash': 0x22AA,
'Wcirc': 0x0174,
'wcirc': 0x0175,
'wedgeq': 0x2259,
'weierp': 0x2118,
'wreath': 0x2240,
'xcirc': 0x25CB,
'xdtri': 0x25BD,
'Xgr': 0x039E,
'xgr': 0x03BE,
'xhArr': 0x2194,
'xharr': 0x2194,
'Xi': 0x039E,
'xi': 0x03BE,
'xlArr': 0x21D0,
'xrArr': 0x21D2,
'xutri': 0x25B3,
'Yacute': 0x00DD,
'yacute': 0x00FD,
'YAcy': 0x042F,
'yacy': 0x044F,
'Ycirc': 0x0176,
'ycirc': 0x0177,
'Ycy': 0x042B,
'ycy': 0x044B,
'yen': 0x00A5,
'YIcy': 0x0407,
'yicy': 0x0457,
'YUcy': 0x042E,
'yucy': 0x044E,
'yuml': 0x00FF,
'Yuml': 0x0178,
'Zacute': 0x0179,
'zacute': 0x017A,
'Zcaron': 0x017D,
'zcaron': 0x017E,
'Zcy': 0x0417,
'zcy': 0x0437,
'Zdot': 0x017B,
'zdot': 0x017C,
'Zeta': 0x0396,
'zeta': 0x03B6,
'Zgr': 0x0396,
'zgr': 0x03B6,
'ZHcy': 0x0416,
'zhcy': 0x0436,
'zwj': 0x200D,
'zwnj': 0x200C,
}

def convert_to_unicode(somestr):
	for gg in iso_unicode.keys():
		try:
			gto="%c" % iso_unicode[gg]
			gcomp="&"+gg+";"
			somestr=somestr.replace(gcomp,gto)
		except:
			pass
	return somestr




## main methods

def runbabel() :
    global with_window
    with_window=1
    mybabel=babelwin()
    mybabel.full_quit=1
    threads_init()
    mainloop()


if __name__== "__main__" :
    runbabel()
