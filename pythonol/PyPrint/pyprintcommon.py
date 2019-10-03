# -*- coding: ISO-8859-1 -*-

########################
# PyPrint / PySpool: Common Lib
#
# Copyright 2002-2004 
# Erica Andrews
# PhrozenSmoke ['at'] yahoo.com
# http://pythonol.sourceforge.net
#
# This software is 
# distributed under the 
# terms of the GNU 
# General Public License.
########################

import sys

try:
	import pygtk
	pygtk.require("2.0")
except:
	pass



# Some PyGtk2 path settings while we upgrade from PyGtk-1 to PyGtk2
try:
        sys.path=["/usr/lib/python2.2/site-packages/gtk-2.0"]+sys.path
        sys.path=["/usr/lib/python2.3/site-packages/gtk-2.0"]+sys.path

        # print str(sys.path)
	import gtk
	import gtk.gdk 
	GDK=gtk.gdk
	GTK=gtk
	from gtk import *
except:
	print "Error: You do not have PyGtk-2 properly installed."
	print "This application will not run without it."
	sys.exit(0)
	pass


def to_utf8(somestr):
	unistr = unicode(str(somestr),"iso8859-1")
	utfstr = unistr.encode("utf-8")
	return utfstr


import sys,os,string
from string import join


global PYPRINT_VERSION
PYPRINT_VERSION="0.3"

def launch(launch_command):
	os.popen(str(launch_command))

def showMessage(message_text,wintitle="PyPrint"):
	global PYPRINT_VERSION
	msg_info(str(wintitle)+" "+PYPRINT_VERSION ,str(message_text))

def showConfirm(message_text,wintitle="PyPrint"):
	global PYPRINT_VERSION
	return msg_confirm(str(wintitle)+" "+PYPRINT_VERSION ,str(message_text))

def showAbout(message_text,myappname="PyPrint"):
	showHelp(str(myappname)+" "+PYPRINT_VERSION+"\n\nCopyright (c) 2002-2004 Erica Andrews\nPhrozenSmoke@yahoo.com\nAll rights reserved.\n\n" +str(message_text) +"\n\n"+str(myappname)+" is open source under the General Public License (GPL).\nNO technical support will be provided for this application.\nEnjoy!","About "+myappname)

def readOSLines(os_popen_cmd):
	try:
		return list(os.popen(str(os_popen_cmd)).xreadlines())
	except:
		return []

def getPrinterNames() :
	xlines=readOSLines("lpstat -v")
	printers=[]
	for pp in xlines:
		pline=pp.strip()
		if pline.find("system for") > -1:
			if pline.find(":")> pline.find("system for"):
				printer_name=pline[pline.find("system for")+len("system for"):pline.find(":")].strip()
				if not printer_name=="all": printers.append(printer_name)
	return printers

def isOnPath(binary):
  if os.environ.has_key("PATH"):
    paths=os.environ["PATH"].split(":")
    for i in paths:
      p=i
      if not p.endswith("/"): p=p+"/"
      if os.path.exists(p+str(binary)):
        return 1
  return 0



def set_window_icon(mywindow,my_xpm, is_stock=0):
	if is_stock==1:
		try:
			tempmen=MenuItem()
			# steal the GTK stock icon for window frame
			mywindow.set_icon(tempmen.render_icon(my_xpm,ICON_SIZE_DIALOG))
			tempmen.destroy()
		except:
			pass
	try:
		mywindow.set_icon(GDK.pixbuf_new_from_xpm_data(my_xpm))
	except:
		pass


def getPixmapButton (picon,btext,windowval=None) :  
	b=Button()
	b.add(getPixmapVBox(picon,btext,windowval))
	b.show_all()        
	Tooltips().set_tip(b,btext)
	return b

MAIN_PATH=[""+sys.argv[0][0:sys.argv[0].rfind(os.sep)+1]]
def getBaseDir() :
	return os.path.abspath(MAIN_PATH[0])+os.sep

def loadImage(picon,windowval):
    try:
        p=Image()
	if (picon.startswith("gtk-")) :  # stock icon
		# hmmm, no way to really scale stock icons to a special size
		p.set_from_stock(picon,ICON_SIZE_BUTTON)
	else: p.set_from_file(getBaseDir()+str(picon).strip())
        p.show_all()
        return p
    except:
        return None

def getPixmapVBox(picon,btext,windowval):
        try:
	  p=loadImage(picon,windowval)
          if not p: raise TypeError
	  v=HBox(1,2)
          v.set_homogeneous(0)
	  v.set_spacing(3)
          v.pack_start(p,0,1,1)
          v.pack_start(Label(str(btext)),0,1,1)
          v.show_all()
          return v
        except:
          v=HBox(1,1)
          v.pack_start(Label(str(btext)),1,1,1)
          v.show_all()
          return v

