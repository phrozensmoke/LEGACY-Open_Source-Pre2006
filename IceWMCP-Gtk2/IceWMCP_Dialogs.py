#! /usr/bin/env python

#######################################
# IceMe - Message Boxes
# =====
#
# Copyright 2000-2002, Dirk Moebius <dmoebius@gmx.net> 
# and Mike Hostetler <thehaas@binary.net>
#
# This software is distributed under the GNU General Public License
#######################################
########
# With Modifications by Erica Andrews (PhrozenSmoke@yahoo.com), April 2003
# This is a modified version of the 'MessageBox.py" module 
#  from IceMe 1.0.0, optimized for IceWM ControlPanel.
# Copyright (c) 2003 Erica Andrews
#
#  Changes to the original IceMe message box code are noted 
#  throughout the source as comments.
##################
##################
# Ported to PyGTK-2 by: David Moore (djm6202@yahoo.co.nz) - March 2003
# Changed to use stock dialog icons
##################

from string import join
#from gtk import *
import gtk


ICON_STOP     = 4 # changed, 4.2.2003 - Erica Andrews, 'icon_stop' changed 
                  # from 1st to 5th list element

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
        self.set_title(title)
        self.set_wmclass("icewmcontrolpanel","IceWMControlPanel") # added, 4.2.2003 - Erica Andrews
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
            pic = gtk.Image()
            if type(pixmap) == type(""):
                pic.set_from_file(pixmap)
            else:
                p, m = gtk.gdk.pixmap_create_from_xpm_d(self.window, 
                                                        None, 
                                                        icons[pixmap])
                pic.set_from_pixmap(p, m)
            pic.set_alignment(0.5, 0.0)
            hbox.pack_start(pic, expand=gtk.FALSE)
            pic.show()
        if type(message) == type(""):
            label = gtk.Label(message)
        else:
            label = gtk.Label(join(message, "\n"))
        label.set_justify(gtk.JUSTIFY_LEFT)
        label.set_line_wrap(1)  # added 4.4.2003 - for better readibility on low-res screens, Erica Andrews
        hbox.pack_start(label)
        label.show()
        if not buttons:
            buttons = ("Ok",)
        tips=gtk.Tooltips()
        for text in buttons:
            b = gtk.Button(text)
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
#        elif event.keyval == gtk.keysyms.Escape:
#            self.quit()


def message(title="Message", message="", buttons=("Ok",),
            pixmap=ICON_INFO, modal=gtk.TRUE):
    "create a message box, and return which button was pressed"
    win = _MessageBox(title, message, buttons, pixmap, modal)
    win.show()
    gtk.mainloop()
    return win.ret


################################################################################
# some icons that can be used for the dialog
################################################################################

# changed 4.2.2003 - Erica Andrews:  new icon colors

