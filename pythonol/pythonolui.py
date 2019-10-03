#! /usr/bin/env python
# -*- coding: ISO-8859-1 -*-

##########################
#  Pythoñol
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

import sys

if __name__== "__main__" :
    # Below is a command line hack to allow windows to use on .exe
    # to run either Pythonol or PyBablePhish-stand-alone
    if str(sys.argv).lower().find("pybabelphish")>-1:
        from pyfilter import *
        from pybabelfish import *
        import random,math,os
        runbabel()
        sys.exit(0)


## NEW FEATURE - CHECK SETUP FIRST and load preferences

import pythonol_setup
pythonol_setup.checkPythonolSetup(0)

from pyfilter import *
import random,math
from pydictionaries import *
import  pytheme,verb_conjugate,verb_families,span2eng,pyquiz,pytrans,pythesaurus
import pythonol_help,irregular_verbs,pythonol_colors,time,pythonol_festival 

if wine_friendly==0: from threading import Thread
from symbolbar import symbolbar
import pybabelfish,pyfilter
from pybabelfish import *
import pyprint


# added in version 2.1, menu icons!

icondict={
	"/ _File/_Quit": STOCK_QUIT,
	"/ _File/_Print...":STOCK_PRINT,
	"/ _Reference Section/_Spanish Slang":STOCK_DIALOG_WARNING,
	"/ _Reference Section/Basic Spanish/_Colors":STOCK_SELECT_COLOR,
	"/ _Reference Section/Basic Spanish/_Alphabets":STOCK_BOLD,
	"/ _Reference Section/Basic Spanish/_Forming Questions":STOCK_DIALOG_QUESTION,
	"/ _Reference Section/_Computer and Internet Terms - F-Q":STOCK_CONVERT,
	"/ _Reference Section/_Computer and Internet Terms - A-E":STOCK_CONVERT,
	"/ _Reference Section/_Computer and Internet Terms - R-Z":STOCK_CONVERT,
	"/_Exercises/_Vocabulary Quiz":STOCK_BOLD,
	"/_Exercises/_Random Vocabulary List":STOCK_SELECT_FONT,
	"/_Exercises/_Translation Exercise":STOCK_FIND_AND_REPLACE,
	"/_Help/_Acknowledgments...":STOCK_DIALOG_INFO, 
	"/_Help/_Frequently Asked Questions...":STOCK_DIALOG_INFO, 
	to_utf8(filter("/_Help/_Making Pythoñol Child-Friendly...")):STOCK_DIALOG_WARNING, 
	"/_Help/_Help...":STOCK_HELP,
	"/_Help/Help _Topics...":STOCK_HELP,
	"/_Help/_License":STOCK_JUSTIFY_FILL,
	to_utf8(filter("/ _Settings/_View Pythoñol's Capabilities...")):STOCK_ZOOM_IN,
        "/ _Settings/_Appearance...":STOCK_SELECT_COLOR,
	"/ _Settings/_Change Translation Service....":STOCK_PREFERENCES, 
	"/ _Settings/Content/_Reload All Data Files":STOCK_REFRESH,
	"/ _Settings/Content/_Enable Mature Content":STOCK_YES,
	"/ _Settings/Content/_Disable Mature Content  [Temporarily]":STOCK_DIALOG_ERROR,
	"/ _Settings/Content/Re-_Install Language Databases...":STOCK_REDO,
	to_utf8(filter("/ _Settings/Content/_Make Pythoñol PERMANENTLY Child-Friendly...")):STOCK_DIALOG_WARNING,
	     }

def setup_menu_images(itemf):
	for ii in icondict.keys():
		try:
			men=itemf.get_widget(ii.replace("_",""))
			meni=Image()
			meni.set_from_stock(icondict[ii],ICON_SIZE_MENU)
			meni.show()
			men.set_image(meni)
		except:
			pass
	 		