def showHelp(helptext,wintitle="PyPrint"):
	ich=icehelp2()
	ich.setHelpText(helptext)
	ich.icehelp.set_title(wintitle)
	ich.icehelp.set_modal(1)
	ich.icehelp.show_all()

class icehelp2 :
    def __init__ (self) :
	icehelp = Window (WINDOW_TOPLEVEL)
	self._root = icehelp
	set_window_icon(icehelp,STOCK_DIALOG_INFO,1)
        icehelp.set_wmclass(HELP_WMC,HELP_WMN) 
	icehelp.set_title ("")
	icehelp.set_position (WIN_POS_CENTER)
        icehelp.set_size_request(400,375)
        icehelp.realize()
        icehelp.connect("destroy",self.hideh)
	self.icehelp = icehelp
	vbox1 = VBox (0, 0)
	vbox1.set_border_width (11)
	self.vbox1 = vbox1
	scrolledwindow1 = ScrolledWindow ()
	self.scrolledwindow1 = scrolledwindow1
	helptext =TextView ()
	helptext.set_editable (0)
        helptext.set_wrap_mode(WRAP_WORD)
	self.helptext = helptext
	scrolledwindow1.add (helptext)
	vbox1.pack_start (scrolledwindow1, 1, 1, 0)
	closebutt =  getPixmapButton (STOCK_CANCEL, "Close" ,1)
        closebutt.connect("clicked",self.hideh)
	self.closebutt = closebutt
	vbox1.pack_start (closebutt, 0, 0, 13)
	icehelp.add (vbox1)
	icehelp.connect("key-press-event", keyPressClose)



    def text_buffer_set_text(self,sctext,stuff):
	sctext.get_buffer().freeze_notify()
        sctext.get_buffer().set_text(to_utf8(stuff))
	sctext.get_buffer().thaw_notify()

    def setHelpText(self, helpstuff):
	self.text_buffer_set_text(self.helptext ,helpstuff.replace("\n\n","{LBREAK}").replace("\n" , " ").replace("{LBREAK}" ,"\n\n") )


    def hideh(self,*args):      
      self.icehelp.hide()
      self.icehelp.destroy()
      self.icehelp.unmap()
      return TRUE