icon_stop = [
"32 32 72 1",
"  c opaque",
". c #8b0000",
"X c #930000",
"o c #9b0000",
"O c #9c0a0a",
"+ c #a20000",
"@ c #a10b0b",
"# c #ab0000",
"$ c #aa0a0a",
"% c #ae1717",
"& c #b30000",
"* c #b70b0b",
"= c #bb0000",
"- c #b41414",
"; c #c40000",
": c #ca0000",
"> c #cd0909",
", c #ca1212",
"< c #d10000",
"1 c #d30b0b",
"2 c #dc0000",
"3 c #dd0909",
"4 c #c72929",
"5 c #cd2929",
"6 c #d62929",
"7 c #e40000",
"8 c #ec0303",
"9 c #ee1515",
"0 c #eb1f1f",
"q c #f20000",
"w c #fd0202",
"e c #f90808",
"r c #f51616",
"t c #f31818",
"y c #fe1111",
"u c #fe1c1c",
"i c #ec2b2b",
"p c #ed3f3f",
"a c #f52222",
"s c #f72c2c",
"d c #fd2222",
"f c #ff2828",
"g c #f23636",
"h c #f33c3c",
"j c #fe3333",
"k c #ff3f3f",
"l c #ec4444",
"z c #ff4545",
"x c #ff4a4a",
"c c #f75656",
"v c #f75c5c",
"b c #fe5555",
"n c #ff6767",
"m c #fc6b6b",
"M c #f57272",
"N c #fd7272",
"B c #fc7979",
"V c #ff8a8a",
"C c #fe9090",
"Z c #fd9c9c",
"A c #fea2a2",
"S c #feabab",
"D c #ffb1b1",
"F c #febcbc",
"G c #ffc4c4",
"H c #fecccc",
"J c #ffd4d4",
"K c #ffdbdb",
"L c #ffe1e1",
"P c #ffebeb",
"I c white",
"U c None",
"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU",
"UUUUUUUUUUU;;;;;=====UUUUUUUUUUU",
"UUUUUUUU:::;;::::;;==&&&UUUUUUUU",
"UUUUUUU::;:20vBBBBMi<;&&&UUUUUUU",
"UUUUUU:::2cDPPPPPPPPHM1&&#UUUUUU",
"UUUUU::<iAKKKKKKKKKKKKHl=##UUUUU",
"UUUU::2hFHHHHHHHHHHHHHHHM;#+UUUU",
"UUU:::hFFFHFFFFFFFFFFHFFFM=++UUU",
"UU:::9ZSSHILSSSSSASSLIHSSSl#++UU",
"UU:;2NZZFIIIKZZCZZZKIIIFZZV1o+UU",
"UU::aVVZIIIIIJVVVVJIIIIIZVVc#oUU",
"U:;2bBBBHIIIIIHBBHIIIIIHNbsr:ooU",
"U;;8nnnnnGIIIIIGGIIIIISsq8qq7ooU",
"U;;rbbbbbbFIIIIIIIIIIZ8qqqqq8+XU",
"U;:dxzzzzzzFIIIIIIIIZ8qqqq8qq#XU",
"U;:tjjjjjjjjDIIIIIIZqqq8qqqqq#XU",
"U;:rddddddddSIIIIIIZ8qqqq8qqq#XU",
"U=:eeyyeyyeSIIIIIIIIZ8qq8wqqq#XU",
"U=;wewwwwwZIIIIIIIIIIZwqw8wqwX.U",
"U==8wwwwwZIIIIIZZIIIIIZwwwwq7X.U",
"U=&2wwwwZIIIIIZwwZIIIIIZwwww;..U",
"UU&=eeejIIIIIAeeeeZIIIIIjeeeo.UU",
"UU&&3yyynIIIAyyyyyySIIImyyy>..UU",
"UU&&&tduuNISuuuuuuuuSINuuu9X..UU",
"UUU&#*dfffbffffffffffbfffaO..UUU",
"UUUU##,sjjjjjjjjjjjjjjjjj-..UUUU",
"UUUUU##*gzkkkkkkzkkkkkzg@..UUUUU",
"UUUUUU#+#6xxxxxxxxxxxz5X..UUUUUU",
"UUUUUUU++o$5lbbbbbbl4O...UUUUUUU",
"UUUUUUUU+ooooO-%%%O.....UUUUUUUU",
"UUUUUUUUUUUooXXXXX...UUUUUUUUUUU",
"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU"
]

# changed 4.2.2003 - Erica Andrews:  new icon colors