class pythonolwin:
    def __init__(self) :
	pythonolwin=Window(WINDOW_TOPLEVEL)
	pythonolwin.set_wmclass(WINDOW_WMC,WINDOW_WMN)
	set_window_icon(pythonolwin,pythonol_icon)
	global MYTIPS
	MYTIPS=TIPS
	self.pversion=PYTHONOL_VERSION
	self.popstatlabel=None
	self.browse_dict={}
	self.browse_list=[]
	self.broad=['a','b','c','d','e','f','g','h','i','j','k','l','n','m','o','p','q','r','s','t','u','v','w','x','y','z','0','1','2','3','4','5','6','7','8','9']+filter_dict.keys()+["É","Ú","Ñ","Ó","Á","Ü","Í"]+unfilter_dict.keys()
	pythonolwin.realize()
	pythonolwin.set_title(to_utf8(filter('Pythoñol ')+self.pversion))
	pythonolwin.set_default_size(GDK.screen_width()-140,GDK.screen_height()-150)
	pythonolwin.set_position(WIN_POS_CENTER)
	self.pythonolwin=pythonolwin
	mainvbox=VBox(0,0)
	self.mainvbox=mainvbox
	pytheme.installTheme(mainvbox)
	pytheme.installTheme(pythonolwin)
	pytheme.setWindowForUpdate(mainvbox)
	pytheme.setWindowForUpdate(pythonolwin)
	mymenubar=MenuBar()
	pytheme.setWindowForUpdate(mymenubar)
	self.mymenubar=mymenubar

	# Fixes for disappearing menubar in PyGtk2
	# save references to AccelGrup and ItemFactory by creating a 
	# 'self' variable, so the MenuBar won't get garbage collected

	# Right now, tearoff menus are causing segmentation faults
	# so they have had to be disabled :-( - probably a bug in PyGtk-2

        ag=AccelGroup()
	self.accel_group=ag
        itemf=ItemFactory(MenuBar, "<main>", ag)
	self.item_factory=itemf

	# Using 'StockItem' below for menu icons
	# instead of 'ImageItem' because ImageItem
	# causes 'gtk warnings' on at least Gtk 2.2.4
	# on Windows, will start all
	# all of these stock icons are changed later
	# default to STOCK_NO for now to eliminate confusion

	menulist =[
            # path              key           callback    action#  type
  ("/ _File",  "<alt>F",  None,  0,"<Branch>"),
  ("/ _File/_Print...", "<control>P", self.openPrintDialog,874,"<StockItem>", STOCK_NO),
  ("/ _File/sep38", None, None, 873, "<Separator>"),
  ("/ _File/_Quit", "<control>Q", self.doQuit,10,"<StockItem>", STOCK_NO),
  ("/ _Settings",  "<alt>S",  None,  5, "<Branch>"),
  # ('/ _Settings/tearoff22', None, None, 3503, '<Tearoff>'),
  ("/ _Settings/Content",  None,  None, 479, "<Branch>"),
  #('/ _Settings/Content/tearoff22', None, None, 3503, '<Tearoff>'),
  ("/ _Settings/Content/_Reload All Data Files", None, self.loadDictionariesGo, 6, "<StockItem>", STOCK_NO),
  ("/ _Settings/Content/sep939", None, None, 4788, "<Separator>"),
  ("/ _Settings/Content/_Disable Mature Content  [Temporarily]", None, self.makeChildSafe, 7, "<StockItem>", STOCK_NO),
  ("/ _Settings/Content/_Enable Mature Content", None, self.loadDictionariesGo2, 8, "<StockItem>", STOCK_NO),

  ("/ _Settings/Content/sep936", None, None, 4788, "<Separator>"),
  ("/ _Settings/Content/Re-_Install Language Databases...", None, self.reinstallDB, 4787, "<StockItem>", STOCK_NO),
  ("/ _Settings/Content/sep937", None, None, 4789, "<Separator>"),
  (to_utf8(filter("/ _Settings/Content/_Make Pythoñol PERMANENTLY Child-Friendly...")), None, self.makePermChildSafe, 4786, "<StockItem>", STOCK_NO),

  ("/ _Settings/_Appearance...","<alt>A",pytheme.showThemeSelector,904,"<StockItem>", STOCK_SELECT_COLOR),
  ("/ _Settings/_Change Translation Service....", None, self.askTransService, 472, "<StockItem>", STOCK_NO),
  ("/ _Settings/sep3", None, None, 1, "<Separator>"),
  (to_utf8(filter("/ _Settings/_View Pythoñol's Capabilities...")), None, self.showCapabilities, 2, "<StockItem>", STOCK_NO),
  ("/ _Reference Section",  "<alt>R",  None,  30, "<Branch>"),

  ("/ _Reference Section/Basic Spanish",  None,  None,  311, "<Branch>"),
  #("/ _Reference Section/Basic Spanish/tearoff27", None, None, 3543, '<Tearoff>'),
  ("/ _Reference Section/Basic Spanish/Acce_nts", None, self.openHelp, 202, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Basic Spanish/_Alphabets", None, self.openHelp, 200, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Basic Spanish/A_rticles", None, self.openHelp, 203, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Basic Spanish/_Colors", None, pythonol_colors.palettewin, 540, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Basic Spanish/_Days of the Week", None, self.openHelp, 280, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Basic Spanish/The _Weather", None, self.openHelp, 517, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Basic Spanish/_Forming Questions", None, self.openHelp, 278, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Basic Spanish/Numbers",  None,  None,  31, "<Branch>"),
  #("/ _Reference Section/Basic Spanish/Numbers/tearoff22", None, None, 3503, '<Tearoff>'),
  ("/ _Reference Section/Basic Spanish/Numbers/_Cardinal Numbers", None, self.openHelp, 32, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Basic Spanish/Numbers/_Ordinal Numbers", None, self.openHelp, 33,"<StockItem>", STOCK_NO),

  #("/ _Reference Section/Comparing People and Things/tearoff22", None, None, 3503, '<Tearoff>'),
  ("/ _Reference Section/Comparing People and Things/_Comparisons", None, self.openHelp, 205, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Comparing People and Things/_Superlatives", None, self.openHelp, 277, "<StockItem>", STOCK_NO),


  ("/ _Reference Section/Pronouns",  None,  None,  34, "<Branch>"),
  #('/ _Reference Section/Pronouns/tearoff22', None, None, 3503, '<Tearoff>'),
  ("/ _Reference Section/Pronouns/_Extended Pronoun Help", None, self.openHelp, 35, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Pronouns/_Subject Pronouns", None, self.openHelp, 36, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Pronouns/_Direct Object Pronouns", None, self.openHelp, 37, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Pronouns/_Indirect Object Pronouns", None, self.openHelp, 38, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Pronouns/_Prepositional Object Pronouns", None, self.openHelp, 39, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Pronouns/Demo_nstrative Pronouns", None, self.openHelp, 40, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Pronouns/I_nterrogative Pronouns", None, self.openHelp, 41, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Pronouns/_Reflexive vs. Reciprocal Pronouns", None, self.openHelp, 42, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Pronouns/_Miscellaneous Pronouns", None, self.openHelp, 43,"<StockItem>", STOCK_NO),
  ("/ _Reference Section/Pronouns/P_ronoun Placement", None, self.openHelp, 201,"<StockItem>", STOCK_NO),

  #('/ _Reference Section/Verbs/tearoff22', None, None, 3503, '<Tearoff>'),
  ("/ _Reference Section/Verbs/_Acaber", None, self.openHelp, 270, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Verbs/_Buscar", None, self.openHelp, 271, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Verbs/_Caber", None, self.openHelp, 272, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Verbs/_Dar", None, self.openHelp, 273, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Verbs/_Estar vs. Ser", None, self.openHelp, 212, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Verbs/_Gustar", None, self.openHelp, 274, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Verbs/_Nacer", None, self.openHelp, 275, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Verbs/_Pensar", None, self.openHelp, 276, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Verbs/_Saber vs. Conocer", None, self.openHelp, 213, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Verbs/_Tener", None, self.openHelp, 214, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Verbs/sep87", None, None, 947, "<Separator>"),
  ("/ _Reference Section/Verbs/_Linked Verbs", None, self.openHelp, 279, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Verbs/_Verbs Like Gustar", None, self.openHelp, 215, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Verbs/_Useful Verb Constructions", None, self.openHelp, 649, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Verbs/Different Tenses - Different _Meanings", None, self.openHelp, 211,"<StockItem>", STOCK_NO),

  #('/ _Reference Section/Verb Tenses/tearoff22', None, None, 3503, '<Tearoff>'),
  ("/ _Reference Section/Verb Tenses/The Subjunctive: In Detail", None, self.openHelp, 239, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Verb Tenses/sep2", None, None, 600, "<Separator>"),
  #('/ _Reference Section/Verb Tenses/Present Tense/tearoff22', None, None, 3503, '<Tearoff>'),
  ("/ _Reference Section/Verb Tenses/Present Tense/Present", None, self.openHelp, 216, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Verb Tenses/Present Tense/Present Progressive", None, self.openHelp, 217,"<StockItem>", STOCK_NO),
  ("/ _Reference Section/Verb Tenses/Present Tense/Present Subjunctive", None, self.openHelp, 218, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Verb Tenses/Present Tense/Present Perfect", None, self.openHelp, 219, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Verb Tenses/Present Tense/Present Perfect Subjunctive", None, self.openHelp, 220, "<StockItem>", STOCK_NO),
  #('/ _Reference Section/Verb Tenses/Past Tense/tearoff22', None, None, 3503, '<Tearoff>'),
  ("/ _Reference Section/Verb Tenses/Past Tense/Past (Preterit)", None, self.openHelp, 221, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Verb Tenses/Past Tense/Past Progressive (Imperfect)", None, self.openHelp, 222, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Verb Tenses/Past Tense/Imperfect Progressive", None, self.openHelp, 223, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Verb Tenses/Past Tense/Past Subjunctive", None, self.openHelp, 224, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Verb Tenses/Past Tense/Past Perfect", None, self.openHelp, 225, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Verb Tenses/Past Tense/Past Perfect Subjunctive", None, self.openHelp, 226, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Verb Tenses/Past Tense/Preterit Perfect", None, self.openHelp, 227, "<StockItem>", STOCK_NO),
  #('/ _Reference Section/Verb Tenses/Future Tense/tearoff22', None, None, 3503, '<Tearoff>'),
  ("/ _Reference Section/Verb Tenses/Future Tense/Future", None, self.openHelp, 228, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Verb Tenses/Future Tense/Future Popular", None, self.openHelp, 229, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Verb Tenses/Future Tense/Future Progressive", None, self.openHelp, 230, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Verb Tenses/Future Tense/Future Subjunctive", None, self.openHelp, 231, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Verb Tenses/Future Tense/Future Perfect", None, self.openHelp, 232, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Verb Tenses/Future Tense/Future Perfect Progressive", None, self.openHelp, 233, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Verb Tenses/Future Tense/Future Perfect Subjunctive", None, self.openHelp, 234,"<StockItem>", STOCK_NO),
  #('/ _Reference Section/Verb Tenses/Conditional Tense/tearoff22', None, None, 3503, '<Tearoff>'),
  ("/ _Reference Section/Verb Tenses/Conditional Tense/Conditional", None, self.openHelp, 235, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Verb Tenses/Conditional Tense/Conditional Perfect", None, self.openHelp, 236, "<StockItem>", STOCK_NO),
  #('/ _Reference Section/Verb Tenses/Commands/tearoff22', None, None, 3503, '<Tearoff>'),
  ("/ _Reference Section/Verb Tenses/Commands/Imperative (Command)", None, self.openHelp, 237,"<StockItem>", STOCK_NO),
  ("/ _Reference Section/Verb Tenses/Commands/Negative Imperative (Command)", None, self.openHelp, 238, "<StockItem>", STOCK_NO),

  ("/ _Reference Section/_Duration Of Time", None, self.openHelp, 207, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Pass_ive Voice", None, self.openHelp, 206, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/P_ersonal 'A'", None, self.openHelp, 204, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/Por _vs. Para", None, self.openHelp, 45, "<StockItem>", STOCK_NO),
  ("/ _Reference Section/_Prepositions", None, self.openHelp, 44, "<StockItem>", STOCK_NO), 
  ("/ _Reference Section/sep1477", None, None, 4776, "<Separator>"),
  ("/ _Reference Section/_Computer and Internet Terms - A-E", None, self.openHelpRC, 587, "<StockItem>", STOCK_NO), 
  ("/ _Reference Section/_Computer and Internet Terms - F-Q", None, self.openHelpRC, 588, "<StockItem>", STOCK_NO), 
  ("/ _Reference Section/_Computer and Internet Terms - R-Z", None, self.openHelpRC, 589, "<StockItem>", STOCK_NO), 
  ("/ _Reference Section/sep1677", None, None, 4776, "<Separator>"),
  ("/ _Reference Section/_Spanish Slang", None, self.showSlangHelp, 46, "<StockItem>", STOCK_NO),

  ("/_Exercises",  "<alt>E",  None,70, "<Branch>"), 
  #('/_Exercises/tearoff22', None, None, 3503, '<Tearoff>'),
  ("/_Exercises/_Random Vocabulary List", None, self.doVocabList, 711, "<StockItem>", STOCK_NO),
  ("/_Exercises/_Vocabulary Quiz", None, self.doVocabQuiz, 71, "<StockItem>", STOCK_NO),
  ("/_Exercises/_Translation Exercise", None, self.doTransQuiz, 72, "<StockItem>", STOCK_NO),
  ("/_Exercises/Reading Comprehension",  None,  None,  73, "<Branch>"),
  #('/_Exercises/Reading Comprehension/tearoff25', None, None, 3502, '<Tearoff>'),
  ("/_Exercises/Reading Comprehension/Exercise #1", None, self.openHelpRC, 74, "<StockItem>", STOCK_NO),
  ("/_Exercises/Reading Comprehension/Exercise #2", None, self.openHelpRC, 75, "<StockItem>", STOCK_NO),
  ("/_Exercises/Reading Comprehension/Exercise #3", None, self.openHelpRC, 76, "<StockItem>", STOCK_NO),
  ("/_Exercises/Reading Comprehension/Exercise #4", None, self.openHelpRC, 77, "<StockItem>", STOCK_NO),
  ("/_Exercises/Reading Comprehension/Exercise #5", None, self.openHelpRC, 78, "<StockItem>", STOCK_NO),
  ("/_Exercises/Reading Comprehension/Exercise #6", None, self.openHelpRC, 79, "<StockItem>", STOCK_NO),
  ("/_Exercises/Reading Comprehension/Exercise #7", None, self.openHelpRC, 80, "<StockItem>", STOCK_NO),
  ("/_Exercises/Reading Comprehension/Exercise #8", None, self.openHelpRC, 81, "<StockItem>", STOCK_NO),
  ("/_Exercises/Reading Comprehension/Exercise #9", None, self.openHelpRC, 82, "<StockItem>", STOCK_NO),
  ("/_Exercises/Reading Comprehension/Exercise #10", None, self.openHelpRC, 83, "<StockItem>", STOCK_NO),
  ("/_Exercises/Reading Comprehension/Exercise #11", None, self.openHelpRC, 84, "<StockItem>", STOCK_NO),
  ("/_Exercises/Reading Comprehension/Exercise #12", None, self.openHelpRC, 85, "<StockItem>", STOCK_NO),
  ("/_Exercises/Reading Comprehension/Exercise #13", None, self.openHelpRC, 86, "<StockItem>", STOCK_NO),
  ("/_Exercises/Reading Comprehension/Exercise #14", None, self.openHelpRC, 87, "<StockItem>", STOCK_NO),
  ("/_Exercises/Reading Comprehension/Exercise #15", None, self.openHelpRC, 88, "<StockItem>", STOCK_NO),
  ("/_Exercises/Reading Comprehension/Exercise #16", None, self.openHelpRC, 89, "<StockItem>", STOCK_NO),
  ("/_Exercises/Reading Comprehension/Exercise #17", None, self.openHelpRC, 90, "<StockItem>", STOCK_NO),
  ("/_Exercises/Reading Comprehension/Exercise #18", None, self.openHelpRC, 91, "<StockItem>", STOCK_NO),
  ("/_Exercises/Reading Comprehension/Exercise #19", None, self.openHelpRC, 92, "<StockItem>", STOCK_NO),
  ("/_Exercises/Reading Comprehension/Exercise #20", None, self.openHelpRC, 93, "<StockItem>", STOCK_NO),
  ("/_Exercises/Reading Comprehension/Exercise #21", None, self.openHelpRC, 94, "<StockItem>", STOCK_NO),
  ("/_Exercises/Reading Comprehension/Exercise #22", None, self.openHelpRC, 95, "<StockItem>", STOCK_NO),
  ("/_Exercises/Reading Comprehension/Exercise #23", None, self.openHelpRC, 96, "<StockItem>", STOCK_NO),
  ("/_Help",  "<alt>H",  None,16, "<LastBranch>"), 
  #('/_Help/tearoff2', None, None, 3501, '<Tearoff>'),
  ("/_Help/_License", "F2", self.openHelp, 851, "<StockItem>", STOCK_NO),
  ("/_Help/sep55", None, None, 568, "<Separator>"),
  ("/_Help/Help _Topics...", "F4", self.openHelpTopics, 567, "<StockItem>", STOCK_NO),
  ("/_Help/_Help...", "F1", self.openHelp, 21, "<StockItem>", STOCK_NO),
  ("/_Help/sep57", None, None, 571, "<Separator>"),
  (to_utf8(filter("/_Help/_Making Pythoñol Child-Friendly...")), "F3", self.openHelp, 22,"<StockItem>", STOCK_NO),
  ("/_Help/sep58", None, None, 569, "<Separator>"),
  ("/_Help/_Conjugate Help...", "F5", self.openHelp, 23, "<StockItem>", STOCK_NO),
  ("/_Help/T_ranslate Help...", "F6", self.openHelp, 268, "<StockItem>", STOCK_NO),
  ("/_Help/_Dictionary Help...", "F7", self.openHelp, 24, "<StockItem>", STOCK_NO),
  ("/_Help/_Thesaurus Help...", "F8", self.openHelp, 26, "<StockItem>", STOCK_NO),
  ("/_Help/_Idioms Help...", "F9", self.openHelp, 25,"<StockItem>", STOCK_NO),
  ("/_Help/_Pronounce Help...",None, self.openHelp, 27, "<StockItem>", STOCK_NO),
  ("/_Help/_Verb Browser Help...", "F10", self.openHelp, 266, "<StockItem>", STOCK_NO),
  ("/_Help/Verb _Families Help...", "F11", self.openHelp, 267, "<StockItem>", STOCK_NO),
  ("/_Help/sep1030", None,None, 1267, "<Separator>"),
  ("/_Help/Festival Text-to-_Speech Help...", None, self.openHelp, 847,"<StockItem>", STOCK_NO),
  ("/_Help/sep1030", None,None, 1267, "<Separator>"),
  ("/_Help/_Frequently Asked Questions...", None, self.openHelp, 848, "<StockItem>", STOCK_NO),
  ("/_Help/sep1030", None,None, 1267, "<Separator>"),
  ("/_Help/_Acknowledgments...", None, self.openHelp, 849, "<StockItem>", STOCK_NO),

        ]


	self.menulist=menulist
        itemf.create_items(menulist)

	for yy in menulist:  # some default menu icons
		if yy[4]=="<StockItem>":
			if not icondict.has_key(yy[0]):
				if yy[0].find("eference Sec")>-1:
					icondict[yy[0]]=STOCK_DIALOG_INFO
				elif yy[0].find("Reading Compre")>-1:
					icondict[yy[0]]=STOCK_JUSTIFY_FILL
				elif yy[0].find(" Help...")>-1:
					icondict[yy[0]]=STOCK_DIALOG_QUESTION
				else: 
					icondict[yy[0]]=STOCK_NEW

	setup_menu_images(itemf)

        pythonolwin.add_accel_group(ag)
        mymenubar=self.item_factory.get_widget("<main>")
        mymenubar.show()

	# do what we can to be memory friendly...
	icondict.clear()
	menulist=[]
	self.menulist=[]

	mainvbox.pack_start(mymenubar,0,0,0)
	label1=getImage(getPixDir()+"title.gif",to_utf8(filter('Pythoñol ')+self.pversion ))
	mainvbox.pack_start(label1,0,0,3)
	self.print_mon=0
	mynotebook=Notebook()
	mynotebook.set_scrollable(1)
	mynotebook.set_border_width(3)
	self.mynotebook=mynotebook
        if use_filter==1: # make a scroll bar for the notebook on windows
            self.addScroller()
	conjbox=VBox(0,0)
	self.conjbox=conjbox
	conjtable=Table(2,3,0)
	conjtable.set_row_spacings(3)
	conjtable.set_col_spacings(10)
	conjtable.set_border_width(5)
	self.conjtable=conjtable
	label10=Label('Verb:')
	label10.set_alignment(0,0.5)
	self.label10=label10
	conjtable.attach(label10,0,1,1,2,(FILL),(0),0,0)
	label12=getIconButton (pythonolwin,STOCK_SAVE,"Save To File",1) 
	label12.connect("clicked",self.saveConj)
	self.label12=label12
	conjtable.attach(label12,0,1,0,1,(FILL),(0),0,0)
	ondemandbox=HBox(0,0)
	ondemandbox.set_spacing(3)
	ondemandcheck=CheckButton('Search-On-Demand')
	self.ondemandcheck=ondemandcheck
	ondemandcheck.set_active(1)
	ondemandbox.pack_start(ondemandcheck,1,1,0)
	verbfambutt=getIconButton (pythonolwin,STOCK_DIALOG_INFO,"Verb Family",1) 
	verbfambutt.connect("clicked",self.showVerbFamily)
	ondemandbox.pack_start(verbfambutt,0,0,0)
	conjtable.attach(ondemandbox,1,2,0,1,(FILL),(0),0,0)
	conjhelpbutt=getIconButton (pythonolwin,STOCK_DIALOG_QUESTION,"Help",1) 
	conjhelpbutt.connect("clicked",self.showConjugateHelp)
	self.conjhelpbutt=conjhelpbutt
	conjtable.attach(conjhelpbutt,2,3,0,1,(FILL),(0),0,0)
	conjbutt=getIconButton (pythonolwin,STOCK_OK,'CONJUGATE!',1) 
	conjbutt.connect("clicked",self.conjugateHandler)
	self.conjbutt=conjbutt
	conjtable.attach(conjbutt,2,3,1,2,(FILL),(0),0,0)
	conjcombo=Combo()
	conjcombo.set_case_sensitive(1)
	self.conjcombo=conjcombo
	conj_entry=conjcombo.entry
	self.conj_entry=conj_entry
	self.conj_history=[]
	conjtable.attach(conjcombo,1,2,1,2,(EXPAND+FILL),(0),0,0)
	conjbox.pack_start(conjtable,0,0,0)
	conjbox.pack_start(symbolbar(conj_entry,pythonolwin),0,0,0)
	conjscroll=ScrolledWindow()
	self.conjscroll=conjscroll
	conjtext=TextView()
	conjtext.set_editable(1)
	self.conjtext=conjtext
	conjscroll.add(conjtext)
	conjtext.set_wrap_mode(WRAP_WORD)

	conjscrollbox=HBox(0,0)
	conjscrollbox.pack_start(pythonol_festival.FestivalBar(pythonolwin,conjtext,2,1),0,0,0)
	conjscrollbox.pack_start(conjscroll,1,1,0)
	conjbox.pack_start(conjscrollbox,1,1,0)

	conjstat=Label(to_utf8(filter("Pythoñol version ")+self.pversion+" initializing..."))
  	if use_filter==1: conjstat=Label(to_utf8(filter("Pythoñol version ")+self.pversion+" ready.") ) 
	self.conjstat=conjstat
	conjbox.pack_start(conjstat,0,0,2)
	mynotebook.append_page(conjbox, getIconButton (pythonolwin,STOCK_CONVERT,"Conjugate",1,1)  )

	# PyBabelFish
	babwin=babelwin(pythonolwin,self.runBabHelp)
	self.babwin=babwin
	mynotebook.append_page(babwin.mainvbox,getIconButton (pythonolwin,STOCK_REFRESH,"Translate",1,1) )

	# Dictionary
	dictbox=VBox(0,0)
	self.dictbox=dictbox
	dicthbox=HBox(0,0)
	dicthbox.set_spacing(8)
	self.dicthbox=dicthbox
	dicthbox.pack_start(Label(" "),1,1,0)
	dictradio1=RadioButton(None,'English to Spanish')
	self.dictradio1=dictradio1
	dicthbox.pack_start(dictradio1,0,0,0)
	dictradio2=RadioButton(self.dictradio1,'Spanish To English')
	self.dictradio2=dictradio2
	dicthbox.pack_start(dictradio2,0,0,0)
	dictradio3=RadioButton(self.dictradio1,'Both')
	self.dictradio3=dictradio3
	dicthbox.pack_start(dictradio3,0,0,0)
	dictbox.pack_start(dicthbox,0,0,0)
	dicttable=Table(2,3,0)
	dicttable.set_row_spacings(4)
	dicttable.set_col_spacings(9)
	self.dicttable=dicttable
	dicttable.attach(Label("  "),0,1,0,1,(FILL),(0),0,0)
	exactbox=HBox(0,0)
	exactbox.set_spacing(2)
	exactbutt=CheckButton('Extended search')
	self.exactbutt=exactbutt
	exactbutt.set_active(1)
	exactbox.pack_start(exactbutt,1,1,0)
	label73=getIconButton (pythonolwin,STOCK_SAVE,"Save To File",1) 
	label73.connect("clicked",self.saveDict)
	exactbox.pack_start(label73,1,1,0)	
	dicttable.attach(exactbox,1,2,0,1,(FILL),(0),0,0)
	dicthelpbutt=getIconButton (pythonolwin,STOCK_DIALOG_QUESTION,"Help",1) 
	dicthelpbutt.connect("clicked",self.showDictHelp)
	self.dicthelpbutt=dicthelpbutt
	dicttable.attach(dicthelpbutt,2,3,0,1,(FILL),(0),0,0)
	dictlookupbutt=getIconButton (pythonolwin,STOCK_ZOOM_IN,'LOOKUP!',1)  
	dictlookupbutt.connect("clicked",self.getDictionaryDefinition)
	self.dictlookupbutt=dictlookupbutt
	dicttable.attach(dictlookupbutt,2,3,1,2,(FILL),(0),0,0)
	label14=Label(' Word / Phrase: ')
	label14.set_alignment(0,0.5)
	self.label14=label14
	dicttable.attach(label14,0,1,1,2,(FILL),(0),0,0)
	dictcombo=Combo()
	dictcombo.set_case_sensitive(1)
	self.dict_history=[]
	self.dictcombo=dictcombo
	dict_entry=dictcombo.entry
	self.dict_entry=dict_entry
	dicttable.attach(dictcombo,1,2,1,2,(EXPAND+FILL),(0),0,0)
	dictbox.pack_start(dicttable,0,0,4)
	dictbox.pack_start(symbolbar(dict_entry,pythonolwin),0,0,0)	
	dictscroll=ScrolledWindow()
	self.dictscroll=dictscroll
	dicttext=TextView()
	dicttext.set_double_buffered(1)
	dicttext.set_editable(1)
	dicttext.set_wrap_mode(WRAP_WORD)
	self.dicttext=dicttext
	dictscroll.add(dicttext)
	dictscrollbox=HBox(0,0)
	dictscrollbox.pack_start(pythonol_festival.FestivalBar(pythonolwin,dicttext,2,1),0,0,0)
	dictscrollbox.pack_start(dictscroll,1,1,0)
	dictbox.pack_start(dictscrollbox,1,1,0)
	mynotebook.append_page(dictbox,getIconButton (pythonolwin,STOCK_BOLD,"Dictionary",1,1) )

	# Thesaurus
	thesbox=VBox(0,0)
	thesbox.set_spacing(4)
	theshbox=HBox(0,0)
	theshbox.set_spacing(8)
	theshbox.set_border_width(2)
	label78=getIconButton (pythonolwin,STOCK_SAVE,"Save To File",1) 
	label78.set_border_width(4)
	label78.connect("clicked",self.saveThes)
	theshbox.pack_start(label78,0,0,0)	
	theshelpbutt=getIconButton (pythonolwin,STOCK_DIALOG_QUESTION,"Help  ",1) 
	theshelpbutt.connect("clicked",self.showThesHelp)
	theshelpbutt.set_border_width(4)
	theshbox.pack_start(theshelpbutt,0,0,0)	
	theshbox.pack_start(Label(" "),1,1,0)
	thesradio1=RadioButton(None,'Spanish')
	self.thesradio1=thesradio1
	theshbox.pack_start(thesradio1,0,0,0)
	thesradio2=RadioButton(self.thesradio1,'English')
	self.thesradio2=thesradio2
	theshbox.pack_start(thesradio2,0,0,0)
	thesbox.pack_start(theshbox,0,0,0)
	theshbox2=HBox(0,0)
	theshbox2.set_spacing(8)	
	label74=Label(' Word / Phrase: ')
	label74.set_alignment(0,0.5)
	theshbox2.pack_start(label74,0,0,0)
	thescombo=Combo()
	thescombo.set_case_sensitive(1)
	self.thescombo=thescombo
	self.thes_history=[]
	self.thescombo=thescombo
	thes_entry=thescombo.entry
	self.thes_entry=thes_entry
	theshbox2.pack_start(thescombo,1,1,0)
	theslookupbutt=getIconButton (pythonolwin,STOCK_ZOOM_IN,'Find Synonyms!',1)  
	theslookupbutt.connect("clicked",self.getThesaurusMatches)
	theshbox2.pack_start(theslookupbutt,0,0,0)
	thesbox.pack_start(theshbox2,0,0,0)
	thesbox.pack_start(symbolbar(thes_entry,pythonolwin),0,0,0)
	thesscroll=ScrolledWindow()
	self.thesscroll=thesscroll
	thestext=TextView()
	thestext.set_double_buffered(1)
	thestext.set_editable(1)
	thestext.set_wrap_mode(WRAP_WORD)
	self.thestext=thestext
	thesscroll.add(thestext)

	thesscrollbox=HBox(0,0)
	thesscrollbox.pack_start(pythonol_festival.FestivalBar(pythonolwin,thestext,2,1),0,0,0)
	thesscrollbox.pack_start(thesscroll,1,1,0)
	thesbox.pack_start(thesscrollbox,1,1,0)
	mynotebook.append_page(thesbox,getIconButton (pythonolwin,STOCK_SELECT_COLOR,"Thesaurus",1,1) )

	#Idioms
	idbox=VBox(0,0)
	self.idbox=idbox
	label15=Label('Idioms: Common phrases, nouns, and expressions')
	self.label15=label15
	idbox.pack_start(label15,0,0,3)
	idtable=Table(2,3,0)
	idtable.set_row_spacings(4)
	idtable.set_col_spacings(7)
	self.idtable=idtable
	idlisteng=getIconButton (pythonolwin,STOCK_REFRESH,'List English Idioms',1) 
	idlisteng.connect("clicked",self.listIdiomsEng)
	self.idlisteng=idlisteng
	idtable.attach(idlisteng,0,1,0,1,(FILL),(0),0,0)
	idlistspan=getIconButton (pythonolwin,STOCK_REFRESH,'List Spanish Idioms',1) 
	idlistspan.connect("clicked",self.listIdiomsSpa)
	self.idlistspan=idlistspan
	idtable.attach(idlistspan,2,3,0,1,(FILL),(0),0,0)
	idsbox=HBox(0,0)
	idsbox.set_spacing(3)
	label16=getIconButton (pythonolwin,STOCK_SAVE,"Save To File",1) 
	label16.connect("clicked",self.saveId)
	self.label16=label16
	idsbox.pack_start(label16,1,1,0)
	label96=getIconButton (pythonolwin,STOCK_DIALOG_QUESTION,"Help",1) 
	label96.connect("clicked",self.showIdiomHelp)
	self.label96=label96
	idsbox.pack_start(label96,1,1,0)
	idtable.attach(idsbox,1,2,0,1,(FILL),(0),0,0)
	label17=Label(' '+'Keyword / Phrase:')
	label17.set_alignment(0,0.5)
	self.label17=label17
	idtable.attach(label17,0,1,1,2,(FILL),(0),0,0)
	idsearchbutt=getIconButton (pythonolwin,STOCK_ZOOM_IN,'Search',1)
	idsearchbutt.connect("clicked",self.searchIdioms)
	self.idsearchbutt=idsearchbutt
	idtable.attach(idsearchbutt,2,3,1,2,(FILL),(0),0,0)
	idcombo=Combo()
	idcombo.set_case_sensitive(1)
	self.idcombo=idcombo
	self.id_history=[]
	id_entry=idcombo.entry
	self.id_entry=id_entry
	idtable.attach(idcombo,1,2,1,2,(EXPAND+FILL),(0),0,0)
	idbox.pack_start(idtable,0,0,5)
	idbox.pack_start(symbolbar(id_entry,pythonolwin),0,0,0)
	idscroll=ScrolledWindow()
	self.idscroll=idscroll
	idtext=TextView()
	idtext.set_double_buffered(1)
	idtext.set_wrap_mode(WRAP_WORD)
	idtext.set_editable(1)
	self.idtext=idtext
	idscroll.add(idtext)
	idscrollbox=HBox(0,0)
	idscrollbox.pack_start(pythonol_festival.FestivalBar(pythonolwin,idtext,2,1),0,0,0)
	idscrollbox.pack_start(idscroll,1,1,0)
	idbox.pack_start(idscrollbox,1,1,0)


	#idiom buttons
	idiombuttbox=HBox(0,0)
	idiombuttbox.set_homogeneous(0)
	idiombuttbox.set_spacing(6)
	idiombuttbox.set_border_width(4)
	browse11=getIconButton (pythonolwin,STOCK_OK,"Show Page",1) 
	self.browse11=browse11
	browse11.connect("clicked",self.listIdiomsCB)
	idiombuttbox.pack_start(Label("View Page:"),0,0,0)
	self.idiompagecombo=Combo()
	self.idiompagecombo.set_popdown_strings([' '])
	self.idiompageentry=self.idiompagecombo.entry
	self.idiompageentry.set_editable(0)
	idiombuttbox.pack_start(self.idiompagecombo,0,0,0)
	idiombuttbox.pack_start(browse11,0,0,0)
	idiombuttbox.pack_start(Label(" "),1,1,0)
	idbox.pack_start(idiombuttbox,0,0,0)
	mynotebook.append_page(idbox,getIconButton (pythonolwin,STOCK_YES,"Idioms",1,1) )

	#Pronounce tab
	probox=VBox(0,0)
	probox.set_spacing(3)
	probox.set_border_width(2)
        if use_filter==1: probox.pack_start(Label(to_utf8("THIS FEATURE IS NOT AVAILABLE ON WINDOWS!\n")),0,0,0)
        
	probox.pack_start(Label(to_utf8("PRONOUNCE\n\nHear words, phrases, and sentences read aloud in Spanish.\nType some text below, paste in some text (up to 10 kilobytes), or\ndrag a text file onto the box.  Then click the 'Play' \nbutton to your left.\n")),0,0,4)
	probb=HBox(0,0)
	probb.pack_start(Label(""),0,0,3)
	probox.pack_start(probb,0,0,2)
	proscroll=ScrolledWindow()
	protext=TextView()
	#protext.set_double_buffered(1)
	protext.set_wrap_mode(WRAP_WORD)
	protext.set_editable(1)
	self.protext=protext
	proscroll.add(protext)
	addDragSupport(protext,setDrag)
	probb.pack_start(symbolbar(protext,pythonolwin,2),1,0,0)
	prohelpbutt=getIconButton (pythonolwin,STOCK_DIALOG_QUESTION,"Help",1) 
	prohelpbutt.connect("clicked",self.showPronounceHelp)
	probb.pack_start(prohelpbutt,1,0,0)
	proscrollbox=HBox(0,0)
	proscrollbox.pack_start(pythonol_festival.FestivalBar(pythonolwin,protext,2),0,0,0)
	proscrollbox.pack_start(proscroll,1,1,0)
	probox.pack_start(proscrollbox,1,1,0)
        if use_filter==1:
            probox.set_sensitive(0) # disable on windows
            setTip(probox,to_utf8("This feature is disabled on Windows"))
	mynotebook.append_page(probox,getIconButton (pythonolwin,STOCK_CDROM,"Pronounce",1,1) )

	# BrowseVerbs
	browseverbbox=VBox(0,0)
	self.browseverbbox=browseverbbox
	label96=Label(' '+'Browse Spanish Verbs:')
	label96.set_alignment(0,0.5)
	browse2hbox=HBox(0,0)
	browse2hbox.set_spacing(10)
	browse2hbox.pack_start(label96,1,1,0)
	browsehelpbutt=getIconButton (pythonolwin,STOCK_DIALOG_QUESTION,"Help",1) 
	browsehelpbutt.connect("clicked",self.showVerbBrowserHelp)
	browse2hbox.pack_start(browsehelpbutt,0,0,2)
	browsensave=getIconButton (pythonolwin,STOCK_SAVE,"Save To File",1) 
	browsensave.connect("clicked",self.saveVerbBrowse)
	browse2hbox.pack_start(browsensave,0,0,0)
	browseverbbox.pack_start(browse2hbox,0,0,4)
	browseverbscroll=ScrolledWindow()
	self.browseverbscroll=browseverbscroll
	browseverbtext=TextView()
	browseverbtext.set_double_buffered(1)
	browseverbtext.set_wrap_mode(WRAP_WORD)
	browseverbtext.set_editable(1)
	self.browseverbtext=browseverbtext
	browseverbscroll.add(browseverbtext)

	browseverbscrollbox=HBox(0,0)
	browseverbscrollbox.pack_start(pythonol_festival.FestivalBar(pythonolwin,browseverbtext,2,1),0,0,0)
	browseverbscrollbox.pack_start(browseverbscroll,1,1,0)
	browseverbbox.pack_start(browseverbscrollbox,1,1,0)

	browsebuttbox=HBox(0,0)
	browsebuttbox.set_homogeneous(0)
	browsebuttbox.set_border_width(4)
	browsebuttbox.set_spacing(6)
	browse1=getIconButton (pythonolwin,STOCK_OK,"Show Page",1) 
	self.browse1=browse1
	browse1.connect("clicked",self.printBrowseVerbsCB )
	browsebuttbox.pack_start(Label("View Page:"),0,0,0)
	self.verbspagecombo=Combo()
	self.verbspagecombo.set_popdown_strings([' '])
	self.verbspageentry=self.verbspagecombo.entry
	self.verbspageentry.set_editable(0)
	browsebuttbox.pack_start(self.verbspagecombo,0,0,0)
	browsebuttbox.pack_start(browse1,0,0,0)
	browsebuttbox.pack_start(Label(" "),1,1,0)
	browseverbbox.pack_start(browsebuttbox,0,0,0)
	mynotebook.append_page(browseverbbox,getIconButton (pythonolwin,STOCK_JUMP_TO,"Verb\nBrowser",1,1) )

	# VerbFamilies
	verbfambox=VBox(0,0)
	self.verbfambox=verbfambox
	label57=Label(' '+'Browse Known Spanish Verb Families:')
	label57.set_alignment(0,0.5)
	verbfamhbox=HBox(0,0)
	verbfamhbox.set_spacing(10)
	verbfamhbox.pack_start(label57,1,1,0)
	verbfamhelpbutt=getIconButton (pythonolwin,STOCK_DIALOG_QUESTION,"Help",1) 
	verbfamhelpbutt.connect("clicked",self.showVerbFamHelp)
	verbfamhbox.pack_start(verbfamhelpbutt,0,0,2)
	famsave=getIconButton (pythonolwin,STOCK_SAVE,"Save To File",1) 
	famsave.connect("clicked",self.saveVerbFam)
	verbfamhbox.pack_start(famsave,0,0,0)
	verbfambox.pack_start(verbfamhbox,0,0,4)
	verbfamscroll=ScrolledWindow()
	self.verbfamscroll=verbfamscroll
	verbfamtext=TextView()
	verbfamtext.set_double_buffered(1)
	verbfamtext.set_wrap_mode(WRAP_WORD)
	verbfamtext.set_editable(0)
	self.verbfamtext=verbfamtext
	verbfamscroll.add(verbfamtext)
	verbfamscrollbox=HBox(0,0)
	verbfamscrollbox.pack_start(pythonol_festival.FestivalBar(pythonolwin,verbfamtext,2,1),0,0,0)
	verbfamscrollbox.pack_start(verbfamscroll,1,1,0)
	verbfambox.pack_start(verbfamscrollbox,1,1,0)
	mynotebook.append_page(verbfambox,getIconButton (pythonolwin,STOCK_HOME,"Verb\nFamilies",1,1) )

	# About Box
	aboutbox=VBox(0,0)
	self.aboutbox=aboutbox
	aboutpix=loadPixmap(getPixDir()+"title.gif",pythonolwin)
	if not aboutpix: aboutpix=Label(to_utf8(filter('Pythoñol ')+self.pversion))
	self.aboutpix=aboutpix
	aboutbox.pack_start(aboutpix,0,0,5)
	aboutscroll=ScrolledWindow()
	self.aboutscroll=aboutscroll
	abouttext=TextView()
	abouttext.set_wrap_mode(WRAP_WORD)
	abouttext.set_editable(0)
	self.abouttext=abouttext
	aboutscroll.add(abouttext)
	aboutbox.pack_start(aboutscroll,1,1,0)
	mynotebook.append_page(aboutbox,getIconButton (pythonolwin,STOCK_DIALOG_INFO,"About",1,1) )

	mainvbox.pack_start(mynotebook,1,1,0)
	pythonolwin.add(mainvbox)
	pythonolwin.connect("destroy",self.doQuit)
	pytheme.installTheme(mainvbox)
	pytheme.installTheme(pythonolwin)
	self.popstat="Loading data files...please wait."
	self.popstatlast=""

	wtips={label1:"Pythoñol "+self.pversion+"\nSpanish software for Linux.",browse1:"Show Page.\nVer Página.",conjcombo:"Choose a verb.\nElige un verbo.",conj_entry:"Enter a verb.\nEscribe un verb.",conjbutt:"Conjugate this verb.\nConjugar el verbo.",label12:"Save to file.\nGuardar.",	conjhelpbutt:"Help me!\n¡Ayudame!",dictradio1:"English -> Spanish\nInglés -> Español",dictradio2:"Spanish -> English\nEspañol -> Inglés",dictradio3:"English and Spanish\nInglés y Español",label73:"Save to file.\nGuardar.",dicthelpbutt:"Help me!\n¡Ayudame!",dictlookupbutt:"Look up this word or phrase.\nBuscarla",dictcombo:"Choose a word.\nElige una palabra.",dict_entry:"Enter a word or phrase.\nEscribe una palabra o una frase.",idlisteng:"List idioms in English.\nVer las frases comúnes en Inglés.",idlistspan:"List idioms in Spanish.\nVer las frases comúnes en Español.",label16:"Save to file.\nGuardar.",idsearchbutt:"Search for this keyword or phrase.\nBuscar la palabra o la frase.",idcombo:"Choose a word.\nElige una palabra.",id_entry:"Enter a word or phrase.\nEscribe una palabra o una frase.",browse11:"Show Page.\nVer Página.",ondemandcheck:"Use extra search features to help conjugate unknown verbs.\n\nUsar otros instrumentos para conjugar las palabras desconocidas.",verbfambutt:"Show the 'verb family' for this verb.\nVer 'la familia del verbo'.",exactbutt:"Show exact matches only, UN-checked.",famsave:"Save To File.\nGuardar.",browsensave:"Save To File.\nGuardar.", thes_entry:"Enter a word or phrase.\nEscribe una palabra o una frase",theslookupbutt:"Search for synonyms.\nBuscar los sinónimos",theshelpbutt:"Help me!\n¡Ayudame!",label78:"Save to file.\nGuardar.",thesradio2:"English\nInglés",thesradio1:"Spanish\nEspañol",label96:"Save to file.\nGuardar.",babwin.babtext1:"Enter a word or phrase to translate.\nEscribe una palabra o una frase para traducir.",babwin.babentry:"Select a language.\nEligir una idioma.",babwin.transbutton:"Translate.\nTraducir.",babwin.savebutton:"Save the translation.\nGuardar la traducción.",babwin.aboutbutton:"About PyBabelFish.\nSobre PyBabelFish.",babwin.babtext2:"Translation from BabelFish.\nLa traducción de BabelFish."}
	setTips(wtips)
	self.loaderW=None
	#pytheme.installTheme(mainvbox)
	#pytheme.installTheme(mynotebook)
	#pytheme.installTheme(pythonolwin)
	if use_filter==0: pythonolwin.show_all()
	self.loadDictionariesGo()
	self.color1=PYTHONOL_COLOR1
	self.color2=PYTHONOL_COLOR2
	self.color3=PYTHONOL_COLOR3
	self.color4=PYTHONOL_COLOR4
	dictradio3.set_active(1)
	self.desense()
	mynotebook.connect("switch_page",self.switchPage)

	self.conjugation_structure={
"PRESENT-PROGRESSIVE PARTICIPLE  (Gerundio)":{"":"gerundio"},
"PAST PARTICIPLE  (Participio)":{"":"participio"},
"PRESENT TENSE  (Presente)":{"Yo":"1pre","Tú":"2pre","Ella / Él / Usted":"3pre","Nosotros":"4pre","Vosotros":"5pre","Ellos / Ustedes":"6pre"},
"PRESENT-PROGRESSIVE  (Presente-Progresivo)":{"Yo":"1ppro","Tú":"2ppro","Ella / Él / Usted":"3ppro","Nosotros":"4ppro","Vosotros":"5ppro","Ellos / Ustedes":"6ppro"},
"PAST TENSE  (Pretérito)":{"Yo":"1pas","Tú":"2pas","Ella / Él / Usted":"3pas","Nosotros":"4pas","Vosotros":"5pas","Ellos / Ustedes":"6pas"},
"PAST-PROGRESSIVE / IMPERFECT  (Copretérito)":{"Yo":"1cop","Tú":"2cop","Ella / Él / Usted":"3cop","Nosotros":"4cop","Vosotros":"5cop","Ellos / Ustedes":"6cop"},
"FUTURE  (Futuro)":{"Yo":"1fut","Tú":"2fut","Ella / Él / Usted":"3fut","Nosotros":"4fut","Vosotros":"5fut","Ellos / Ustedes":"6fut"},
"CONDITIONAL (Condicional)":{"Yo":"1pos","Tú":"2pos","Ella / Él / Usted":"3pos","Nosotros":"4pos","Vosotros":"5pos","Ellos / Ustedes":"6pos"},
"PRESENT-SUBJUNCTIVE  (Subjuntivo-Presente)":{"Yo":"1pres","Tú":"2pres","Ella / Él / Usted":"3pres","Nosotros":"4pres","Vosotros":"5pres","Ellos / Ustedes":"6pres"},
"PAST-SUBJUNCTIVE  (Subjuntivo-Pretérito)":{"Yo":"1pass","Tú":"2pass","Ella / Él / Usted":"3pass","Nosotros":"4pass","Vosotros":"5pass","Ellos / Ustedes":"6pass"},
"PAST-SUBJUNCTIVE #2  (Subjuntivo-Pretérito #2)":{"Yo":"1passb","Tú":"2passb","Ella / Él / Usted":"3passb","Nosotros":"4passb","Vosotros":"5passb","Ellos / Ustedes":"6passb"},
"FUTURE-SUBJUNCTIVE  (Subjuntivo-Futuro)":{"Yo":"1futs","Tú":"2futs","Ella / Él / Usted":"3futs","Nosotros":"4futs","Vosotros":"5futs","Ellos / Ustedes":"6futs"},
"IMPERATIVE  (Imperativo)":{"Yo":"1imp","Tú":"2imp","Ella / Él / Usted":"3imp","Nosotros":"4imp","Vosotros":"5imp","Ellos / Ustedes":"6imp"},
"NEGATIVE IMPERATIVE (Imperativo-Negativo)":{"Yo":"1impn","Tú":"2impn","Ella / Él / Usted":"3impn","Nosotros":"4impn","Vosotros":"5impn","Ellos / Ustedes":"6impn"},
"PRESENT-PERFECT  (Presente-Perfecto)":{"Yo":"1pp","Tú":"2pp","Ella / Él / Usted":"3pp","Nosotros":"4pp","Vosotros":"5pp","Ellos / Ustedes":"6pp"},
"PAST-PERFECT  (Pasado-Perfecto)":{"Yo":"1pasp","Tú":"2pasp","Ella / Él / Usted":"3pasp","Nosotros":"4pasp","Vosotros":"5pasp","Ellos / Ustedes":"6pasp"},
"PRETERIT-PERFECT  (Pretérito-Perfecto)":{"Yo":"1prep","Tú":"2prep","Ella / Él / Usted":"3prep","Nosotros":"4prep","Vosotros":"5prep","Ellos / Ustedes":"6prep"},
"FUTURE-PERFECT  (Futuro-Perfecto)":{"Yo":"1futp","Tú":"2futp","Ella / Él / Usted":"3futp","Nosotros":"4futp","Vosotros":"5futp","Ellos / Ustedes":"6futp"},
"CONDITIONAL-PERFECT  (Condicional-Perfecto)":{"Yo":"1conp","Tú":"2conp","Ella / Él / Usted":"3conp","Nosotros":"4conp","Vosotros":"5conp","Ellos / Ustedes":"6conp"},
"FUTURE-PERFECT-SUBJUNCTIVE  (Futuro-Perfecto-Subjuntivo)":{"Yo":"1fps","Tú":"2fps","Ella / Él / Usted":"3fps","Nosotros":"4fps","Vosotros":"5fps","Ellos / Ustedes":"6fps"},
"FUTURE-PROGRESSIVE  (Futuro-Progresivo)":{"Yo":"1fpro","Tú":"2fpro","Ella / Él / Usted":"3fpro","Nosotros":"4fpro","Vosotros":"5fpro","Ellos / Ustedes":"6fpro"},
"FUTURE-PERFECT-PROGRESSIVE  (Futuro-Perfecto-Progresivo)":{"Yo":"1fpp","Tú":"2fpp","Ella / Él / Usted":"3fpp","Nosotros":"4fpp","Vosotros":"5fpp","Ellos / Ustedes":"6fpp"},
"IMPERFECT PROGRESSIVE (Progresivo-Imperfecto)":{"Yo":"1ip","Tú":"2ip","Ella / Él / Usted":"3ip","Nosotros":"4ip","Vosotros":"5ip","Ellos / Ustedes":"6ip"},
"PRESENT PERFECT SUBJUNCTIVE (Presente-Perfecto-Subjuntivo)":{"Yo":"1presps","Tú":"2presps","Ella / Él / Usted":"3presps","Nosotros":"4presps","Vosotros":"5presps","Ellos / Ustedes":"6presps"},
"PAST PERFECT SUBJUNCTIVE (Pasado-Perfecto-Subjuntivo)":{"Yo":"1pastps","Tú":"2pastps","Ella / Él / Usted":"3pastps","Nosotros":"4pastps","Vosotros":"5pastps","Ellos / Ustedes":"6pastps"},
"FUTURE POPULAR (Futuro-Popular)":{"Yo":"1fpop","Tú":"2fpop","Ella / Él / Usted":"3fpop","Nosotros":"4fpop","Vosotros":"5fpop","Ellos / Ustedes":"6fpop"},
"CONDITIONAL PERFECT PROGRESSIVE  (Condicional-Perfecto-Progresivo)":{"Yo":"1cpp","Tú":"2cpp","Ella / Él / Usted":"3cpp","Nosotros":"4cpp","Vosotros":"5cpp","Ellos / Ustedes":"6cpp"},
"PRESENT PERFECT PROGRESSIVE  (Presente-Perfecto-Progresivo)":{"Yo":"1ppp","Tú":"2ppp","Ella / Él / Usted":"3ppp","Nosotros":"4ppp","Vosotros":"5ppp","Ellos / Ustedes":"6ppp"}
}  
	self.normal_order=["Yo","Tú","Ella / Él / Usted","Nosotros","Vosotros","Ellos / Ustedes"]
	self.conjugation_order=["PRESENT-PROGRESSIVE PARTICIPLE  (Gerundio)","PAST PARTICIPLE  (Participio)","BREAK-IT","PRESENT TENSE  (Presente)","PRESENT-PROGRESSIVE  (Presente-Progresivo)","PRESENT-SUBJUNCTIVE  (Subjuntivo-Presente)","PRESENT-PERFECT  (Presente-Perfecto)","PRESENT PERFECT PROGRESSIVE  (Presente-Perfecto-Progresivo)","PRESENT PERFECT SUBJUNCTIVE (Presente-Perfecto-Subjuntivo)","BREAK-IT","PAST TENSE  (Pretérito)","PAST-PROGRESSIVE / IMPERFECT  (Copretérito)","IMPERFECT PROGRESSIVE (Progresivo-Imperfecto)","PAST-SUBJUNCTIVE  (Subjuntivo-Pretérito)","PAST-SUBJUNCTIVE #2  (Subjuntivo-Pretérito #2)","PAST-PERFECT  (Pasado-Perfecto)","PAST PERFECT SUBJUNCTIVE (Pasado-Perfecto-Subjuntivo)","PRETERIT-PERFECT  (Pretérito-Perfecto)","BREAK-IT","FUTURE  (Futuro)","FUTURE POPULAR (Futuro-Popular)","FUTURE-PROGRESSIVE  (Futuro-Progresivo)","FUTURE-SUBJUNCTIVE  (Subjuntivo-Futuro)","FUTURE-PERFECT  (Futuro-Perfecto)","FUTURE-PERFECT-PROGRESSIVE  (Futuro-Perfecto-Progresivo)","FUTURE-PERFECT-SUBJUNCTIVE  (Futuro-Perfecto-Subjuntivo)","BREAK-IT","CONDITIONAL (Condicional)","CONDITIONAL-PERFECT  (Condicional-Perfecto)","CONDITIONAL PERFECT PROGRESSIVE  (Condicional-Perfecto-Progresivo)","BREAK-IT","IMPERATIVE  (Imperativo)","NEGATIVE IMPERATIVE (Imperativo-Negativo)"]

    def addScroller(self,*args):
        # create a notebook scroller on windows
        # The windows version of Gtk doesnt create a visible Notebook scroller
        # NOTE: As of verson 2.1, this isn't needed anymore, since
        # Gtk-2 on Windows now has this fixed!
        # code kept for legacy
        return
        left_button=Button()
        right_button=Button()
        left_button.add(loadScaledImage(STOCK_GO_BACK,16,16,1))
        right_button.add(loadScaledImage(STOCK_GO_FORWARD,16,16,1))
        left_button.connect("clicked",self.moveTabLeft)
        right_button.connect("clicked",self.moveTabRight)
        smallhb=HBox(0,0)
        smallhb.set_border_width(1)
        smallhb.pack_start(Label("      "),0,0,0)
        smallhb.pack_start(left_button,0,0,0)
        smallhb.pack_start(Label("  "),1,1,0)
        smallhb.pack_start(right_button,0,0,0)
        smallhb.pack_start(Label("      "),0,0,0)
        smallhb.show_all()
        self.mainvbox.pack_start(smallhb,0,0,0)

    def moveTabLeft(self,*args):
        self.mynotebook.prev_page()
    def moveTabRight(self,*args):
        self.mynotebook.next_page()


    def reinstallDB(self,*args):
	if not self.showConfirm("You are about to re-build the Pythoñol language databases.\n\nThis could make permanent (possibly dangerous) changes to the Pythoñol databases needed to run this application. Rebuilding the databases may take up to 3-4 minutes, so be patient.  Be sure you know exactly what you're doing before you proceed.\n\nAre you SURE you want to continue?")==1: return
	pythonol_setup.checkPythonolSetup(1)
	pythonol_setup.showGtkMessage("Pythoñol will now exit.\n\nPlease re-start Pythoñol.")
	self.doQuit()

    def makePermChildSafe(self,*args):
	if not self.showConfirm("Are you SURE you want to make Pythoñol\nPERMANENTLY Child-friendly and remove ALL\n'mature' content from the language databases?")==1: return
	if pythonol_setup.checkWantChildFriendly()==1:
		pythonol_setup.showGtkMessage("Pythoñol will now exit.\n\nPlease re-start Pythoñol.")
		self.doQuit()

    def loadDictionariesGo2(self,*args):
	child_friendly=0 # try to enable mature content if avail on hard-disk
	pythonol_setup.child_friendly=0
	self.loadDictionariesGo()

    def askTransService(self,*args):
	self.babwin.askService()

    def showCapabilities(self,*args):
	showHelp(self.getCapabilities(),"Pythoñol Capabilities",1,1)

    def getCapabilities(self) :
	s="[PHSECTION]Pythoñol Capabilities\n\n[PH-HI]MATURE CONTENT ENABLED:[/PH-HI]  "
	if self.mature_cursor==None: s=s+"no  (child-friendly mode)\n"
	else: s=s+"yes\n"
	if pythonol_festival.isSupported(): s=s+"[PH-HI]TEXT-TO-SPEECH AUDIO SUPPORT ENABLED (FESTIVAL):[/PH-HI]  yes\n"
	else: s=s+"[PH-HI]TEXT-TO-SPEECH AUDIO SUPPORT ENABLED (FESTIVAL):[/PH-HI]  no\n"
	s=s+"\n[PH-HI]DICTIONARY:[/PH-HI]\n\tEnglish words:  "+str (self.getDatabaseCount(self.idioms_cursor)+self.getDatabaseCount(self.dictionary_cursor)+self.getDatabaseCount(self.mature_cursor) )+"\n\tSpanish words:  "+str (self.getDatabaseCount(self.idioms_cursor)+self.getDatabaseCount(self.dictionary_cursor)+self.getDatabaseCount(self.mature_cursor) )+"\n\n"
	s=s+"[PH-HI]IDIOMS:[/PH-HI]\n\tEnglish and Spanish idioms:  "+str (self.getDatabaseCount(self.idioms_cursor)+self.getDatabaseCount(self.mature_cursor) )+"\n\n"
	s=s+"[PH-HI]THESAURUS:[/PH-HI]  "+str(self.getDatabaseCount(self.thes_cats_cursor))+" synonym categories\n"
	s=s+"[PH-HI]VERB TENSES SUPPORTED (Conjugation):[/PH-HI]  "+str(len(self.conjugation_structure.keys()))+"\n"
	s=s+"[PH-HI]IRREGULAR VERBS SUPPORTED:[/PH-HI]  "+str(len(irregular_verbs.irregular_verbs.keys()))+"\n"
	s=s+"[PH-HI]KNOWN VERB FAMILIES:[/PH-HI]  "+str(len(verb_families.verb_families.keys()))+"\n"
	s=s+"[PH-HI]VERSION:[/PH-HI]  "+self.pversion+"\n"
	return s

    def getDatabaseCount(self,db_cursor) :
	try:
		db_cursor.execute("SELECT COUNT(*) from definitions")
		for dbrow in db_cursor.fetchall():
			return int(dbrow[0])
	except:
		return 0

    def closeDatabaseConnections(self,*args):
	db_connections=[self.mature,self.idioms,self.thes_cats,self.dictionary]
	for gg in db_connections:
		try:
			gg.close()
		except:
			pass
	try:
		del self.dictionary
		del self.dictionary_cursor
		del self.thes_cats
		del self.thes_cats_cursor
		del self.idioms
		del self.idioms_cursor
		del self.mature
		del self.mature_cursor
	except:
		pass

    def resense(self,*args) :
	self.browse11.set_sensitive(1)

    def desense(self,*args) :
	self.browse11.set_sensitive(0)


    def showVerbFamily(self,*args) :
	if not self.conj_entry.get_text().strip(): return
	if not verb_conjugate.isVerb(self.conj_entry.get_text().strip().lower()):
		ss=" The term [PH-HI]'"+self.conj_entry.get_text().strip().lower()+"'[/PH-HI] does not appear to be a normal Spanish verb."
	else:
		ss="[PHSECTION]VERB FAMILY FOR\n[PH-HI]'"+self.conj_entry.get_text().strip().lower()+"'[/PH-HI]\n\n Verbs conjugated in a very similar manner include:\n\n"
		vf=verb_families.getVerbFamily(self.conj_entry.get_text().strip().lower())
		if len(vf)<2: ss=ss+"    The verb [PH-HI]'"+self.conj_entry.get_text().strip().lower()+"'[/PH-HI] does not appear to be a member of any well-known verb 'family'."
		else:
			for ii in vf:
				ss=ss+"    "+ii+"\n"
	showHelp(ss,"Pythoñol: Verb Family")

    def printVerbFamilies(self,*args) :
     try:
	self.setText(self.verbfamtext,"")
	text_area_insert(self.verbfamtext, self.color1, "THIS IS A LIST OF KNOWN GROUPS OF SPANISH VERBS WHICH ARE ALL CONJUGATED PRETTY MUCH THE SAME WAY.\n\n")
	verb_families.loadVF()
	vc=verb_families.verb_families.values()
	vc.sort()
	ss=""
	for ii in vc:
		vclist=ii
		if len(vclist)<2: continue
		vclist.sort()
		for yy in vclist:
				#print yy
				if not vclist.index(yy)==len(vclist)-1: 
					text_area_insert(self.verbfamtext, self.color3,filter(yy)+", ")
				else: 
					text_area_insert(self.verbfamtext, self.color3,  filter(yy))
		text_area_insert(self.verbfamtext, self.color3, "\n\n")
     except:
	pass   
     return FALSE

    def printBrowseVerbsCB(self,*args):
	if self.verbspageentry.get_text().find("Page")==-1: return
	pageno=self.verbspageentry.get_text().replace("Page","").strip()
	self.printBrowseVerbs(None,int(pageno))


    def printBrowseVerbs(self,ww=None,mypage=1,*args) :
	self.browseverbtext.freeze_notify()
	startpage=mypage-1
	pagelimit=500
	startpage=startpage*pagelimit
	try:
		self.setText(self.browseverbtext,"")
		getSQL(self.dictionary_cursor,"SELECT spanish,english from definitions where (spanish LIKE '%ar') OR (spanish LIKE '%ir') OR (spanish LIKE '%er') OR (spanish LIKE '%er %')  OR (spanish LIKE '%er/%')  OR (spanish LIKE '%ar %')  OR (spanish LIKE '%ar/%')   OR (spanish LIKE '%ir %')  OR (spanish LIKE '%ir/%') ORDER by spanish LIMIT "+str(startpage)+","+str(pagelimit))
		for ii in self.dictionary_cursor.fetchall():
			text_area_insert(self.browseverbtext, self.color2,filter(ii[0])+"       ")
			text_area_insert(self.browseverbtext, self.color3, "["+filter(ii[1])+"]\n")
	except:
		pass
	self.browseverbtext.thaw_notify()
	return FALSE

    def saveVerbBrowse(self,*args) :
	self.saveTextArea(self.browseverbtext)

    def saveVerbFam(self,*args) :
	self.saveTextArea(self.verbfamtext)

    def saveConj(self,*args) :
	self.saveTextArea(self.conjtext)

    def saveThes(self,*args) :
	self.saveTextArea(self.thestext)

    def saveId(self,*args) :
	self.saveTextArea(self.idtext)

    def saveDict(self,*args) :
	self.saveTextArea(self.dicttext)

    def saveTextArea(self,text_widget) :
	if len(text_buffer_get_text(text_widget).strip())==0:
		self.showMessage("Nothing to save in this window!")
		return
	self.save_to_text=text_buffer_get_text(text_widget)
	SELECT_A_FILE(self.saveTextAreaCB,
 	FILE_SELECTOR_TITLE,  DIALOG_WMC, DIALOG_WMN, None, "Save", "Cancel")

    def saveTextAreaCB(self, *args) :
	ff=GET_SELECTED_FILE()
	if ff==None: return
	if ff=="": return
	if ff.endswith("/"): return # a directory
	try:
		if wine_friendly==0:
			if os.path.exists(ff):
				if not self.showConfirm("The file '"+ff+"' already exists.\nReplace It?")==1: return
		f=open(ff,"w")
		f.write( unfilter( self.save_to_text ) )
		f.flush()
		f.close()
		self.showMessage("Data successfully saved to file '"+ff+"'.")
	except:
		self.showMessage("Error saving data to file: "+ff)

    def getQuickDefinition(self,any_word) :
	if str(any_word).strip()=="comer": return "eat"
	if str(any_word).strip()=="mentir": return "tell a lie"
	for ii in self.all_dicts:
		exdef=getExactDefinition(ii,str(any_word).strip(),0,1,0)
		if len(exdef)>0: return exdef[0][0]
	return ""

    def searchOnDemand(self,verb_word) :
	# first do a conjugation check
	vtense=str(verb_word).lower().strip()
	reflexive=0
	#rflist=["me ","te ","se ","nos ","os "]
	for ii in ["me ","te ","se ","nos ","os "]:
		if vtense.startswith(ii): 
			reflexive=1
			vtense=vtense.replace(ii,"").strip()
			break
	if vtense.find(" ")==-1: # we have a single word, possibly conjugatable
	  try:
		revc=verb_conjugate.reverseConjugate(vtense,self.all_dicts)
		#print "revc:   "+str(revc)
		if revc:
			if len(revc)==2:
				newverb=revc[0]
				nvdef=revc[1]
				if reflexive==1: 
					newverb=newverb+"se"
					for ii in self.all_dicts:
						edef=getExactDefinition(ii,newverb,0,1,0)
						if len(edef)>0: 
							nvdef=edef[0][0]
							break
				cmessage="The term '"+str(verb_word).strip()+"' appears to be a verb tense of the infinitive '"+newverb.upper()+"'.\n\n"
				if len(nvdef)>0: cmessage=cmessage+"   Meaning:  ("+nvdef+")\n"
				cmessage=cmessage+"\nThis may not be an actual Spanish VERB, but is Pythoñol's best guess.\n\nWhat would you like to do?"
				conjask=self.askConjugateConfirm(cmessage)
				if conjask==1:
					self.conjugateVerb(newverb)
					return
				if conjask==3: return
	  except:
		pass
	searchask=self.askSearchConfirm()
	if searchask==4: return
	MENG=0
	MSPA=0
	if searchask==1: MENG=1
	if searchask==2: MSPA=1
	if searchask==3: 
		MENG=1
		MSPA=1
	lookdata=[]
	pverbs={}
	for ii in self.all_dicts:
		mat=getDefinitions2(ii,verb_word,1,MSPA,MENG,0)
		if len(mat)>0: lookdata=lookdata+mat
	for ii in lookdata:
	    #if (ii[1].strip().find(" ")==-1) and (ii[1].strip().find("/")==-1) and (ii[1].strip().find("[")==-1) and (ii[1].strip().find("(")==-1):
	    if verb_conjugate.isVerb(ii[0].strip()): 
			pverbs[ii[0]]=0  # spanish
	    else: 
		iism=ii[1]
		for mm in ["  ","[","(","/"]:
			if iism.find(mm)>0: iism=iism[0:iism.find(mm)].strip()
		if verb_conjugate.isVerb(iism): pverbs[iism]=1  # english	
	lookdata=[]
	verb_dict={}
	#print str(pverbs)
	for yy in pverbs.keys()[:80]:
		pdef=""
		for ii in self.all_dicts:
			pdef=getExactDefinition(ii,yy,0,1,0)
			if len(pdef)>0:				
				break
		if len(pdef)>0:
			verb_dict[yy]=pdef[0][0]
	pverbs.clear()
	if not len(verb_dict)>0:
		self.setText(self.conjtext," Sorry, the word '"+str(verb_word).strip()+"' could not be conjugated as a spanish verb.\nIn addition, 'Search-On-Demand' was unable to find any possible matches.  Please check your spelling and try again.")
	else:
		searchwin(self,verb_dict)
		verb_dict.clear()

    def conjugateHandler(self,*args) :
	if not self.conj_entry.get_text().strip(): return
	self.conjugateVerb(self.conj_entry.get_text())

    def conjugateVerb(self,verb_word) :
     quick_def=""
     try:
	verbdict=verb_conjugate.conjugate(str(verb_word).strip())
	self.conj_history.insert(0,str(filter(verb_word)).strip())
	if len(self.conj_history)>30: self.conj_history=self.conj_history[0:20]
	self.conjcombo.set_popdown_strings(self.conj_history)

	if len(verbdict)==0:
		if self.ondemandcheck.get_active():
			self.searchOnDemand(str(verb_word).strip())
		else: 
			self.setText(self.conjtext," Sorry, the word '"+str(filter(verb_word)).strip()+"' could not be conjugated as a spanish verb.\nCheck your spelling and/or try enabling 'Search-On-Demand' above.")
	else:
		self.setText(self.conjtext,"")
		text_area_insert(self.conjtext, self.color1, " INFINITIVE: "+str(unfilter(verb_word)).strip())
		quick_def=filter(self.getQuickDefinition(str(verb_word).strip()))
		if len(quick_def)==0: quick_def=self.getQuickDefinition(verb_conjugate.getRawVerb(filter(str(verb_word)).strip()))
		if len(quick_def)>0: 
			if quick_def.find("(")>1: quick_def=quick_def[0:quick_def.find("(")].strip()
			text_area_insert(self.conjtext, self.color1, "  ("+quick_def+")")
		if verb_conjugate.isReflexive(str(verb_word).strip()): 
			text_area_insert(self.conjtext, self.color1, "  - Reflexive")
		text_area_insert(self.conjtext, self.color1, "\n")
		for ii in self.conjugation_order:
			if ii=="BREAK-IT":				
				text_area_insert(self.conjtext, self.color3, "_________________________________________________________\n")
				continue				
			text_area_insert(self.conjtext, self.color3, "\n "+filter(ii)+":\n")
			iidict=self.conjugation_structure[ii]
			if iidict.has_key("Yo") and iidict.has_key("Vosotros"):
				iikeys=self.normal_order
			else:
				iikeys=iidict.keys()
				iikeys.sort()
			for yy in iikeys:
				try:
					text_area_insert(self.conjtext, self.color2, "     "+filter(yy)+" "+verbdict[iidict[yy]]+".")
					if len(quick_def)>0:
						try:
							vscheme=self.getVerbScheme(iidict[yy])
							vsubj=self.isSubjunctive(iidict[yy])
							translation=span2eng.translate(yy,unfilter(verbdict[iidict[yy]]),quick_def,vscheme,vsubj)
							if len(translation)>0: 
								text_area_insert(self.conjtext, self.color4, "      ["+filter(translation)+"]")
						except:
							pass
					text_area_insert(self.conjtext, self.color1, "\n")
				except:
					pass
     except:
	pass
     if len(quick_def)>0: self.conjstat.set_text(to_utf8(str(filter(verb_word)).strip().upper()+"  ("+quick_def+")"))
     else: self.conjstat.set_text(to_utf8(str(filter(verb_word)).strip().upper()))

    def  isSubjunctive(self,yy):
	#gerundio  participio  pre  ppro  pas  cop  fut pos pres pass passb futs  imp  impn pp pasp prep futp  conp fps
	for zz in ["pres","pass","passb","futs","fps"]:
		if yy.endswith(zz): return 1
	return 0

    def  getVerbScheme(self,yy):
	#gerundio  participio  pre  ppro  pas  cop  fut pos pres pass passb futs  imp  impn pp pasp prep futp  conp fps
	vscheme=1
	for zz in ["pres","pre","pp"]:
		if yy.endswith(zz): vscheme=1
	if yy.endswith("ppro"): vscheme=2
	for zz in ["pas","pass","passb"]:
		if yy.endswith(zz): vscheme=3
	if yy.endswith("fut") or yy.endswith("futs"): vscheme=4
	if yy.endswith("pos"): vscheme=5
	if yy.endswith("imp"): vscheme=6
	if yy.endswith("impn"): vscheme=7
	if yy.endswith("participio"): vscheme=8
	for zz in ["pp","pasp","prep","futp","conp","fps","presps","pastps"]:
		if yy.endswith(zz): vscheme=8
	if yy.endswith("gerundio"): vscheme=9
	if yy.endswith("cop") or yy.endswith("ip"): vscheme=10
	if yy.endswith("fpro") or yy.endswith("fpp"):  vscheme=11
	if yy.endswith("ppp") or yy.endswith("cpp"):  vscheme=11
	if yy.endswith("fpop"): vscheme=12
	return vscheme

    def loadDictionariesGo(self,*args) :	
	self.popstat="Loading data files...please wait."
	self.popstatlast=""
	if use_filter==0:  
		self.loaderW=loaderwin(self)
		tt=Thread(None,self.loadDictionaries)
		timeout_add(150,self.checkLoader)
		tt.start()
	elif wine_friendly==1: 
		timeout_add(150,self.checkLoader)
		self.loadDictionaries()
	else:
		self.loaderW=loaderwin(self)
		timeout_add(50,self.loadDictionaries)
		timeout_add(3000,self.checkLoader)
                #self.loadDictionaries()


    def showLoader(self,*args):
        self.loaderW=loaderwin(self)
        return 0

    def makeChildSafe(self,*args) :
	try:
		self.mature.close()
	except:
		pass
	self.mature=None
	self.mature_cursor=None
	self.all_dicts=[]
	if not self.dictionary_cursor==None: self.all_dicts.append(self.dictionary_cursor)
	if not self.idioms_cursor==None: self.all_dicts.append(self.idioms_cursor)
	self.idioms_dicts=[]
	if not self.idioms_cursor==None: self.idioms_dicts.append(self.idioms_cursor)
	self.doChildSafe()

    def doChildSafe(self) :
	if self.mature_cursor==None:
		if use_filter==0: self.pythonolwin.set_title(to_utf8(filter('Pythoñol '+self.pversion+" (Child-friendly mode)")))
		self.conjstat.set_text(to_utf8(filter("Pythoñol version "+self.pversion+" ready.  (Child-friendly mode)")))
		child_friendly=1
		pythonol_setup.child_friendly=1
		pybabelfish.child_friendly=1
		pyfilter.child_friendly=1
	else: 
		if use_filter==0: self.conjstat.set_text(to_utf8(filter("Pythoñol version "+self.pversion+" ready.")))
		self.pythonolwin.set_title(to_utf8(filter('Pythoñol '+self.pversion)))
		child_friendly=0
		pythonol_setup.child_friendly=0
		pybabelfish.child_friendly=0
		pyfilter.child_friendly=0
	self.pythonolwin.show_all()

    def checkLoader(self,*args) :
	if self.popstat==self.popstatlast: return TRUE
	if self.popstat.find("Loading Complete")>-1:
		if len(self.warnings)>0: self.setText(self.conjtext,filter(self.warnings))
		self.warnings=""		
		self.showAbout()
		self.printVerbFamilies()
		self.max_idioms=-2
		try:
			idpages=self.getDatabaseCount(self.idioms_cursor)
			if idpages>499:
				pagelist=[]
				pcounter=math.ceil(idpages/500.0)
				pci=0
				while pci<pcounter:
					pagelist.append("Page "+str(pci+1))
					pci=pci+1
				self.max_idioms=pci+1
				pagelist.append("Page "+str(pci+1))
				self.idiompagecombo.set_popdown_strings(pagelist)
		except:
			pass
		try:
			idpages=41
			pagelist=[]
			pci=0
			while pci<idpages:
					pagelist.append("Page "+str(pci+1))
					pci=pci+1
			self.verbspagecombo.set_popdown_strings(pagelist)
		except:
			pass
		if use_filter==1: self.doChildSafe()
		if wine_friendly==0:  self.loaderW.hide()
		if wine_friendly==0:  self.loaderW.destroy()
		if use_filter==0:  self.loaderW.unmap()
		self.pythonolwin.show_all()
		if use_filter==0: self.doChildSafe()
		self.popstat=""
		self.popstatlast=""
		return FALSE
	if wine_friendly==1: print self.popstat
	if pyfilter.use_filter==0:  self.popstatlabel.set_text(to_utf8(filter(self.popstat)))
	self.popstatlast=""+self.popstat
	return TRUE

    def loadDictionaries(self,*args) :
     try:
	self.warnings=""
	try:
		self.closeDatabaseConnections()
	except:
		pass
	time.sleep(1.3) # hold, so the splash screen can show
	# database connections and cursors
	self.dictionary=None
	self.dictionary_cursor=None
	self.thes_cats=None
	self.thes_cats_cursor=None
	self.idioms=None
	self.idioms_cursor=None
	self.mature=None
	self.mature_cursor=None
	load_dict=["dictionary.db",  "dictionary-idioms.db", "dictionary-mature.db"]

	thes_file="thesaurus-cats.db"
	self.popstat="Loading thesaurus file: "+thes_file 
	self.thes_cats=pythesaurus.loadThesaurus(thes_file)
	if self.thes_cats==None: self.warnings=self.warnings+"\n"+thes_file
	else: 
		try:
			self.thes_cats_cursor=self.thes_cats.cursor()
		except:
			self.warnings=self.warnings+"\n"+thes_file

	for ii in load_dict:
		self.popstat="Loading data file: "+ii
		if (pythonol_setup.child_friendly==1) and (ii.lower().find("mature")>-1): continue #block loading of mature content on CF versions
		conn=loadDictionary(ii)  # get database connection
		if conn==None:  # connection failed
			if ii.lower().find("mature")==-1:  # don't complain about not being able to load the 'mature'/adult content
				self.warnings=self.warnings+"\n"+ii
			else: 
				self.mature_cursor=None
				self.mature=None
		else: # get database conn cursor
			try:
				cursor=conn.cursor()
				if ii=="dictionary.db":
					self.dictionary=conn
					self.dictionary_cursor=cursor
				if ii=="dictionary-mature.db":
					self.mature=conn
					self.mature_cursor=cursor
				if ii=="dictionary-idioms.db":
					self.idioms=conn
					self.idioms_cursor=cursor
			except:
				self.warnings=self.warnings+"\n"+ii

	if len(self.warnings)>0:
		self.warnings="ATTENTION:  There were ERRORS loading the following data files:\n"+self.warnings+"\n\nThese errors may severely limit the functionality of this program.  You are strongly encouraged to exit this program, restore the problem data files to their original location and condition, and restart this program."
		#print warnings

	#print str(dir(self.mature_cursor))
	self.popstat="Initializing databases...please wait."
	self.all_dicts=[]
	if not self.dictionary_cursor==None: self.all_dicts.append(self.dictionary_cursor)
	if not self.idioms_cursor==None: self.all_dicts.append(self.idioms_cursor)
	if not self.mature_cursor==None: self.all_dicts.append(self.mature_cursor)
	self.idioms_dicts=[]
	self.popstat="Preparing idioms...please wait."
	if not self.idioms_cursor==None: self.idioms_dicts.append(self.idioms_cursor)
	if not self.mature_cursor==None: self.idioms_dicts.append(self.mature_cursor)
	#self.listIdiomsEng()
	self.popstat="Initializing verb families...please wait."
	#self.printVerbFamilies()
	#self.popstat="Initializing verb browser...please wait."
	#self.printBrowseVerbs()  # disabled for start-up speed improvement
     except:
	pass
     self.popstat="Loading Complete."
     return FALSE


    def listIdiomsEng(self,*args):
	self.IDIOM_BROWSE=1
	self.listIdioms(None,1)
	self.resense()

    def listIdiomsSpa(self,*args):
	self.IDIOM_BROWSE=2
	self.listIdioms(None,1)
	self.resense()

    def listIdiomsCB(self,*args):
	if self.idiompageentry.get_text().find("Page")==-1: return
	pageno=self.idiompageentry.get_text().replace("Page","").strip()
	self.listIdioms(None,int(pageno))

    def listIdioms(self,wwwidget=None,mypage=1,*args) :
	self.idscroll.freeze_notify()
	startpage=mypage-1
	pagelimit=500
	startpage=startpage*pagelimit
	try:
		ddkeys=[]
		ddict={}
		if self.IDIOM_BROWSE==1: 
			query_str="SELECT english,spanish from definitions  ORDER BY english LIMIT "
		else: 
			query_str="SELECT spanish,english from definitions ORDER BY spanish LIMIT "
		ddkeys.sort()
		for gg in self.idioms_dicts:
			if gg==self.mature_cursor: continue			
			else: 
				if mypage < self.max_idioms:
					#getSQL(gg,query_str+str(pagelimit)+","+str(startpage))
					getSQL(gg,query_str+str(startpage)+","+str(pagelimit))
					for dbrow in gg.fetchall():
						ddict[filter(str(dbrow[0]))]=filter(str(dbrow[1]))
		self.setText(self.idtext,"")
		ddkeys=ddict.keys()
		ddkeys.sort()
		for ii in ddkeys:
			text_area_insert(self.idtext, self.color2, ii+"       ")
			text_area_insert(self.idtext, self.color3, "["+ddict[ii]+"]\n")
		if (not self.mature_cursor==None) and (mypage==self.max_idioms): # append 'mature' stuff at the end
			text_area_insert(self.idtext, self.color1, "\n____________________________\n")
			text_area_insert(self.idtext, self.color1, "           MATURE IDIOMS    ")
			text_area_insert(self.idtext, self.color1,"\n____________________________\n\n")
			getSQL(self.mature_cursor,query_str.replace("LIMIT",""))
			ddict={}
			for dbrow in self.mature_cursor.fetchall():
				ddict[filter(str(dbrow[0]))]=filter(str(dbrow[1]))
			ddkeys=ddict.keys()
			ddkeys.sort()
			for ii in ddkeys:
				text_area_insert(self.idtext, self.color1, ii+"       ")
				text_area_insert(self.idtext, self.color3, "["+ddict[ii]+"]\n")
	except:
		pass
	self.idscroll.thaw_notify()
	return FALSE

    def getDictionaryDefinition(self,*args):
	if not self.dict_entry.get_text().strip(): return
	if self.isTooBroad(self.dict_entry.get_text().strip())==1:
		self.setText(self.dicttext ,"Sorry, the search term '"+filter(self.dict_entry.get_text().strip())+"' is too broad.")
		return
	dexact=""
	lookdata=[]
	spanish=0
	english=0
	lgroup=self.all_dicts
	if self.dictradio1.get_active(): #english
		english=1
	if self.dictradio2.get_active(): #span
		spanish=1
	if self.dictradio3.get_active(): #both
		english=1
		spanish=1
	if self.exactbutt.get_active()==0:  # do short search
		for ii in lgroup:
			mat=getExactDefinition(ii,self.dict_entry.get_text(),1,spanish,english)
			if len(mat)>0: lookdata=lookdata+mat
	else:  # do extended search
		for ii in lgroup:
				dex=getExactDefinition(ii,self.dict_entry.get_text(),1,spanish,english)
				if len(dex)>0: 
					appender="\t"+dex[0][0] +"    ("+dex[0][1]+")\n"
					if dexact.find(appender)==-1: dexact=dexact+appender
				mat=getDefinitions2(ii,self.dict_entry.get_text(),1,spanish,english,0)
				if len(mat)>0: lookdata=lookdata+mat
				if len(lookdata)>1499: break
	self.dicttext.freeze_notify()
	self.setText(self.dicttext,"")
	match_found=0
	if len(dexact)>0:  
		text_area_insert(self.dicttext, self.color1, " EXACT MATCHES:\n"+dexact+"\n\n")
		match_found=1
	tempdict=[]
	for ii in lookdata: 
		if not ii in tempdict: tempdict.append(ii)
	lookdata=tempdict
	lookdata.sort()	
	for ii in lookdata:
		match_found=1
		text_area_insert(self.dicttext, self.color2, " "+ii[0])
		if len(ii[1]): 
			text_area_insert(self.dicttext, self.color3, "    ("+ii[1]+")\n")
		else: 
			text_area_insert(self.dicttext, self.color3, " \n")

	if match_found==1: 
		if not self.exactbutt.get_active():	
			text_area_insert(self.dicttext, self.color3, "\n\n_______________________\nFor more results, re-try your search, making sure that 'Extended search' is CHECKED and 'Both' is selected.\n\nHowever, 'extended' searches do take slightly longer.")
	tempdict=[]
	lookdata=[]
	if match_found==0: 
		text_area_insert(self.dicttext, self.color3, "Sorry, no matches found.")		
		if not self.exactbutt.get_active(): 
			text_area_insert(self.dicttext, self.color3, "\n\nRe-try your search, making sure that 'Extended search' is CHECKED and 'Both' is selected.\n\nYou may get better results using the slightly longer 'extended' search.")
	self.dicttext.thaw_notify()
	self.dict_history.insert(0,filter(self.dict_entry.get_text().strip()))
	if len(self.dict_history)>30: self.dict_history=self.dict_history[0:20]
	self.dictcombo.set_popdown_strings(self.dict_history)

    def searchIdioms(self,*args):
	if not self.id_entry.get_text().strip(): return
	if self.isTooBroad(self.id_entry.get_text().strip())==1:
		self.setText(self.idtext ,"Sorry, the search term '"+filter(self.id_entry.get_text().strip())+"' is too broad.")
		return
	lookdata=[]
	dexact=""
	self.idtext.freeze_notify()
	self.setText(self.idtext,"")
	for ii in self.idioms_dicts:
		dex=getExactDefinition(ii,self.id_entry.get_text())
		if len(dex)>0: dexact=dexact+"    "+dex[0][0]+"    ("+dex[0][1]+")\n"
		mat=getDefinitions2(ii,self.id_entry.get_text(),1,1,1,0)
		if len(mat)>0: lookdata=lookdata+mat
	match_found=0
	if len(dexact)>0:  
		match_found=1
		text_area_insert(self.idtext, self.color1," EXACT MATCHES:\n"+dexact+"\n")
	for ii in lookdata:
		match_found=1
		text_area_insert(self.idtext, self.color2," "+ii[0])
		text_area_insert(self.idtext, self.color3,"    ("+ii[1]+")\n")
	if match_found==0: 
		text_area_insert(self.idtext, self.color3,"Sorry, no matches found.\nTry using ONE keyword for the best results.")
	self.idtext.thaw_notify()
	self.id_history.insert(0,filter(self.id_entry.get_text().strip()))
	if len(self.id_history)>30: self.id_history=self.id_history[0:20]
	self.idcombo.set_popdown_strings(self.id_history)
	self.desense()

    def isTooBroad(self,search_term):
	if str(search_term).strip().lower() in self.broad: return 1
	return 0

    def setText(self,temptext,textdata):
	text_buffer_set_text(temptext,textdata)

    def doQuit(self,*args) :
	self.closeDatabaseConnections()
	pythonol_setup.savePreferences()
	mainquit()

    def showMessage(self,message_text):
	msg_info(to_utf8(filter("Pythoñol ")+self.pversion),to_utf8(filter(str(message_text))))

    def showConfirm(self,message_text):
	return msg_confirm(to_utf8(filter("Pythoñol ")+self.pversion),to_utf8(filter(str(message_text))), "Yes","No")

    def askConjugateConfirm(self,message_text):
	gres=dialog_message(to_utf8(filter("Pythoñol ")+self.pversion),to_utf8(filter(str(message_text))).split("\n"),('Conjugate This Verb!','Find Better Matches','Cancel',),3,1)
	if gres=='Conjugate This Verb!': return 1
	if gres=='Find Better Matches': return 2
	return 3

    def askVocabConfirm(self):
	gres=dialog_message(to_utf8(filter("Pythoñol ")+self.pversion), to_utf8( "You can create random vocabulary lists of different sizes.\n\nGenerating larger lists may take a short moment.\nHow many words do you want in your vocabulary list?" ).split("\n"), ('10 Words','25 Words','50 Words','100 Words','Cancel',) ,3,1)
	if gres=='10 Words': return 10
	if gres=='25 Words': return 25
	if gres=='50 Words': return 50
	if gres=='100 Words': return 100
	return -1

    def askSearchConfirm(self):
	gres=dialog_message(to_utf8(filter("Pythoñol ")+self.pversion), to_utf8( filter("Pythoñol was unable to conjugate the term you entered.\nHowever, the dictionaries can be searched for the closest\nmatching verbs. This may take a moment.\n\nWhich dictionaries would you like to search?") ).split("\n"), ('English->Spanish','Spanish->English','BOTH','Cancel Search',) ,3,1)
	if gres=='English->Spanish': return 1
	if gres=='Spanish->English': return 2
	if gres=='BOTH': return 3
	return 4

    #####  HELP SECTION  ####


    def showPronounceHelp(self,*args):
	self.openHelp(27)

    def showVerbBrowserHelp(self,*args):
	self.openHelp(266)

    def showVerbFamHelp(self,*args):
	self.openHelp(267)

    def runBabHelp(self,*args):
	self.openHelp(268)

    def showIdiomHelp(self,*args):
	self.openHelp(25)

    def showChildHelp(self,*args):
	self.openHelp(22)

    def showConjugateHelp(self,*args):
	self.openHelp(23)

    def showDictHelp(self,*args):
	self.openHelp(24)

    def showThesHelp(self,*args):
	self.openHelp(26)

    def showSlangHelp(self,*args):
	if self.mature_cursor==None: 
		showHelp("Sorry. The Spanish slang data file is not available on your system or could not be loaded. You appear to be running the 'child-friendly' version of Pythoñol.  The Spanish slang dictionary is NOT available in the 'child-friendly' version of Pythoñol.","Pythoñol: Spanish Slang")
		return
	ich=pythonol_help.pyhelpui(1,0,0,None,[],1)
	getSQL(self.mature_cursor,"SELECT spanish,english from definitions ORDER by spanish")
	for ii in self.mature_cursor.fetchall():
		text_area_insert(ich.helptext, self.color2, filter(ii[0])+" : ")
		text_area_insert(ich.helptext, self.color4, "["+filter(ii[1])+"]\n")
	ich.icehelp.set_title(to_utf8(filter("Pythoñol Spanish Slang")))
	ich.icehelp.set_default_size(570,375)
	ich.icehelp.show_all()

    def openHelp(self,*args):
	pythonol_help.openHelp(args[0])

    def openHelpTopics(self,*args):
	pythonol_help.helpbrowser()

    def openHelpRC(self,*args):
	pythonol_help.openHelp(args[0],1)
    #####  END HELP SECTION  ####

    ##### PRINT SECTION  ######
    def switchPage(self,*args):
	if len(args)>2: self.print_mon=args[2]

    def openPrintDialog(self,*args):
	if use_filter==1:  # We are running Windows, no printing avail.
		self.showMessage("SORRY: Printing is not available in the Windows version of Pythoñol.\nPrinting is only supported in the LINUX / UNIX version of Pythoñol.\nKeep in mind that Pythoñol is a LINUX application ported to Windows.\n\nHowever, you may click 'Save To File' in whatever section you are viewing.\nThen, open the saved file and print from another program, like Notepad.\nThere are no immediate plans to support printing under Windows at this time.")
		return
	if self.print_mon==-1: return
	tarea=[]
	if self.print_mon==0: tarea=[self.conjtext]
	if self.print_mon==1: tarea=[self.babwin.babtext1,self.babwin.babtext2]
	if self.print_mon==2: tarea=[self.dicttext]
	if self.print_mon==3: tarea=[self.thestext]
	if self.print_mon==4: tarea=[self.idtext]
	if self.print_mon==5: return  # dont print pronunciation tab
	if self.print_mon==6: tarea=[self.browseverbtext]
	if self.print_mon==7: tarea=[self.verbfamtext]
	if self.print_mon==8: tarea=[self.abouttext]
	self.printTextArea(tarea)	

    def printTextArea(self,text_widgets) :
	ss=""
	for mm in text_widgets:
		if len(ss)>0: ss=ss+"\n\n__________________\n\n"+text_buffer_get_text(mm)
		else: ss=ss+text_buffer_get_text(mm)
	ss=ss.strip()
	if not len(ss)>0:
		self.showMessage("There is nothing to print in this window!")
		return
	tempfile=pyprint.makeTempFile(ss)
	if not tempfile==None:
		printw=pyprint.pyprintwin()
		printw.setFile(tempfile)
	else:  self.showMessage("ERROR: Printing failed.\nCould not create a temporary file in the /tmp/ directory for printing.")

    #####  END PRINT SECTION  ####
    def doVocabList(self,*args):
      try:
	vcount=self.askVocabConfirm()
	if vcount==-1: return
	qdict={}
	phelp=pythonol_help.pyhelpui(1,0,1,self.saveTextArea)
	phelp.icehelp.set_title(to_utf8(filter("Pythoñol Vocabulary List")))
	dbs=self.all_dicts
	mature_objs=0
	while len(qdict)<vcount:
		if len(dbs)==0: break
		choosedb=dbs[random.randrange(len(dbs))-1]
		if choosedb==self.mature_cursor:
			# make sure the num. of 'mature' words in a list never exceeds 20%
			if ((mature_objs*1.0)/vcount)>0.20: continue
		counter=random.randrange(self.getDatabaseCount(choosedb)-4)
		#print str(counter)
		query=getSQL(choosedb,"SELECT spanish,english from definitions LIMIT "+str(counter)+","+str(3))
		for dbrow in choosedb.fetchmany(3):
			#print "rows: "+str(len(choosedb.fetchall()))
			pww=dbrow[0]
			pwwdef=dbrow[1]
			if pww.find("[")>0: pww=pww[0:pww.find("[")].strip()
			if pww.find("(")>0: pww=pww[0:pww.find("(")].strip()
			if pww.find("/")>0: pww=pww[0:pww.find("/")].strip()
			if (len(pww)>2) and (len(pww)<25):
				if choosedb==self.mature_cursor:
					# make sure the num. of 'mature' words in a list never exceeds 20%
					mature_objs=mature_objs+1
					if ((mature_objs*1.0)/vcount)>0.20: continue
				qdict[pww]=pwwdef
				if len(qdict)==vcount: break
			if  len(qdict)==vcount: break
	text_area_insert(phelp.helptext, self.color4, "VOCABULARY LIST  ("+str(vcount)+" words)\n\n")
	lq=qdict.keys()
	lq.sort()
	for mm in lq:
		text_area_insert(phelp.helptext, self.color2, filter(mm)+"\t\t")
		text_area_insert(phelp.helptext, self.color4, filter(qdict[mm])+"\n")
	phelp.icehelp.show_all()
      except:
	pass

    def doVocabQuiz(self,*args):
	pyquiz.quizwin(self.all_dicts,getIconButton,self.getDatabaseCount)

    def doTransQuiz(self,*args):
	pytrans.transwin(getIconButton)

    def getThesaurusMatches(self,*args):   
      try:
	if len(self.thes_entry.get_text().strip())==0: return
	searchw=self.thes_entry.get_text().strip().lower()
	matches=[]
	span=1
	eng=0
	if self.thesradio2.get_active()==1: 
		span=0
		eng=1
	span_check=[]
	if span==1: 
	    	for hh in self.all_dicts:
			dex=getExactDefinition(hh,filter(searchw),0,span,eng)  # convert to english then lookup
			if len(dex)>0: 
				cando=1
				if pythonol_setup.child_friendly==1:
					for gg in banned_for_kids:
						if dex[0][0].lower().find(gg)>-1: 
							cando=0
							break
				if cando==1: 
					dexpart=dex[0][0]
					if dexpart.find("[")>-1: dexpart=dexpart[0:dexpart.find("[")].strip()
					if dexpart.find("/")>-1: dexpart=dexpart[0:dexpart.find("/")].strip()
					if len(dexpart)>2: 
						if not dexpart in span_check: span_check.append(dexpart)
					break
	    	#print "span check: "+str(span_check)
		thes_match={}
		for ss in span_check:
				tres=pythesaurus.thesaurusLookup(ss,self.thes_cats_cursor)
				#print "tres: "+str(tres)
				for gg in tres:
					thes_match[gg]="1"

		#print "thes_match:  "+str(thes_match.keys())
		for hh in thes_match.keys():
					for cursor in self.all_dicts:
						dex=getExactDefinition(cursor,hh,0,0,1)
						if len(dex)>0: 
							mymatch=dex[0][0]+"    ("+hh+")"	
							if not mymatch in matches: matches.append(mymatch)
							break

	# use the thesaurus module
	else:  # english
		thes_match={}
		tres=pythesaurus.thesaurusLookup(searchw,self.thes_cats_cursor)
		for mm in tres:
			cando=1
			if child_friendly==1:
				for gg in banned_for_kids:
					if mm.lower().find(gg)>-1: 
							cando=0
							break
			if cando==1: thes_match[mm]="1"			
		for hh in thes_match.keys():
					for cursor in self.all_dicts:
						dex=getExactDefinition(cursor,hh,0,0,1)
						if len(dex)>0: 
							mymatch=hh+"    ("+dex[0][0]+")"	
							if not mymatch in matches: matches.append(mymatch)
							break

	self.setText(self.thestext,"")
	matches.sort()
	if len(matches)==0:
		self.setText(self.thestext,"Sorry, no thesaurus matches found for '"+searchw+"'.  Be sure that you have selected the correct language above.  You may search either the Spanish or English thesaurus.")
                if use_filter==1: self.setText(self.thestext,"Sorry, no thesaurus matches found.  Be sure that you have selected the correct language above.  You may search either the Spanish or English thesaurus.")
        	closem=[]
		if span==0: closem=pythesaurus.getClosestMatches(searchw,self.thes_cats_cursor)
		if len(closem)>0:
			text_area_insert(self.thestext, self.color3, "\n\nThe closest matches in the thesaurus are:\n")
			for tt in closem:
				text_area_insert(self.thestext, self.color2, "        "+filter(tt)+"\n")
	else:
		text_area_insert(self.thestext, self.color1, " Thesaurus matches for '"+filter(searchw)+"':\n\n")
		for yy in matches:
			text_area_insert(self.thestext, self.color3, "    "+yy.replace("/",", ").replace(";",", ")+"\n\n")

	self.thes_history.insert(0,filter(self.thes_entry.get_text().strip()))
	if len(self.thes_history)>25: self.thes_history=self.thes_history[0:20]
	self.thescombo.set_popdown_strings(self.thes_history)
      except:
	pass


### ABOUT ###
    def showAbout(self,*args):
	text_area_insert(self.abouttext, self.color2, filter("\nPythoñol, PyBabelPhish, and PyPrint\nCopyright (c) 2002-2004 Erica Andrews\n\n"))
	text_area_insert(self.abouttext, self.color1, filter("AUTHOR:\nErica Andrews\nPhrozenSmoke@yahoo.com\nhttp://pythonol.sourceforge.net/\n\n"))

	text_area_insert(self.abouttext, self.color3, filter("Pythoñol and PyBabelPhish are distributed free-of-charge, as open source, under the terms of the Pythoñol Free Education Initiative License (FEI). Any use, copying, modification, or distribution of this software not in accordance with the FEI is strictly prohibited.  See 'License' on the 'Help' menu for the full details of the license, or read the 'LICENSE' files in the Pythoñol documentation  directory.\n\nPythoñol's features:  Text-to-speech pronunciation of words, phrases, and sentences (Linux/Unix only), Verb conjugation of 28 verb tenses (with translation), English-Spanish dictionary, English and Spanish thesauri, full-text translation, thousands of searchable idioms, a verb browser, extensive help (with animations), and an extensive grammar reference section.\n\nPythoñol is a Gtk-based, Linux application written in mostly Python and does NOT require Gnome.  It's even somewhat cross-platform.  Pythoñol is my attempt to make a free, much-needed, all-in-one Spanish-English language educational software package for Linux.  I created it, initially, for my personal use after becoming frustrated and disenchanted with a number of unstable, incomplete, and often, abandoned, Spanish language programs for Linux.  Over time, I started adding features that would not only be useful to myself, but others wishing to further their knowledge of Spanish as well.\n\nPythoñol is intended to provide at least SOME benefit to people of all ages and skill levels.  The use of a 'Child-Friendly' version of Pythoñol is strongly advised for users under the age of 18 or anyone else who may be sensitive to offensive content in the Pythoñol knowledge databases within the application.  See  'Making Pythoñol Child-Friendly' on the 'Help' menu for a quick, painless way to make Pythoñol safe and friendly for minors and other sensitive users.  Of course, truly concerned parents should monitor their child's use of Pythoñol.\n\nIf you are a teacher or instructor using this software for your students, I would LOVE to hear from you.  If you are using Pythoñol to teach students, please email me and let me know.  I would be very pleased to know if some student somewhere is benefitting from Pythoñol.  Suggestions, comments, and corrections are always welcome, especially from Spanish teachers/instructors and other linguistics experts.\n\nNOTE: Pythoñol is a LINUX application.  It was initially designed for Linux and will always be a Linux/Unix application FIRST.  However, a ported version of Pythoñol has been created for Windows.  Nevertheless, Windows users should NOT expect the same stability as the Linux version, and some features are missing in the Windows version (most notably, printing and text-to-speech pronunciation support.)    Quite frankly, it's unlikely that I will put the same effort into the Windows version as I do the Linux version.  I don't really use Windows anymore (I emancipated myself from MicroSlave Windows a long time ago.)  and there is enough software for Windows, provided you're willing to borrow out of your life insurance to be able to purchase it.  Somebody I care about asked me for a Windows version, so I made one.  However, Pythoñol for Windows has NOT been as extensively tested as Pythoñol for Linux.  So, Windows users: Please do not expect complete perfection and stability, as Gtk is made for Linux/Unix, not Windoze.\n\nThis product includes software developed by the PySQLite Sourceforge Team.\n\nPythoñol uses the following third-party software:\n\tSQLite : http://www.sqlite.org/\n\tPySQLite : http://pysqlite.sourceforge.net\n\tFestival : http://www.cstr.ed.ac.uk/projects/festival/\n (depending which version of this software you run)\n\n"))


#########
class loaderwin (Window) :
    def __init__ (self,pythonol_inst) :
	if use_filter==1: 
		Window.__init__ (self,WINDOW_POPUP)
		# old work around bug fix for Gtk-1 on Windows
                # looks like we dont need this anymore and the
                # 'splash' screen works on windows as well
		#Window.__init__ (self)
		#set_window_icon(self,pythonol_icon)
	else: 
		Window.__init__ (self,WINDOW_POPUP)
	self.set_wmclass("pythonol-splash","Pythonol-Splash")
	self.realize()
	self.set_title (to_utf8(filter('Pythoñol: Loading....')))
	self.set_position (WIN_POS_CENTER)
	if use_filter==0: self.set_modal(1)
	pytheme.installTheme(self)
	vbox1 =VBox (0, 2)
	vbox1.set_border_width(4)
	pixlap=getImage(getPixDir()+"title.gif", to_utf8(filter('PYTHOÑOL'))  )
	vbox1.pack_start (pixlap, 0, 0, 0)
	versionlab = Label ("version "+pythonol_inst.pversion)
	vbox1.pack_start (versionlab, 0, 0, 0)
	popstatlabel = Label ( to_utf8(filter(pythonol_inst.popstat))) #status here
	pythonol_inst.popstatlabel = popstatlabel
	vbox1.pack_start (HSeparator(), 0, 0, 0)
	vbox1.pack_start (popstatlabel, 0, 0, 14)
	vbox1.pack_start (HSeparator(), 0, 0, 0)
	vbox1.pack_start (Label("Copyright (c) 2002-2004 Erica Andrews."), 0, 0,0)
	self.add(vbox1)
	self.set_default_size(410,-1)
	self.show_all()
#########
class searchwin(Window):
    def __init__(self,pythonol_inst,data_dict={}) :
	self.pyinst=pythonol_inst
	Window.__init__(self,WINDOW_TOPLEVEL)
	self.set_wmclass(WINDOW_WMC,WINDOW_WMN)
	set_window_icon(self,pythonol_icon)
	self.set_title( to_utf8(filter('Pythoñol Search Results')))
	self.set_position(WIN_POS_CENTER)
	self.realize()
	self.set_modal(1)
	vbox1=VBox(0,0)
	self.sel_def=""
	vbox1.pack_start(Label('I could not conjugate the word you entered.'),0,0,2)
	vbox1.pack_start(Label('Please select the best match from this list:'),0,0,2)
	searchscroll=ScrolledWindow()
	searchscroll.set_border_width(4)
	searchlist=CList(2,[' Verb',' Meaning'])
	#searchscroll.set_hadjustment(searchlist.get_hadjustment())
	searchlist.set_column_width(0,185)
	searchlist.set_column_width(1,580)
	searchlist.set_selection_mode(SELECTION_SINGLE)
	searchlist.column_titles_show()
	dkys=data_dict.keys()
	dkys.sort()
	inum=0
	for yy in dkys:
		searchlist.append([ to_utf8(filter(str(yy))), to_utf8(filter(str(data_dict[yy])))])
		searchlist.set_row_data(inum,yy)
		inum=inum+1
	self.searchlist=searchlist
	searchscroll.add(searchlist)
	searchlist.connect("select_row",self.select_row_callback,"")
	searchlist.connect("unselect_row",self.unselect_callback,"")
	vbox1.pack_start(searchscroll,1,1,0)
	hbox1=HBox(0,0)
	hbox1.set_border_width(6)
	cancelbutt=getIconButton (self,STOCK_CANCEL,'Cancel',1) 
	cancelbutt.connect("clicked",self.killit)
	hbox1.pack_start(cancelbutt,0,0,0)
	hbox1.pack_start(Label('     '),1,1,0)
	conjbutt=getIconButton (self,STOCK_OK,'CONJUGATE!',1) 
	conjbutt.connect("clicked",self.conjit)
	hbox1.pack_start(conjbutt,0,0,0)
	vbox1.pack_start(hbox1,0,0,0)
	searchscroll.set_size_request(390,275)
	self.add(vbox1)
	wtips={conjbutt:"Conjugate this verb.\nConjuga el verbo.",cancelbutt:"Cancel.\nCancelar"}
	setTips(wtips)
	self.show_all()

    def conjit(self,*args):
	try:
		if len(self.sel_def)>0:
			if self.sel_def.find("[")>0: self.sel_def=self.sel_def[0:self.sel_def.find("[")].strip()
			if self.sel_def.find("(")>0: self.sel_def=self.sel_def[0:self.sel_def.find("(")].strip()
			if self.sel_def.find("/")>0: self.sel_def=self.sel_def[0:self.sel_def.find("/")].strip()
			self.pyinst.conjugateVerb(self.sel_def)
	except:
		pass
	self.killit()

    def killit(self,*args):
	self.destroy()
	self.unmap()

    def select_row_callback(self,widget,row,column,*args):
	self.sel_def=widget.get_row_data(row)
	#self.sel_def=str(self.searchlist.get_text(row,0))

    def unselect_callback(self,*args):
	self.sel_def=""
#########
def showHelpFile(helpfile,wintitle="Pythoñol",wide=0,noedit=0):
	pythonol_help.showHelpFile(helpfile,wintitle,wide,noedit)

def showHelp(helptext,wintitle="Pythoñol",wide=0,noedit=0):
	pythonol_help.showHelp(helptext,wintitle,wide,noedit)


def getIconButton (iwin,picon,lab="?",both=0,frame=0) :  
        try:
	  b=Button()
	  if both==1:
	    h=HBox(0,0)
	    h.set_spacing(4)
	    h.set_border_width(2)
	    if frame==1: h.pack_start( loadScaledImage(picon,16,16,1) ,0,0,0)
            else: h.pack_start(loadPixmap(str(picon),None),0,0,0)
	    l=Label(str(lab))
	    l.set_alignment(0,0.5)
            h.pack_start(l,1,1,0)
            h.show_all()
	    if frame==1:
		h.set_spacing(2)
                if use_filter==1: h.set_spacing(1)
		b.destroy()
		b.unmap()
		f=Frame()
		h.set_border_width(1)
                if use_filter==1: pytheme.installTheme(l)
		f.add(h)
		f.show_all()
		return f
	    b.add(h)
	  else:
	    b.add(loadPixmap(str(picon),None))
          b.show_all()        
          return b
        except:
	  if frame==1: return Label(str(lab))
          return Button(str(lab))

def setTip(wwidget,tip_str):
	global MYTIPS
	MYTIPS.set_tip(wwidget,str(tip_str))

def setTips(widget_dict):
	for rrr in widget_dict.keys():
		setTip(rrr,to_utf8(widget_dict[rrr]))

def run() :
    threads_init()
    pythonolwin()	
    mainloop()

if __name__== "__main__" :
    run()