pyprint_icon_xpm = [
"32 32 256 2",
"  	c None",
". 	c #161E54",
"+ 	c #4D7BCF",
"@ 	c #7EBA9A",
"# 	c #CACE4A",
"$ 	c #EAD272",
"% 	c #A2A6E0",
"& 	c #C6AA9A",
"* 	c #3A3E6E",
"= 	c #6E869E",
"- 	c #E2EAC2",
"; 	c #D2B6B6",
"> 	c #1E56B2",
", 	c #A6BAE2",
"' 	c #8686A6",
") 	c #C6D2A6",
"! 	c #FAE6B2",
"~ 	c #869692",
"{ 	c #EAE6E6",
"] 	c #E6D692",
"^ 	c #2A56A6",
"/ 	c #B6BACE",
"( 	c #BAD6B6",
"_ 	c #C6D6D2",
": 	c #3E5E92",
"< 	c #BE9E9E",
"[ 	c #72B6CA",
"} 	c #DAD2CA",
"| 	c #FAFAD8",
"1 	c #567AB2",
"2 	c #7A9ACA",
"3 	c #AEC2CE",
"4 	c #82AAEE",
"5 	c #6A86AE",
"6 	c #A6B4DA",
"7 	c #9498BE",
"8 	c #52567E",
"9 	c #F0EEDC",
"0 	c #C2D0D8",
"a 	c #769EE2",
"b 	c #2E4A8A",
"c 	c #CADEFE",
"d 	c #AEC6EE",
"e 	c #D6D6EE",
"f 	c #E7EFE2",
"g 	c #F0AEAA",
"h 	c #86AEC2",
"i 	c #9EA6D2",
"j 	c #565E96",
"k 	c #AED6DE",
"l 	c #D3E2FE",
"m 	c #96AECE",
"n 	c #F6F6FE",
"o 	c #C8CEF4",
"p 	c #627AA6",
"q 	c #12326A",
"r 	c #9EB0E4",
"s 	c #CED0EA",
"t 	c #FAFAE6",
"u 	c #7E86AA",
"v 	c #BEC2DE",
"w 	c #9AC2E2",
"x 	c #E28A6A",
"y 	c #9296C2",
"z 	c #BCC2EE",
"A 	c #424666",
"B 	c #5F679A",
"C 	c #B2BED6",
"D 	c #222236",
"E 	c #DCE8DA",
"F 	c #AEB2E2",
"G 	c #C6D2E2",
"H 	c #6282BA",
"I 	c #7E86C2",
"J 	c #8AAAE2",
"K 	c #DA968E",
"L 	c #E6EEFD",
"M 	c #9AA2D6",
"N 	c #DFE2FA",
"O 	c #EED6D6",
"P 	c #222A66",
"Q 	c #F2F6EA",
"R 	c #B6CAEE",
"S 	c #3C4A86",
"T 	c #B2BAEF",
"U 	c #A2C3FE",
"V 	c #7686C6",
"W 	c #76AAFE",
"X 	c #E2EEFE",
"Y 	c #5266A6",
"Z 	c #EEF2CA",
"` 	c #86AEF2",
" .	c #4E6A96",
"..	c #C6D2EE",
"+.	c #F2C282",
"@.	c #8E96C6",
"#.	c #CAD3F9",
"$.	c #7078B8",
"%.	c #CABEBE",
"&.	c #EE9A92",
"*.	c #878FC5",
"=.	c #F2F2FE",
"-.	c #464A7E",
";.	c #DAE6FE",
">.	c #D8DAEE",
",.	c #EECEC2",
"'.	c #6992DB",
").	c #7E82AA",
"!.	c #626A9E",
"~.	c #AECEFE",
"{.	c #C6C8D4",
"].	c #BDC9EE",
"^.	c #9EB6E6",
"/.	c #94B2EC",
"(.	c #777EBC",
"_.	c #B4BEFE",
":.	c #8E9CD2",
"<.	c #CED2EE",
"[.	c #9498B6",
"}.	c #8991CA",
"|.	c #EAEEFE",
"1.	c #BEDAF2",
"2.	c #A6AEEE",
"3.	c #D6A672",
"4.	c #BEC6FE",
"5.	c #769EEA",
"6.	c #D4DAF7",
"7.	c #CAC8DE",
"8.	c #8296BE",
"9.	c #B5CEFB",
"0.	c #8A99D6",
"a.	c #E2E6FB",
"b.	c #A6AADE",
"c.	c #92A6DE",
"d.	c #EAEAEA",
"e.	c #BED6FD",
"f.	c #2E324E",
"g.	c #C2C6F2",
"h.	c #262E62",
"i.	c #AFB6F6",
"j.	c #EAF2FE",
"k.	c #F2F2CE",
"l.	c #FAECCE",
"m.	c #9ABEFE",
"n.	c #F9FAFE",
"o.	c #8692BE",
"p.	c #A9CAFE",
"q.	c #B6D2FE",
"r.	c #B5BAF7",
"s.	c #A6AEE5",
"t.	c #7292D2",
"u.	c #929ADD",
"v.	c #DBDEFA",
"w.	c #BEC2F2",
"x.	c #666EB2",
"y.	c #90B7FC",
"z.	c #72769A",
"A.	c #CEDAE6",
"B.	c #D2DECA",
"C.	c #4C5686",
"D.	c #98B0E2",
"E.	c #F2F6FE",
"F.	c #8290D4",
"G.	c #9DA6E1",
"H.	c #686E9A",
"I.	c #AEB1D1",
"J.	c #E6E6F6",
"K.	c #4D517D",
"L.	c #7078C2",
"M.	c #76A6FE",
"N.	c #525A96",
"O.	c #D2A28A",
"P.	c #C6D6F2",
"Q.	c #FEF8BC",
"R.	c #9ABAF2",
"S.	c #82B2FE",
"T.	c #BECADE",
"U.	c #CED2F2",
"V.	c #2D3362",
"W.	c #A9B2E8",
"X.	c #969ED6",
"Y.	c #F6F6DA",
"Z.	c #3E4577",
"`.	c #CADAFE",
" +	c #E1EAFE",
".+	c #365282",
"++	c #3E5292",
"@+	c #B8BEEC",
"#+	c #868ED8",
"$+	c #C6CAF4",
"%+	c #7E86BE",
"&+	c #9EA2DA",
"*+	c #BAD2FD",
"=+	c #C6DAFE",
"-+	c #666A90",
";+	c #6E74A8",
">+	c #769ADA",
",+	c #B2B6CE",
"'+	c #6E7AAA",
")+	c #7AAAFE",
"!+	c #B4BAE6",
"~+	c #828AD0",
"{+	c #EEF2FD",
"]+	c #B6BEF9",
"^+	c #5A62A2",
"/+	c #A6C2F2",
"(+	c #4A528A",
"_+	c #7682C6",
":+	c #5E6292",
"<+	c #8A96C6",
"[+	c #A5BEED",
"}+	c #C2D6FE",
"|+	c #333B7A",
"1+	c #BEC6FA",
"2+	c #2E366E",
"3+	c #E7EAFA",
"4+	c #D6E6FE",
"5+	c #CED6FC",
"6+	c #BBC2FD",
"7+	c #666BA7",
"8+	c #D9DEEB",
"9+	c #8992D4",
"0+	c #969ED2",
"a+	c #C6CADE",
"b+	c #8397C7",
"c+	c #AAB2F1",
"d+	c #9AA2E2",
"e+	c #B2CAFA",
"f+	c #CED2FC",
"g+	c #969EE1",
"h+	c #EEEEFB",
"i+	c #9DA6E7",
"j+	c #AAAEDF",
"k+	c #CEDAD2",
"l+	c #F6F2DE",
"m+	c #CEDEFE",
"n+	c #868AC6",
"o+	c #767AAE",
"p+	c #6A6E9E",
"q+	c #BABEFE",
"r+	c #C6CAFE",
"s+	c #B0B6EA",
"t+	c #FEFED8",
"u+	c #58629A",
"v+	c #BAD6FE",
"w+	c #C8CEFC",
"x+	c #D2D6F7",
"y+	c #B6BAEE",
"z+	c #AEB2DA",
"A+	c #DAEAFE",
"B+	c #7A82BE",
"C+	c #444E88",
"D+	c #A8C6FC",
"E+	c #86B2FE",
"F+	c #A2AAEA",
"G+	c #C2CAFE",
"                                                                ",
"                                                                ",
"                                    M.p.*+9.p.S.W W             ",
"                                    y.l j.X L ;.m+m+c v+U y.    ",
"                                  m.c j.n.E.n E.|.3+l l D+`     ",
"                                E+U n.E.{+|.|.L L X  +A+[+'.    ",
"                                v+`.n {+{+j. +X ;.;.l c >+>     ",
"                              )+q.j.E.j. +l =+}+=+`.m+R.>+      ",
"                              m.=+{+L A+c e.q.*+e.}+=+'.+       ",
"                              e.;.{+ +`.*+~.9.e.}+e.e++         ",
"                            )+4+j. +;.q.e+q.*+}+}+e+5.          ",
"                    u.d+G.W.p.;. +l e.9.q.*+}+}+}+/++           ",
"                o <.!+i+g+d+, ].}+e.q.q.q.*+e.}+9.J ^           ",
"              $+e >.I.n+_+%+<+:.c.D.^., /+R }+}+m.t.b           ",
"            N E.n.a+* V.Z.j (.x.j N.Y '+<+6 P.P.D.V o+n+}.      ",
"          $+E.n n.3+{.7 ).H.-.h.h.2+|+-.C+C+C.!.;+o...>.U.y+u.  ",
"        j+@+3+{+E.E.n.>.{.I.[.' ;+:+C.|+P . . Z.).v 3+e z 2.d+(.",
"        s.s+x+N J.3+j.E.=.E.E.E.e I.y -+C+* V.-+/ h+N i.g+u.0.!.",
"        s+T g.]+T y+1+v.|.n n.n.n.n.{+n 3+s @.e h+a.T d+u.#+L.(+",
"        W.T w+j+X.&+!+z g.$+$+>. +{+n.n n |.8+J.h+v.c+g+~+~+d+N.",
"        F+c+4.M :.u.:.@.u.i W.g.o <.6.8+N 3+{+|.a.f+i+9+9+g+r.^+",
"        g+i+_.D.j+F <+F.*.y &+j+j+% s.T $+6.v.v.6.z u.F.i+q+r.B ",
"        ~+0.U X Y.9 N r+y+j+u.(.x.;+*.j+@+1+T T i.i+#+2.]+q+y+!.",
"        D.[+>.O g &.,.{ J.8+$+W.G.0._+}.s+U.o f+r+6+c+6+6+W.(.  ",
"        R v %.; K x $ ( k 1.a.v.G+G.Y $.b.f+#.5+f+r+6+6+s.}.7+  ",
"      *+A.} < & O.3.# @ [ w N {+..b+Z.!.@.]+6+r+G+4.]+2.*.!.    ",
"    U f t+l.g +.] ) ~ h c.I.%.`.b+++Z.K.-+*.c+r._.]+i.(.7+      ",
"M.y.=+Z Q.Q.! l.l+E 0 0 {.{.P.p :           A K.p+B+(.          ",
"M.4 D.3 _ E Z | t+Y.f d.8+C 5 .+            D f.8 ;+7+          ",
"    H './.9.R k+- Z Y.k.G 8.q                                   ",
"            1 '.y.e.E k+= q                                     ",
"                    2  ..+                                      "]