icon_alert = [
"34 34 220 2",
"   c None",
".  c #100F0F",
"+  c #000000",
"@  c #060606",
"#  c #0B0B0B",
"$  c #74736E",
"%  c #B4B19F",
"&  c #F9F1CC",
"*  c #F7EAA6",
"=  c #B3A767",
"-  c #736930",
";  c #454545",
">  c #EDEDEC",
",  c #FFFCF1",
"'  c #FEED91",
")  c #FEDF49",
"!  c #FCD828",
"~  c #FEDF46",
"{  c #FEE460",
"]  c #DFC330",
"^  c #3B3309",
"/  c #1E1E1E",
"(  c #020202",
"_  c #999998",
":  c #FFFFFB",
"<  c #FEEE99",
"[  c #FCD621",
"}  c #FCD416",
"|  c #FCD51B",
"1  c #FCDA30",
"2  c #856F0C",
"3  c #8E8A7A",
"4  c #FFFAE2",
"5  c #FEE258",
"6  c #FBD316",
"7  c #FAD116",
"8  c #F7D016",
"9  c #F6CF16",
"0  c #F9D016",
"a  c #FBD626",
"b  c #6C5A09",
"c  c #2B2A28",
"d  c #74715E",
"e  c #FFFBE3",
"f  c #FEE45F",
"g  c #8E760D",
"h  c #655509",
"i  c #635409",
"j  c #CFAE13",
"k  c #EDC815",
"l  c #FBD627",
"m  c #544708",
"n  c #4E4A38",
"o  c #FFFAE0",
"p  c #FEE771",
"q  c #675709",
"r  c #564908",
"s  c #D8B514",
"t  c #F3CC16",
"u  c #352D05",
"v  c #030303",
"w  c #15140F",
"x  c #F3EABB",
"y  c #FEEC8E",
"z  c #A1870F",
"A  c #3D3305",
"B  c #C1A211",
"C  c #E2BE14",
"D  c #D9B714",
"E  c #100E03",
"F  c #282727",
"G  c #020201",
"H  c #B8AD6E",
"I  c #FEF3BB",
"J  c #FCD51D",
"K  c #AE9210",
"L  c #3E3405",
"M  c #B29610",
"N  c #DAB814",
"O  c #FCD519",
"P  c #917A0D",
"Q  c #534C2A",
"R  c #FFF6CA",
"S  c #FCDE42",
"T  c #E7C114",
"U  c #181402",
"V  c #AC900F",
"W  c #D6B413",
"X  c #F2CB16",
"Y  c #FCD823",
"Z  c #F1CA16",
"`  c #3B3205",
" . c #171716",
".. c #090805",
"+. c #ECDE8C",
"@. c #FEEC8A",
"#. c #E8C315",
"$. c #282103",
"%. c #AB8F0F",
"&. c #D5B313",
"*. c #FCD61E",
"=. c #C4A511",
"-. c #73692E",
";. c #FFF5BD",
">. c #EAC515",
",. c #0F0D01",
"'. c #443906",
"). c #AD910F",
"!. c #504307",
"~. c #100F0E",
"{. c #0F0D07",
"]. c #F2E287",
"^. c #FEE772",
"/. c #251F03",
"(. c #5B4C08",
"_. c #B19510",
":. c #F5CE16",
"<. c #FCD622",
"[. c #CFAE12",
"}. c #736626",
"|. c #FEF3B5",
"1. c #F0C915",
"2. c #4E4207",
"3. c #5D4F08",
"4. c #B79910",
"5. c #DFBB14",
"6. c #5E5008",
"7. c #060503",
"8. c #ECD665",
"9. c #FEE873",
"0. c #7F6C0B",
"a. c #846E0B",
"b. c #BEA011",
"c. c #E3BF14",
"d. c #B99C10",
"e. c #4D4414",
"f. c #FEEE9A",
"g. c #A68C0F",
"h. c #171402",
"i. c #977E0D",
"j. c #C5A611",
"k. c #382F05",
"l. c #B29A24",
"m. c #2B2404",
"n. c #A0860E",
"o. c #CEAD12",
"p. c #816D0B",
"q. c #060501",
"r. c #F6D93E",
"s. c #0F0C01",
"t. c #4C4007",
"u. c #C6A712",
"v. c #332B06",
"w. c #FEE35D",
"x. c #957D0D",
"y. c #B99C11",
"z. c #DEBA14",
"A. c #FCD419",
"B. c #221D03",
"C. c #645409",
"D. c #FEE35B",
"E. c #6F5E0A",
"F. c #544608",
"G. c #CCAC12",
"H. c #E0BD14",
"I. c #705E0A",
"J. c #FCDD3B",
"K. c #CAAA12",
"L. c #EEC815",
"M. c #4D4007",
"N. c #615109",
"O. c #DDB914",
"P. c #362D05",
"Q. c #261F03",
"R. c #88720C",
"S. c #ECC615",
"T. c #DBB814",
"U. c #403605",
"V. c #443A06",
"W. c #A1880F",
"X. c #2B2504",
"Y. c #100D02",
"Z. c #715F0A",
"`. c #B79A10",
" + c #7B670B",
".+ c #191502",
"++ c #A2880F",
"@+ c #CBAB12",
"#+ c #5A4C08",
"$+ c #050400",
"%+ c #A78C0F",
"&+ c #534508",
"*+ c #473C06",
"=+ c #8F790D",
"-+ c #D0AF13",
";+ c #87710C",
">+ c #090801",
",+ c #86710C",
"'+ c #D1B113",
")+ c #BB9E11",
"!+ c #6E5B09",
"~+ c #312904",
"{+ c #BFA111",
"]+ c #E5C014",
"^+ c #E4C014",
"/+ c #C9A812",
"(+ c #73610A",
"_+ c #262103",
":+ c #110F02",
"<+ c #6E5C0A",
"[+ c #836E0C",
"}+ c #B89A11",
"|+ c #B79A11",
"1+ c #B59811",
"2+ c #B39710",
"3+ c #A88D0F",
"4+ c #7F6B0B",
"5+ c #372E05",
"6+ c #151102",
"7+ c #070707",
"                            . + + + + .                             ",
"                        @ + + + + + + + + @                         ",
"                      + + + + + + + + + + + +                       ",
"                    + + + # $ % & * = - # + + +                     ",
"                  + + + ; > , ' ) ! ~ { ] ^ + + +                   ",
"                / + ( _ : < [ } } } } | 1 [ 2 ( + /                 ",
"              + + + 3 4 5 } } 6 7 8 9 8 0 a 0 b + + +               ",
"            c + + d e f } } 6 8 g h i j k 9 l 8 m + + c             ",
"          + + + n o p } } } 8 q + + + r s k 0 ! t u + + +           ",
"          v + w x y } } } 6 z + + + + A B C 9 } [ D E + v           ",
"        F + G H I J } } } 7 K + + + + L M N t } O 6 P G + F         ",
"      + + + Q R S } } } } 7 T + + + + U V W X } } Y Z ` + + +       ",
"       .+ ..+.@.} } } } } 6 #.+ + + + $.%.&.X } } } *.=...+  .      ",
"    + + + -.;.[ } } } } } } >.,.+ + + '.).s X } } } | t !.+ + +     ",
"    ~.+ {.].^.} } } } } } } k /.+ + + (._.N :.} } } } <.[.{.+ ~.    ",
"  + + + }.|.J } } } } } } } 1.2.+ + + 3.4.5.8 } } } } O X 6.+ + +   ",
"  + + 7.8.9.} } } } } } } } X 0.+ + + a.b.c.0 } } } } } J d.7.+ +   ",
"  + + e.f.[ } } } } } } } } 9 g.+ + h.i.j.#.7 } } } } } J #.k.+ +   ",
"+ + + l.' } } } } } } } } } 0 c.+ + m.n.o.k 6 } } } } } } 7 p.+ + + ",
"+ + q.r.~ } } } } } } } } } 6 #.s.+ t.V &.X } } } } } } } [ u.q.+ + ",
"+ + v.w.O } } } } } } } } } } k /.+ x.y.z.9 } } } } } } } A.s B.+ + ",
"+ + C.D.} } } } } } } } } } } Z E.F.4.G.#.0 } } } } } } } } H.k.+ + ",
"+ + I.J.} } } } } } } } } } } t H.o.K.N L.6 } } } } } } } } C M.+ + ",
"+ + N.[ } } } } } } } } } } 6 O.P.Q.R.D S.0 } } } } } } } } T.U.+ + ",
"+ + V.0 } } } } } } } } } } 8 k.+ + + W.5.X 6 } } } } } } } o.X.+ + ",
"+ + Y.S.0 } } } } } } } } } t + + + + Z.[.S.6 } } } } } } 1.`.Y.+ + ",
"+ + +  +#.6 } } } } } } } } Z !.+ + .+++@+>.7 } } } } } 8 G.#++ +   ",
"  + + $+%+T 7 } } } } } } } t z.&+*+=+M &.1.6 } } } } t -+;+$++     ",
"  + + + >+,+'+L.6 } } } } } 0 S.&.=.B '+T 9 } } } :.z.)+!+>++ +     ",
"    + + + + ~+2 {+&.T :.6 } 6 9 k ]+^+S.:.9 L.z./+M (+_++ + +       ",
"      + + + + + :+L <+[+).M }+}+|+1+1+2+).3+4+h 5+:++ + + +         ",
"          + + + + + + + + >+6+6+6+6+6+6+>++ + + + + + +             ",
"              + + + + + + + + + + + + + + + + + + 7+                ",
"                    + + + + + + + + + + + + + +                     "
]

# changed 4.2.2003 - Erica Andrews:  new icon colors

icon_info = [
"32 32 127 2",
"   c opaque",
".  c #00001b",
"X  c #000030",
"o  c #002b66",
"O  c #004b9f",
"+  c #004ea6",
"@  c #0052ab",
"#  c #005cbc",
"$  c #1f72bd",
"%  c #005dc0",
"&  c #0062c3",
"*  c #006cca",
"=  c #0074cf",
"-  c #0876ce",
";  c #007bd3",
":  c #106ac5",
">  c #1775c3",
",  c #177bc6",
"<  c #177fc9",
"1  c #1a7ace",
"2  c #207dcf",
"3  c #2874c6",
"4  c #337dca",
"5  c #0681d5",
"6  c #0084d8",
"7  c #008add",
"8  c #0d8ddc",
"9  c #1d83c8",
"0  c #1394df",
"q  c #0093e2",
"w  c #238cca",
"e  c #2f85c0",
"r  c #2081d2",
"t  c #2b85d2",
"y  c #3384ce",
"u  c #3086d2",
"i  c #3d8bd4",
"p  c #3a95da",
"a  c #4189b8",
"s  c #4884b1",
"d  c #5284ac",
"f  c #5d81a4",
"g  c #408cd3",
"h  c #4a8cd2",
"j  c #4094d9",
"k  c #4d94d7",
"l  c #4899db",
"z  c #5296c0",
"x  c #5e99d7",
"c  c #54aee2",
"v  c #6c9bd0",
"b  c #7d9cc8",
"n  c #65a4dd",
"m  c #72a6dc",
"M  c #77a9dd",
"N  c #70b3d7",
"B  c #75ade0",
"V  c #75bde2",
"C  c #6dc6f1",
"Z  c #78c0ec",
"A  c #7dd2f6",
"S  c #8495b1",
"D  c #85aad4",
"F  c #8eb1d8",
"G  c #99a9c4",
"H  c #97badd",
"J  c #98b9dc",
"K  c #85b2e1",
"L  c #95bce5",
"P  c #9abfe6",
"I  c #a2aec3",
"U  c #a5b1c4",
"Y  c #a3bad8",
"T  c #aabcd5",
"R  c #aebedb",
"E  c #81c6e7",
"W  c #87ccef",
"Q  c #8ac0e0",
"!  c #8bc3ea",
"~  c #88cded",
"^  c #8fd1f3",
"/  c #9bc0e2",
"(  c #99d1ed",
")  c #b5c3dc",
"_  c #b9c6df",
"`  c #bcc9dd",
"'  c #a4c5e9",
"]  c #aecbeb",
"[  c #acd7f2",
"{  c #b5c5e0",
"}  c #b3cfec",
"|  c #bdcae2",
" . c #bfd7ef",
".. c #bfd9f0",
"X. c #bce8fa",
"o. c #c3cdde",
"O. c #c1cee4",
"+. c #c4d0e5",
"@. c #c4d1e8",
"#. c #ccd3e2",
"$. c #cad6ea",
"%. c #ced9ec",
"&. c #c2d8f0",
"*. c #cedaf0",
"=. c #d2d6e1",
"-. c #d4d9e3",
";. c #d2dcec",
":. c #d3def0",
">. c #d9e0ee",
",. c #d5e3f3",
"<. c #dbe4f4",
"1. c #dbe6f8",
"2. c #dce9f4",
"3. c #deeaf9",
"4. c #e5e7ed",
"5. c #eaeaee",
"6. c #e0e7f6",
"7. c #e1e9f6",
"8. c #e3ebf9",
"9. c #e9ecf0",
"0. c #e8eefb",
"q. c #ebf0f4",
"w. c #ebf1fc",
"e. c #f0f6fe",
"r. c #f4f9fe",
"t. c #fefeff",
"y. c None",
"y.y.y.y.y.y.y.+.o.+.+.+.+.+.+.+.+.+.+.+.+.+.G y.y.y.y.y.y.y.y.y.",
"y.y.y.y.y.y.y.+.r.t.t.t.t.t.t.t.t.t.r.w.7.<.O.U y.y.y.y.y.y.y.y.",
"y.y.y.y.y.y.y.+.t.t.t.t.t.t.t.t.t.r.e.w.7.;.O.w.I y.y.y.y.y.y.y.",
"y.y.y.y.y.y.y.+.t.t.t.t.t.t.t.t.t.t.r.w.8.<.O.e.3.I y.y.y.y.y.y.",
"y.y.y.y.y.y.y.+.t.t.t.t.t.t.t.t.t.t.t.e.w.<.O.t.0.1.I y.y.y.y.y.",
"y.y.y.y.y.y.y.+.t.t.t.t.t.t.t.t.t.t.r.e.w.2.O.t.t.w.3.U y.y.y.y.",
"y.y.y.y.y.y.y.+.t.t.t.t.t.t.t.t.t.t.t.r.w.8.| e.w.7.<.%.G y.y.y.",
"y.y.y.y.y.y.f T >.>.-.r.t.t.t.t.t.t.t.r.e.0.$.G G G G G G G y.y.",
"y.y.y.y.f z N ~ X.X.A V Z 2.t.t.t.t.t.r.r.0.0.7.:.O.) ) R S   y.",
"y.y.y.d ! 3.0.w.0.w.w.q.w.W ( t.t.t.t.t.r.0.8.7.$.| | | ) S   y.",
"y.y.a ! ,.,.,.,.w.w.2.,.1.,.[ C t.t.t.r.r.0.0.:.$.O.O.O.| S   y.",
"y.d c &.&.&.&.,.t.t.w...&.&.&.V ^ t.t.t.t.0.:.;.$.$.#.O.@.S   y.",
"f w ! ] ] ] ] &.t.t.w.] ] ] ] ] q 3.t.r.0.2.:.;.;.$.$.$.$.S   y.",
"s 7 / / P P P } t.t.0.! P P P P 0 c e.w.2.<.<.:.:.:.;.;.%.S   y.",
"e 7 B K K K K ' t.t.7.F F K K K 7 8 >.8.7.6.<.<.:.:.:.;.:.S   y.",
"9 6 p m m m m L t.t.2.m m m m l 6 6 / 8.8.8.7.6.6.1.:.<.,.S   y.",
"9 ; ; k x x v K t.t.,.x x v k ; 5 ; H 0.0.7.7.3.1.2.<.<.:.S   y.",
"< ; ; = t h g M t.t.,.h h u ; ; ; ; J 7.0.0.0.7.8.1.<.2.1.S   y.",
"> = = = = - 1 i 3.t.n 1 - = = = = = J 7.w.0.0.8.8.7.6.6.1.S   y.",
": = * = * = * * r j * * = = * = * * o.7.w.0.0.0.8.7.6.6.3.S   y.",
"$ * & * * * * 2 t.t.K * * * & * * y #.e.w.w.0.8.8.7.6.6.7.S   y.",
"f : & & & & & i t.t...& & * & & & F -.e.w.w.0.0.8.8.8.8.3.S   y.",
"y.# & # % & & : ]  .4 & & # # # 4 #.9.e.e.w.w.0.0.8.7.8.3.S   y.",
"y.y.@ % % % % % % % % % % % & 3 o.-.e.e.w.w.w.0.8.8.1.8.3.S   y.",
"y.y.y.+ # % % % # & % % % % 4 o.-.w.e.e.w.w.w.0.8.8.8.7.1.S   y.",
"y.y.y.X o + # % # # % % 3 D =.-.w.e.e.e.w.w.w.8.8.8.8.7.3.S   y.",
"y.y.y.y.y.. . b D v Y ` -.-.4.r.r.r.e.e.w.w.w.8.8.8.7.6.3.S   y.",
"y.y.y.y.y.y.  ` 5.5.4.9.q.r.r.r.r.e.e.e.w.w.0.0.8.7.8.6.3.S   y.",
"y.y.y.y.y.y.y.+.r.t.t.r.t.r.r.r.r.e.e.e.w.w.w.8.0.7.7.7.3.S   y.",
"y.y.y.y.y.y.y.+.S S S S S S S S S S S S S S S S S S S S S S   y.",
"y.y.y.y.y.y.y.y.                                              y.",
"y.y.y.y.y.y.y.y.y.y.y.y.y.y.y.y.y.y.y.y.y.y.y.y.y.y.y.y.y.y.y.y."
]