pyspool_icon_xpm = [
"32 32 256 2",
"  	c None",
". 	c #060A0A",
"+ 	c #4A5E66",
"@ 	c #7A8686",
"# 	c #9AAEAA",
"$ 	c #C6CED6",
"% 	c #2C3636",
"& 	c #EEEEEE",
"* 	c #366ABA",
"= 	c #8A929A",
"- 	c #AEBAB8",
"; 	c #5A5E62",
"> 	c #264696",
", 	c #E6E6E6",
"' 	c #4A565A",
") 	c #8692AA",
"! 	c #DADEE0",
"~ 	c #B2BEBA",
"{ 	c #727E82",
"] 	c #9A9EB2",
"^ 	c #1E1A4A",
"/ 	c #565E82",
"( 	c #224E92",
"_ 	c #D0CEEA",
": 	c #BEBEE2",
"< 	c #121E1E",
"[ 	c #2A5EA6",
"} 	c #6A7EAE",
"| 	c #3A4646",
"1 	c #929EBA",
"2 	c #6E727A",
"3 	c #B2B2B2",
"4 	c #DADADA",
"5 	c #465AAE",
"6 	c #9AA2B6",
"7 	c #BEBEBE",
"8 	c #625EBA",
"9 	c #527272",
"0 	c #627A7E",
"a 	c #2A324A",
"b 	c #32568A",
"c 	c #6A86AE",
"d 	c #C2C2C2",
"e 	c #52628A",
"f 	c #B6BACA",
"g 	c #1A1A46",
"h 	c #A2A6BA",
"i 	c #7A8696",
"j 	c #CECEE6",
"k 	c #4E5A5E",
"l 	c #5E666E",
"m 	c #AAAEBA",
"n 	c #263250",
"o 	c #3A5692",
"p 	c #747AA2",
"q 	c #0A1616",
"r 	c #363E6A",
"s 	c #DED6EA",
"t 	c #768AA2",
"u 	c #626AA2",
"v 	c #5682CA",
"w 	c #C2C6C6",
"x 	c #7682A6",
"y 	c #A6AED2",
"z 	c #3A3E66",
"A 	c #D2D2D2",
"B 	c #96A29E",
"C 	c #E6E2F2",
"D 	c #F6F6F6",
"E 	c #4A6E6E",
"F 	c #4A527E",
"G 	c #666A90",
"H 	c #8E92B6",
"I 	c #CED6EA",
"J 	c #5A6A6A",
"K 	c #868A9E",
"L 	c #BABEDE",
"M 	c #6A7AB6",
"N 	c #C6CAE6",
"O 	c #202A28",
"P 	c #726E96",
"Q 	c #567AAA",
"R 	c #C0C2DE",
"S 	c #DEDEEE",
"T 	c #3E6286",
"U 	c #3E4246",
"V 	c #3A6AA6",
"W 	c #5A6A76",
"X 	c #B2B2C6",
"Y 	c #9EAAAE",
"Z 	c #222A4E",
"` 	c #425292",
" .	c #9EA6CE",
"..	c #2A3650",
"+.	c #9E9EC2",
"@.	c #666E86",
"#.	c #5A726A",
"$.	c #3E4672",
"%.	c #A2A6CE",
"&.	c #C6CADA",
"*.	c #929ABA",
"=.	c #6272CE",
"-.	c #5E62B2",
";.	c #B2BED6",
">.	c #C6C6C7",
",.	c #566686",
"'.	c #869A96",
").	c #8E8EAE",
"!.	c #B2B6DA",
"~.	c #686E98",
"{.	c #C6CACA",
"].	c #A6A6D0",
"^.	c #D6D2E6",
"/.	c #C2C2E2",
"(.	c #6E7AD6",
"_.	c #A8AEAE",
":.	c #A29ECA",
"<.	c #828286",
"[.	c #565AC6",
"}.	c #161A16",
"|.	c #6A6A6E",
"1.	c #303A60",
"2.	c #D6D6ED",
"3.	c #D2D6DA",
"4.	c #1E2222",
"5.	c #8A9EA2",
"6.	c #CACEDE",
"7.	c #8EA6CA",
"8.	c #AEAED2",
"9.	c #DAD2EA",
"0.	c #BABACE",
"a.	c #323262",
"b.	c #62668E",
"c.	c #162622",
"d.	c #EAEAE9",
"e.	c #526672",
"f.	c #9292C2",
"g.	c #9C9EC8",
"h.	c #B2B6C2",
"i.	c #8286AA",
"j.	c #4E5676",
"k.	c #F2F2F2",
"l.	c #2A3A3E",
"m.	c #666AC4",
"n.	c #B2AED7",
"o.	c #999AC3",
"p.	c #B6B6DC",
"q.	c #868692",
"r.	c #A8AAAC",
"s.	c #46466E",
"t.	c #4A5E9E",
"u.	c #8282AA",
"v.	c #A6AAD0",
"w.	c #C6C6E2",
"x.	c #CACECE",
"y.	c #E2E2E2",
"z.	c #323A58",
"A.	c #62726E",
"B.	c #C0C2CE",
"C.	c #222252",
"D.	c #424E52",
"E.	c #62729A",
"F.	c #7E8EB2",
"G.	c #B6BECE",
"H.	c #0E1612",
"I.	c #E6DEEE",
"J.	c #969EC4",
"K.	c #D2D2EB",
"L.	c #D8DAEC",
"M.	c #B4B2D6",
"N.	c #BEBADA",
"O.	c #8A96A2",
"P.	c #CDCAE7",
"Q.	c #6E728A",
"R.	c #929AA2",
"S.	c #C6CAD2",
"T.	c #C2C6CE",
"U.	c #B6BAC4",
"V.	c #FAF6FA",
"W.	c #AAAAD1",
"X.	c #4E6266",
"Y.	c #7A7E86",
"Z.	c #727AAE",
"`.	c #3E3E6A",
" +	c #9E9EA2",
".+	c #526A6A",
"++	c #C2BEDE",
"@+	c #426EAA",
"#+	c #6676CE",
"$+	c #727292",
"%+	c #A4A2CE",
"&+	c #9EA2CE",
"*+	c #101A1A",
"=+	c #060E0C",
"-+	c #9296BE",
";+	c #BABADE",
">+	c #4A5E6E",
",+	c #66669A",
"'+	c #8A8EBA",
")+	c #DEDEDE",
"!+	c #DEDAEE",
"~+	c #CED2DA",
"{+	c #DEDAE2",
"]+	c #DAD6EC",
"^+	c #D6D2EC",
"/+	c #9CA2B0",
"(+	c #CCD2E6",
"_+	c #AAB2CE",
":+	c #2E3A52",
"<+	c #9CA2C0",
"[+	c #D6D6E6",
"}+	c #AEB2D6",
"|+	c #848AAE",
"1+	c #4E5A7C",
"2+	c #E6E2EE",
"3+	c #BAB6DD",
"4+	c #CAC6E3",
"5+	c #0A0E0B",
"6+	c #AAAAD6",
"7+	c #9A9ACA",
"8+	c #EAE6EA",
"9+	c #CACACB",
"0+	c #CECECE",
"a+	c #466262",
"b+	c #7A8A86",
"c+	c #869692",
"d+	c #224A96",
"e+	c #1E1E4A",
"f+	c #3E4A4E",
"g+	c #527676",
"h+	c #7E8A96",
"i+	c #A6B2BA",
"j+	c #36425E",
"k+	c #8E96BA",
"l+	c #566E76",
"m+	c #3E5692",
"n+	c #667282",
"o+	c #5A7672",
"p+	c #3A4A72",
"q+	c #5E76C6",
"r+	c #626A8A",
"s+	c #C2CAD2",
"t+	c #B6AED6",
"u+	c #D6D6D6",
"v+	c #9696BD",
"w+	c #C6C2E2",
"x+	c #E2DAEA",
"y+	c #929AC2",
"z+	c #A6AAD6",
"A+	c #C6C6EA",
"B+	c #969ECA",
"C+	c #AAB2D6",
"D+	c #EAE2F2",
"E+	c #E2DEEE",
"F+	c #AAA6D2",
"G+	c #B6B6C2",
"                                                                ",
"                                                                ",
"                                        d w w 7                 ",
"                                      A d >.9+9+A A 0+0+        ",
"                                    >.9+9+9+>.>.w >.9+0+>.7     ",
"                                    4 u+A A 0+0+9+0+9+9+3       ",
"                                    )+)+4 4 u+A 0+0+0+0+ +      ",
"                                    y.)+)+4 )+{+4 4 A >.        ",
"                                  , , d.y.y., )+)+)+A r.        ",
"                                k.k.& d.& d.d., , y.>.          ",
"                                D V.V.k.k.k.k.& d.)+d           ",
"                      j R.W 9 6 ! , k.V.V.D D k.& x.            ",
"                  I (+h % < < O l.D.; @ r.>.4 , , G+).          ",
"                2.L.$ Y.O *+=+=+q q *+c.% f+l b+m +.g.          ",
"                ^+!+[+h.= 2 k | 4.q . 5+=+q *+l.6 3+3+8.p.      ",
"              R (+!+E+x+[+6.s+U.B q.|.U H.=+< n+w.j _ P.4+].,+  ",
"            L 6+].W.N.R P.K.^.4+{+I.E+L.I U.f [+x+x+]+K.v.E.r   ",
"            w+n.:.-+*.o. .W.3+++w.j 3.]+!+I.C I.E+2.K.R Z.j.a.  ",
"            N n.J.x ) x K f.v+v+%+v.}+N.P.K.9.]+2.^+/.H j.$.C.  ",
"            w+6+_+_.5.J ' e.r+~.F.k+g.].W.t+n.t+;+p.v+e r ..g   ",
"              *.T.- # = #.+ e t.m+p+j.$+u.v+&+&+ .J.~.z :+a g   ",
"              Y '.B - ~ O.p m.[.` 1.j.K @.|+y }+!.F+P z.a n e+  ",
"            _.@ #.A.{ h+M q+#+(.m.u 6 X Q.i %.6+M.%+G z...Z     ",
"            0 9 o+e.+ T [ * v =.-.v.f   Z.'+B+g.%+v+b.j+..      ",
"        A Y 9 E a+k e b ( > > b G+++      b.*.C+].f.P $.        ",
"        ! 4 x.- '.t Q @+V 5 8 :.7                 v+P           ",
"                3.6.;.7.c } 6 U.                                ",
"                      {.s+s+s+                                  ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                "]



ICON_STOP     = 4 
ICON_ALERT    = 1
ICON_INFO     = 2
ICON_QUESTION = 3
ICON_ERROR    = 4


class _MessageBox(gtk.Dialog):
    def __init__(self, 
                 title="Message", 
                 message="", 
                 buttons=("Ok",),
                 pixmap=ICON_INFO, 
                 modal=gtk.TRUE):
        """
        message: either a string or a list of strings
        pixmap: either a filename string, or one of: ICON_STOP, ICON_ALERT,
                ICON_INFO or ICON_QUESTION, or None

        """
        gtk.Dialog.__init__(self)
	set_window_icon(self,STOCK_DIALOG_QUESTION,1)
	self.set_position(WIN_POS_CENTER)
        self.set_title(title)
        self.set_wmclass(DIALOG_WMC,DIALOG_WMN) # added, 4.2.2003 - Erica Andrews
        self.connect("destroy", self.quit)
        self.connect("delete_event", self.quit)
        self.connect("key-press-event", self.__press)
        self.set_modal(gtk.TRUE)
        hbox = gtk.HBox(spacing=5)
        hbox.set_border_width(10)
        self.vbox.pack_start(hbox)
        hbox.show()
        if pixmap:
            self.realize()
	    try:
            	pic = gtk.Image()
            	if type(icons[pixmap]) == type(""):
                	pic.set_from_stock(icons[pixmap],ICON_SIZE_DIALOG)
            	else:
                	p, m = gtk.gdk.pixmap_create_from_xpm_d(self.window, 
                                                        None, 
                                                        icons[pixmap])
                	pic.set_from_pixmap(p, m)
            	pic.set_alignment(0.5, 0.0)
            	hbox.pack_start(pic, expand=gtk.FALSE)
            	pic.show()
	    except:
		pass
        if type(message) == type(""):
            label = gtk.Label(message.replace("\n\n","{LBREAK}").replace("\n" , " ").replace("{LBREAK}" ,"\n\n"))
        else:
            label = gtk.Label(join(message, "\n"))
        label.set_justify(gtk.JUSTIFY_LEFT)
        label.set_line_wrap(1)  # added 4.4.2003 - for better readibility on low-res screens, Erica Andrews
        hbox.pack_start(label)
        label.show()
        if not buttons:
            buttons = ("OK",)
        tips=gtk.Tooltips()
        for text in buttons:
	    if text=="Cancel": b=getPixmapButton (STOCK_CANCEL, "Cancel" ,1)
	    elif text=='Cancel Search': b=getPixmapButton (STOCK_CANCEL, 'Cancel Search' ,1)
	    elif text=="OK": b=getPixmapButton (STOCK_OK, "OK" ,1)
	    elif text=="Yes": b=getPixmapButton (STOCK_YES, "Yes" ,1)
	    elif text=="No": b=getPixmapButton (STOCK_NO, "No" ,1)
            else: b=getPixmapButton (STOCK_APPLY, text.strip() ,1)
            tips.set_tip(b,text)
            if text == buttons[0]:
                # the first button will always be the default button:
                b.set_flags(gtk.CAN_FOCUS|gtk.CAN_DEFAULT|gtk.HAS_FOCUS|gtk.HAS_DEFAULT)
            b.set_flags(gtk.CAN_DEFAULT)
            b.set_data("user_data", text)
            b.connect("clicked", self.__click)
            self.action_area.pack_start(b)
            b.show()
        self.default = buttons[0]
        self.ret = None

    def quit(self, *args):
        self.hide()
        self.destroy()
        gtk.mainquit()

    def __click(self, button):
        self.ret = button.get_data("user_data")
        self.quit()

    def __press(self, widget, event):
        if event.keyval == gtk.keysyms.Return:
            self.ret = self.default
            self.quit()