# changed 4.2.2003 - Erica Andrews:  new icon colors

icon_question = [
"32 32 117 2",
"   c opaque",
".  c #660089",
"X  c #69008d",
"o  c #6d0092",
"O  c #700096",
"+  c #73009a",
"@  c #760a9a",
"#  c #7a00a3",
"$  c #7e00a8",
"%  c #8100ac",
"&  c #820aaa",
"*  c #8500b2",
"=  c #8900b6",
"-  c #8d00bb",
";  c #9926bf",
":  c #9200c3",
">  c #9600c8",
",  c #9800ca",
"<  c #980ec5",
"1  c #9b0bca",
"2  c #9e00d3",
"3  c #9d22c5",
"4  c #a101d5",
"5  c #a300da",
"6  c #a30ed3",
"7  c #a411d4",
"8  c #a419d2",
"9  c #ac1bdc",
"0  c #a900e1",
"q  c #af01e8",
"w  c #ae0de2",
"e  c #b104e9",
"r  c #b600f2",
"t  c #b800f5",
"y  c #bd00fc",
"u  c #ba0af3",
"i  c #b31fe4",
"p  c #b718eb",
"a  c #bb10f3",
"s  c #a620d1",
"d  c #aa22d7",
"f  c #ac23da",
"g  c #ab2fd4",
"h  c #aa3dce",
"j  c #b02fda",
"k  c #b53bdd",
"l  c #bc2fea",
"z  c #bf20f3",
"x  c #bf3dea",
"c  c #b64ad9",
"v  c #b352d3",
"b  c #b852d9",
"n  c #c107fe",
"m  c #c20efd",
"M  c #c413fe",
"N  c #c71ffe",
"B  c #c721fe",
"V  c #c42df6",
"C  c #c62ff8",
"Z  c #ca2bfe",
"A  c #c330f3",
"S  c #c73df4",
"D  c #cc34fe",
"F  c #c838f7",
"G  c #c247ea",
"H  c #c348eb",
"J  c #c944f5",
"K  c #ce47fa",
"L  c #cf57f6",
"P  c #d044fe",
"I  c #d24dfe",
"U  c #d456fd",
"Y  c #d55bfe",
"T  c #cc6fea",
"R  c #d462f9",
"E  c #d26bf4",
"W  c #d66df8",
"Q  c #d866fe",
"!  c #d969fe",
"~  c #d775f8",
"^  c #db73fe",
"/  c #dd7bfd",
"(  c #c381d8",
")  c #cb82e3",
"_  c #d08be7",
"`  c #d38fea",
"'  c #d881f4",
"]  c #db80f9",
"[  c #d491ea",
"{  c #d89dec",
"}  c #dc9df2",
"|  c #ddaded",
" . c #ddbee7",
".. c #debce9",
"X. c #e08bfd",
"o. c #e393fe",
"O. c #e59cfd",
"+. c #e5a3fb",
"@. c #e7a9fc",
"#. c #e9acfe",
"$. c #e0b2ef",
"%. c #e3b7f2",
"&. c #e9b4fa",
"*. c #edbefd",
"=. c #e5ceec",
"-. c #e8c0f5",
";. c #edc2fc",
":. c #f0c5fe",
">. c #f1cbfd",
",. c #f2d1fd",
"<. c #f5dafd",
"1. c #f7e1fe",
"2. c #f6eef9",
"3. c #f9eefd",
"4. c #fbf3fe",
"5. c white",
"6. c None",
"6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.",
"6.6.6.6.6.6.6.6.6.6.6.6.> > : : : - - - 6.6.6.6.6.6.6.6.6.6.6.6.",
"6.6.6.6.6.6.6.6.6., > > > : : - - : - - - = = 6.6.6.6.6.6.6.6.6.",
"6.6.6.6.6.6.6.6., > > , 4 0 l S S x 5 2 - = = * 6.6.6.6.6.6.6.6.",
"6.6.6.6.6.6., , , , 0 L +.3.3.3.3.3.4.;.~ 9 - % % % 6.6.6.6.6.6.",
"6.6.6.6.6., , > 5 J :.1.1.,.{ ` $.| %.<.1.1.W 6 % % % 6.6.6.6.6.",
"6.6.6.6., > > w ] ,.,.%.` | 5.5.5.5.5...%.,.,.@.f % $ # 6.6.6.6.",
"6.6.6.6., > 0 O.*.*.} _ 5.5.5.5.5.5.5.5.5.<.*.*.&.f $ # 6.6.6.6.",
"6.6.6., > 5 W #.#.@._ 5.5.5.5.5.5.5.5.5.5.5.>.#.#.X.1 $ # 6.6.6.",
"6.6.> > , V O.O.O.T 5.5.5.5.<.+.O.@.3.5.5.5.3.O.O.O.R % + # 6.6.",
"6.6.> > 0 / X.X.X.[ 5.5.5.3.X.X.X.X.' 2.5.5.5.o.X.X.X.7 # + 6.6.",
"6.6.> > V / / / / } 5.5.5.@./ / / / E  .5.5.5.O./ L F a $ + 6.6.",
"6.> > 4 K ! ! ! ! X.5.5.4.^ ! ! Q ! v 5.5.5.5.A t e r r - + O 6.",
"6.: : 0 U U U U U U X.@.! U U U U b =.5.5.5.*.r r r r r 5 O O 6.",
"6.: : e K P P P P P P P P P P P c 2.5.5.5.3.z r r r r r 0 O O 6.",
"6.: - u D Z D D D D D D D D D h 2.5.5.5.2.A r r r r r r r O o 6.",
"6.: - t B B B B B B B B B B d 2.5.5.5.3.V r r r r r r r r o o 6.",
"6.- - q m M M y m m m m m m ( 5.5.5.<.V r r r r r r r r 0 o o 6.",
"6.- - 0 n y y n y y y y y t  .5.5.5.A t r r t t y r r r 2 o X 6.",
"6.- - 4 y y y y y y y t y y  .5.5.,.y y y y y y y y y y - X X 6.",
"6.6.- - n y y n y y n y y y { 5.5./ n n y n y n n y y n + X 6.6.",
"6.6.= * 0 n n n n n n n n n M / ! y m y n n n n n n n 4 X X 6.6.",
"6.6.* * - M M M M M M M M M 8 ) ) A M M m M M M m M M + . . 6.6.",
"6.6.6.* * 6 N N N N N N N p =.5.5.3.Z B N N N N N N < . . 6.6.6.",
"6.6.6.6.% % 9 Z Z Z Z D Z l 5.5.5.5.Y Z Z Z Z Z Z 8 . . 6.6.6.6.",
"6.6.6.6.% % $ 9 D D D D D l 3.5.5.5.U D D D D D s . . . 6.6.6.6.",
"6.6.6.6.6.$ $ $ 8 P P P D P ! :.,./ P P P P P 3 X . . 6.6.6.6.6.",
"6.6.6.6.6.6.$ # # & j I I I I I I I I I I g @ . . . 6.6.6.6.6.6.",
"6.6.6.6.6.6.6.6.# # + & 3 x H U U G k ; @ X . . 6.6.6.6.6.6.6.6.",
"6.6.6.6.6.6.6.6.6.+ + + O O O o o o o X X . . 6.6.6.6.6.6.6.6.6.",
"6.6.6.6.6.6.6.6.6.6.6.6.O O o o o X X X 6.6.6.6.6.6.6.6.6.6.6.6.",
"6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6."
]


# changed, 4.2.2003 - Erica Andrews, added 'icon_stop' as 5th list element
icons = [ icon_stop, icon_alert, icon_info, icon_question, icon_stop ]



#############
# test case
#############

# changed 4.2.2003 - Erica Andrews:  'test' method now tests all 4 built-in icon types

if __name__ == "__main__":
    ret = message("Test MessageBox #1",
                  ["This is a test for the message box.", "", "Enjoy."])
    ret = message("Test MessageBox #1", 
                  ["This is a test for the message box.", "", "Enjoy."],
                  ("Ok",),ICON_STOP,gtk.FALSE)
    ret = message("Test MessageBox #1", 
                  ["This is a test for the message box.", "", "Enjoy."],
                  ("Ok",),ICON_ALERT,gtk.FALSE)
    ret = message("Test MessageBox #1", "America\n\nRich and\nPowerful".split("\n"),
                  ("Ok",),ICON_QUESTION,gtk.FALSE)