def dialog_message(title="Message", message="", buttons=("Ok",),
            pixmap=ICON_INFO, modal=gtk.TRUE):
    "create a message box, and return which button was pressed"
    win = _MessageBox(title, message, buttons, pixmap, modal)
    win.show()
    gtk.mainloop()
    return win.ret


icon_alert = STOCK_DIALOG_WARNING
icon_stop = STOCK_DIALOG_ERROR
icon_info = STOCK_DIALOG_INFO
icon_question = STOCK_DIALOG_QUESTION


# changed, 4.2.2003 - Erica Andrews, added 'icon_stop' as 5th list element
icons = [ icon_stop, icon_alert, icon_info, icon_question, icon_stop ]



#############
# easy dialog callbacks
#############

DIALOG_OK="OK"
DIALOG_CANCEL="Cancel"
HELP_WMC="pyprint-help"  # wm_class for help windows
HELP_WMN="PyPrint-Help"  # wm_name for help windows
DIALOG_WMC="pyprint-dialog"  # wm_class for dialogs
DIALOG_WMN="PyPrint-Dialog"  # wm_name for dialogs

def msg_info(wintitle,message):
	if str(message).lower().find("error")>-1:
		msg_err(wintitle,message)
	else:
		dialog_message(wintitle,message.split("\n"),(DIALOG_OK,),2,1)

def msg_err(wintitle,message):
	dialog_message(wintitle,message.split("\n"),(DIALOG_OK,),4,1)

def msg_confirm(wintitle,message,d_ok="Yes",d_cancel="No"):
	ret=dialog_message(wintitle,message.split("\n"),(d_ok,d_cancel,),3,1)
	if str(ret)==d_ok: return 1
	else: return 0



### COMMON FILE SELECTIONS, borrowed from my IceWMCP Project

def keyPressClose(widget, event,*args):
    if event.keyval == gtk.keysyms.Escape:
        widget.hide()
        widget.destroy()
        widget.unmap()
    elif event.keyval == gtk.keysyms.Return:
        if widget.get_data("ignore_return")==None: # some windows shouldn't close on 'Return'
            widget.hide()
            widget.destroy()
            widget.unmap()


# added 6.18.2003 - common file selection functionality
PYPRINT_FILE_WIN=None
PYPRINT_LAST_FILE=os.getcwd()
FILE_SELECTOR_TITLE="Select a file..."

def CLOSE_FILE_SELECTOR(*args):
	   try:
		PYPRINT_FILE_WIN.hide()
		PYPRINT_FILE_WIN.destroy()
		PYPRINT_FILE_WIN.unmap()
	   except:
		pass

def GET_SELECTED_FILE(*args):
		gfile=PYPRINT_FILE_WIN.get_filename()
		global PYPRINT_LAST_FILE
		if not gfile==None: PYPRINT_LAST_FILE=gfile
		CLOSE_FILE_SELECTOR()
		return gfile

def SET_SELECTED_FILE(file_name):
		global PYPRINT_LAST_FILE
		PYPRINT_LAST_FILE=str(file_name)

def SELECT_A_FILE(file_sel_cb,title=FILE_SELECTOR_TITLE,wm_class=DIALOG_WMC,wm_name=DIALOG_WMN,widget=None,ok_button_title=None,cancel_button_title=None,ok_button_icon=STOCK_OPEN):
		global PYPRINT_FILE_WIN
		PYPRINT_FILE_WIN = FileSelection(title)
		set_window_icon(PYPRINT_FILE_WIN, STOCK_SAVE,1)
		PYPRINT_FILE_WIN.set_wmclass(wm_class,wm_name)
		PYPRINT_FILE_WIN.ok_button.connect('clicked', file_sel_cb)
		value = PYPRINT_LAST_FILE
		if not widget==None: 
			PYPRINT_FILE_WIN.ok_button.set_data("cfg_path",widget.get_data("cfg_path"))
			if value=='': value=widget.get_data("cfg_path").get_text()
		if not ok_button_title==None:
			PYPRINT_FILE_WIN.ok_button.remove(PYPRINT_FILE_WIN.ok_button.get_children()[0])
			PYPRINT_FILE_WIN.ok_button.add( 
				getPixmapButton (ok_button_icon, str(ok_button_title) ,1))			
		if not cancel_button_title==None:
			PYPRINT_FILE_WIN.cancel_button.remove(PYPRINT_FILE_WIN.cancel_button.get_children()[0])
			PYPRINT_FILE_WIN.cancel_button.add( 
				getPixmapButton (STOCK_CANCEL, str(cancel_button_title) ,1))			
		PYPRINT_FILE_WIN.cancel_button.connect('clicked', CLOSE_FILE_SELECTOR)
		PYPRINT_FILE_WIN.connect("destroy",CLOSE_FILE_SELECTOR)
		if value != '""':
			PYPRINT_FILE_WIN.set_filename(value)
		#print "Last File:  "+str(value)
		PYPRINT_FILE_WIN.set_data("ignore_return",1)
		PYPRINT_FILE_WIN.connect("key-press-event", keyPressClose)
		PYPRINT_FILE_WIN.set_modal(1)
                PYPRINT_FILE_WIN.set_position(WIN_POS_CENTER)
		PYPRINT_FILE_WIN.show_all()
