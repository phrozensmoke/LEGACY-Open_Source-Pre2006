#! /usr/bin/python
#####################################
#  PhrozenClock gtk
#  Copyright 2002-1003 by Erica Andrews
#  (PhrozenSmoke@yahoo.com)
#
#  PhrozenClock gtk is distributed as open source 
#  under the GPL (General Public License).
#
#  A simple Windows-like or KDE-like clock 
#  management application\nsuitable for managing the 
#  clock in most window managers' system trays.
#  PhrozenClock was orginally created for use with the 
#  system tray clock in IceWM,\nand was designed to 
#  be lightweight and less memory intensive than the 
#  KDE clock. No technical support is provided for 
#  this application. However, bug reports and 
#  suggestions are welcome.
#
#  Ported to PyGTK-1 by: David Moore (djm6202@yahoo.co.nz) - March 2003
#
#####################################
import time
import os
import sys
import gtk

#set translation support
from icewmcp_common import *
_=translateCP   # from icewmcp_common.py

class clockwin: 
    def __init__(self) :
        # You may need to edited TIME_ZONE_INFO_FILE and ZONEINFO_DIR values if
        # your timezone information is stored someplace other than
        # /etc/localtime or if your 'zoneinfo' directory cannot be determined
        # (you will see the error).  The value of ZONEINFO_DIR can usually be
        # determined automatically if TIME_ZONE_INFO_FILE is valid on your
        # system, exists, and is a SYMLINK. You may also need to edit
        # TIME_ZONE_DESC_FILE.
            
        # This file usually links to a timezone file on /usr/share/zoneinfo/
        # path
        global TIME_ZONE_INFO_FILE
        TIME_ZONE_INFO_FILE='/etc/localtime'    
        
        # Leave blank for automatic detection
        global ZONEINFO_DIR
        ZONEINFO_DIR='/usr/share/zoneinfo'
        
        # This file holds the 'nice' name for the timezone
        global TIME_ZONE_DESC_FILE
        TIME_ZONE_DESC_FILE='/etc/timezone'   
        
        global CLOCK_VERSION        
        CLOCK_VERSION=this_software_version
        clockwin=gtk.Window(gtk.WINDOW_TOPLEVEL)
        self._root=clockwin
        clockwin.realize()
        clockwin.set_title('PhrozenClock gtk')
        clockwin.set_position(gtk.WIN_POS_CENTER)
        self.clockwin=clockwin
        mynotebook=gtk.Notebook()
        mynotebook.set_border_width(2)
        self.mynotebook=mynotebook
        self.locateZoneinfo()
        global TZ_DICT
        self.loadTimeZones()
        tznames=TZ_DICT.keys()
        tznames.sort()
        global HOUR24
        HOUR24={"00":"12","01":"1","02":"2","03":"3","04":"4","05":"5","06":"6","07":"7","08":"8","09":"9","10":"10","11":"11","12":"12","13":"1","14":"2","15":"3","16":"4","17":"5","18":"6","19":"7","20":"8","21":"9","22":"10","23":"11","24":"12"}
        vbox1=gtk.VBox(0,0)
        self.vbox1=vbox1
        hbox2=gtk.HBox(0,0)
        self.hbox2=hbox2
        mycal=gtk.Calendar()
        mycal.display_options((gtk.CALENDAR_SHOW_HEADING | 
                               gtk.CALENDAR_SHOW_DAY_NAMES))
        self.mycal=mycal
        hbox2.pack_start(mycal,0,0,2)
        mycal.connect("button_press_event",self.stopDayUpdates)
        vbox2=gtk.VBox(1,0)
        self.vbox2=vbox2
        label3=gtk.Label('timeholder')
        self.label3=label3
        vbox2.pack_start(label3,0,0,3)
        hbox3=gtk.HBox(0,4)
        hbox3.set_border_width(6)
        self.hbox3=hbox3
        hourcombo=gtk.Combo()
        hourcombo.set_popdown_strings(['1','2','3','4','5','6','7','8','9','10','11','12'])
        hourcombo.set_size_request(58,-1)
        self.hourcombo=hourcombo
        combo_entry4=hourcombo.entry
        combo_entry4.set_editable(0)
        hourcombo.set_border_width(2)
        self.combo_entry4=combo_entry4
        hbox3.pack_start(hourcombo,0,0,0)
        label4=gtk.Label(':')
        self.label4=label4
        hbox3.pack_start(label4,0,0,0)
        minutecombo=gtk.Combo()
        sixty=['00','01','02','03','04','05','06','07','08','09','10','11','12','13','14','15','16','17','18','19','20','21','22','23','24','25','26','27','28','29','30','31','32','33','34','35','36','37','38','39','40','41','42','43','44','45','46','47','48','49','50','51','52','53','54','55','56','57','58','59']
        minutecombo.set_popdown_strings(sixty)
        minutecombo.set_size_request(58,-1)
        minutecombo.set_border_width(2)
        self.minutecombo=minutecombo
        combo_entry5=minutecombo.entry
        combo_entry5.set_editable(0)
        self.combo_entry5=combo_entry5
        hbox3.pack_start(minutecombo,0,0,0)
        label5=gtk.Label(':')
        self.label5=label5
        hbox3.pack_start(label5,0,0,0)
        secondcombo=gtk.Combo()
        secondcombo.set_popdown_strings(sixty)
        secondcombo.set_size_request(58,-1)
        secondcombo.set_border_width(2)
        self.secondcombo=secondcombo
        combo_entry6=secondcombo.entry
        combo_entry6.set_editable(0)
        self.combo_entry6=combo_entry6
        hbox3.pack_start(secondcombo,0,0,0)
        apcombo=gtk.Combo()
        apcombo.set_popdown_strings(['AM','PM'])
        apcombo.set_border_width(2)
        apcombo.set_size_request(68,-1)
        self.apcombo=apcombo
        combo_entry7=apcombo.entry
        combo_entry7.set_editable(0)
        self.combo_entry7=combo_entry7
        hbox3.pack_start(apcombo,0,0,0)
        combo_entry4.connect("grab_focus",self.stopUpdates)
        combo_entry5.connect("grab_focus",self.stopUpdates)
        combo_entry6.connect("grab_focus",self.stopUpdates)
        combo_entry7.connect("grab_focus",self.stopUpdates)
        vbox2.pack_start(hbox3,0,0,0)
        hbox2.pack_start(vbox2,1,1,0)
        vbox1.pack_start(hbox2,1,1,0)
        hbox1=gtk.HBox(1,4)
        hbox1.set_border_width(9)
        self.hbox1=hbox1
        aboutbutt=gtk.Button(_('About'))
        TIPS.set_tip(aboutbutt,_('About'))
        aboutbutt.connect("clicked",self.doAbout)
        self.aboutbutt=aboutbutt
        hbox1.pack_start(aboutbutt,0,1,0)
        rsetbutt=gtk.Button(_('Reset'))
        TIPS.set_tip(rsetbutt,_('Reset'))
        rsetbutt.connect("clicked",self.doReset)
        self.rsetbutt=rsetbutt
        hbox1.pack_start(rsetbutt,0,1,0)
        okbutt=gtk.Button(_('OK'))
        TIPS.set_tip(okbutt,_('OK'))
        okbutt.connect("clicked",self.applyQuit)
        self.okbutt=okbutt
        hbox1.pack_start(okbutt,0,1,0)
        applybutt=gtk.Button(_('Apply'))
        TIPS.set_tip(applybutt,_('Apply'))
        applybutt.connect("clicked",self.applyDate)
        self.applybutt=applybutt
        hbox1.pack_start(applybutt,0,1,0)
        closebutt=gtk.Button(_('Cancel'))
        TIPS.set_tip(closebutt,_('Cancel'))
        self.closebutt=closebutt
        hbox1.pack_start(closebutt,0,1,0)
        closebutt.connect("clicked",doQuit)
        vbox1.pack_start(hbox1,0,0,0)
        tab1lab=gtk.Label(_('Date & Time'))
        self.tab1lab=tab1lab
        mynotebook.append_page(vbox1,tab1lab)
        vbox3=gtk.VBox(0,0)
        vbox3.set_border_width(4)
        self.vbox3=vbox3
        hbox4=gtk.HBox(0,10)
        self.hbox4=hbox4
        label6=gtk.Label(_('Current Time Zone:'))
        label6.set_alignment(0.0,0.5)
        self.label6=label6
        hbox4.pack_start(label6,0,0,1)
        timezonetext=gtk.Entry()
        timezonetext.set_editable(0)
        self.timezonetext=timezonetext
        hbox4.pack_start(timezonetext,1,1,0)
        vbox3.pack_start(hbox4,1,1,0)
        label7=gtk.Label(_('To change the timezone,select your area from the list below:'))
        label7.set_alignment(0.01,0.5)
        self.label7=label7
        vbox3.pack_start(label7,0,0,0)
        zonebox=gtk.HBox()
        zonebox.set_spacing(12)
        timezonelist=gtk.Combo()
        self.timezonelist=timezonelist
        timezoneentry=timezonelist.entry
        self.timezoneentry=timezoneentry
        timezoneentry.set_editable(0)
        timezonelist.set_popdown_strings(tznames)
        zonebox.pack_start(timezonelist,1,1,0)
        zonebutton=gtk.Button(_("Set Time Zone"))
        TIPS.set_tip(zonebutton,_("Set Time Zone"))
        zonebutton.connect("clicked",self.applyZone)
        self.zonebutton=zonebutton
        zonebox.pack_start(zonebutton,0,0,0)
        vbox3.pack_start(zonebox,0,1,0)
        tab2lab=gtk.Label(_('Time Zone'))
        self.tab2lab=tab2lab
        mynotebook.append_page(vbox3,tab2lab)
        vbox=gtk.VBox(0,0)
        vbox.set_spacing(3)
    
        
        menu_items = (
      (_('/_File'), None, None, 0, '<Branch>'),
      (_("/_File")+"/_"+FILE_RUN,'<control>R', rundlg,421,""),
      (_("/_File")+"/_"+_("Check for newer versions of this program..."), 
       '<control>U', checkSoftUpdate,420,""),
      (_('/File/sep1'), None, None, 1, '<Separator>'),
      (_('/File/_Exit'), '<control>Q', doQuit, 0, ''),
      (_('/_Help'), None, None, 0, '<LastBranch>'),
      (_('/Help/_About...'), "F2", self.doAbout, 0, ''),
      (_("/_Help")+"/_"+APP_HELP_STR, "F4", displayHelp,5010, ""),
      (_("/_Help")+"/_"+CONTRIBUTORS+"...", "F3", show_credits,913, ""),
      (_("/_Help")+"/_"+BUG_REPORT_MENU+"...", "F5", file_bug_report,5010, ""),
                        )
    
        ag = gtk.AccelGroup()
        self.itemf = gtk.ItemFactory(gtk.MenuBar, '<main>', ag)
        clockwin.add_accel_group(ag)
        self.itemf.create_items(menu_items)
        self.menubar = self.itemf.get_widget('<main>')       
        vbox.pack_start(self.menubar,0,0,0)
        vbox.pack_start(mynotebook,1,1,0)
        clockwin.add(vbox)
        self.startUpdates()
        self.startDayUpdates()
        global LAST_DATE
        LAST_DATE=(0,0,0)
        self.showTime()
        self.loadTime(1,1)
        clockwin.show_all()
        clockwin.connect("destroy",doQuit)
        global TZW
        if TZW==1: self.showTZWarning()
        gtk.timeout_add(100,self.showTime)
        
        
    def locateZoneinfo(self) :
        global ZONEINFO_DIR
        global TIME_ZONE_INFO_FILE
        try:
            if not ZONEINFO_DIR: #do nothing if its been set manually
                rpath=os.path.realpath(TIME_ZONE_INFO_FILE)
                ZONEINFO_DIR=rpath[0:rpath.rfind("zoneinfo")+8].strip()
                if not os.path.exists(ZONEINFO_DIR): raise TypeError
        except:
            self.showError(_("CRITICAL ERROR\n\n"+
               "PhrozenClock was unable to determine the location "+
               "of your 'zoneinfo' time zone files, normally located in "+
               "/usr/share/zoneinfo/.  You may have to edit the "+
               "phrozenclock.py script and set the ZONEINFO_DIR variable "+
               "manually (sorry!).Also, make sure that your /etc/localtime "+
               "file is properly LINKED.\n\nCritical Error: Quitting."))
            doQuit()

    def getCurrentTZName(self) :
        global TIME_ZONE_INFO_FILE
        global ZONEINFO_DIR
        try:
            rpath=os.path.realpath(TIME_ZONE_INFO_FILE) 
            zdir=ZONEINFO_DIR
            if not zdir.endswith("/"): zdir=zdir+"/"
            tzname=rpath[rpath.rfind(zdir)+len(zdir):len(rpath)].strip()
            return tzname
        except:
            self.showMessage(_("Could not determine your timezone's 'nickname'.\nYou may have to set this yourself."))
            return "America/New_York"   

    def showTime(self) :
        self.label3.set_text(time.ctime()+" "+time.tzname[1])
        self.timezonetext.set_text(time.tzname[1])
        global UPDATE_TIME
        if UPDATE_TIME==1: self.loadTimeClock()
        global UPDATE_DAY
        if UPDATE_DAY==1: self.loadTimeDay()
        return gtk.TRUE

    def showError(self,message_text):
        global CLOCK_VERSION
        msg_err("PhrozenClock "+CLOCK_VERSION,_(str(message_text)))

    def showMessage(self,message_text):
        global CLOCK_VERSION
        msg_info("PhrozenClock "+CLOCK_VERSION,_(str(message_text)))

    def loadTime(self,with_zone=0,with_time=0):
        self.loadTimeDay()
        if with_zone==1:
            self.timezoneentry.set_text(self.getCurrentTZName())
        if with_time==0: return
        self.loadTimeClock()

    def loadTimeDay(self):
        ltime=time.localtime()
        global LAST_DATE
        if (ltime[0],ltime[1]-1,ltime[2])==LAST_DATE: return    # only update on day/month/year changes to avoid flicker
        LAST_DATE= (ltime[0],ltime[1]-1,ltime[2])
        self.mycal.select_day(ltime[2])
        self.mycal.select_month(ltime[1]-1,ltime[0])

    def loadTimeClock(self,realtime=None):
        ltime=time.localtime()
        hour=str(ltime[3])
        min=str(ltime[4])
        sec=str(ltime[5])
        if len(sec)<2: sec="0"+sec
        if len(min)<2: min="0"+min
        if len(hour)<2: hour="0"+hour
        global HOUR24
        hour=HOUR24[hour]
        self.combo_entry4.set_text(hour)
        self.combo_entry5.set_text(min)
        self.combo_entry6.set_text(sec)
        if ltime[3] < 12: self.combo_entry7.set_text("AM")
        else: self.combo_entry7.set_text("PM")

    def stopUpdates(self,*args):
        global UPDATE_TIME
        UPDATE_TIME=0   

    def startUpdates(self):
        global UPDATE_TIME
        UPDATE_TIME=1

    def stopDayUpdates(self,*args):
        global UPDATE_DAY
        UPDATE_DAY=0

    def startDayUpdates(self):
        global UPDATE_DAY
        UPDATE_DAY=1

    def loadTimeZones(self):
        global ZONEINFO_DIR
        global TZ_DICT
        TZ_DICT={}
        try:
            if not ZONEINFO_DIR: raise TypeError
            zdir=ZONEINFO_DIR
            if not zdir.endswith("/"): zdir=zdir+"/"
            os.path.walk(zdir, self.grabZones,None)
        except:
            self.showMessage(_("ERROR: Could not load available time zones from '")+ZONEINFO_DIR+_("'....Quitting."))
            doQuit()

    def grabZones(self,arg,zdir,zfiles):
        global ZONEINFO_DIR
        global TZ_DICT
        stripper=ZONEINFO_DIR
        if not stripper: return
        if stripper=="/": return
        if not stripper.endswith("/"): stripper=stripper+"/"
        section=zdir.replace(stripper,"")
        if not section.endswith("/"): section=section+"/"
        for izone in zfiles: 
            if not izone.startswith("/"):   
                tzname=section+izone
                if not tzname.startswith("/"): TZ_DICT[tzname]=tzname
        
    def doReset(self,with_zone=1,activate=1,*args):
        if not with_zone==0: with_zone=1
        self.loadTime(1,1)
        self.startUpdates()
        self.startDayUpdates()

    def applyDate(self,set_zone=0,*args):
        hour=self.combo_entry4.get_text().strip()
        min=self.combo_entry5.get_text().strip()
        sec=self.combo_entry6.get_text().strip()
        ap=self.combo_entry7.get_text().strip()
        year=self.mycal.get_date()[0]
        month=self.mycal.get_date()[1]+1
        day=self.mycal.get_date()[2]
        upstring="date --set='"+str(month)+"/"+str(day)+"/"+str(year)+" "+hour+":"+min+":"+sec+" "+ap+"'"
        os.popen(upstring +" > /dev/null")
        if set_zone==1:
            global TIME_ZONE_DESC_FILE
            global ZONEINFO_DIR
            global TIME_ZONE_INFO_FILE
            zdir=ZONEINFO_DIR
            if not zdir.endswith("/"): zdir=zdir+"/"
            self.stopUpdates()
            self.stopDayUpdates()
            try:
                f=os.popen("rm -f "+TIME_ZONE_INFO_FILE)
                f.readline()
                os.popen("ln -sf "+zdir+self.timezoneentry.get_text().strip()+" "+TIME_ZONE_INFO_FILE)
            except:
                self.showMessage(_("Unknown error setting your timezone.\nYou may have to do this manually or edit the source for the phrozenclock.py script"))
                self.doReset(0)
                return  

            try:
                os.popen("cp -f "+TIME_ZONE_DESC_FILE+" "+TIME_ZONE_DESC_FILE+".backup")
            except:
                self.showMessage(_("Unknown error setting your timezone.\nYou may have to do this manually or edit the source for the phrozenclock.py script"))
                self.doReset(0)
                return  

            try:
                f=open(TIME_ZONE_DESC_FILE,"w")
                f.write(self.timezoneentry.get_text().strip())
                f.flush()
                f.close()
            except:
                self.showMessage(_("Unknown error writing your timezone.\nYou may have to do this manually or edit the source for the phrozenclock.py script"))
                self.doReset(0)
                return          
            self.showMessage(_("Phrozen Clock will now be quickly RESTARTED.\nThis is absolutely necessary.\n\nYou may see a quick flicker on your display,\nor may see the screen 'black' for a moment.\nDo not be alarmed, as this is normal."))
            f=os.popen(upstring +" &> /dev/null")
            time.sleep(1.5)
            runit=sys.argv[0]
            if not runit.endswith("phrozenclock.py"):
                runit=sys.argv[0]+" phrozenclock.py"
            os.system('killall -HUP -q icewm &')
            os.system('killall -HUP -q icewm-gnome &')
            #print runit
            os.popen(runit+"  TZ_WARN &")
            doQuit()
        self.doReset(0)

    def showTZWarning(self):
        self.showMessage(_("WARNING: Changes in timezones do NOT\ntake affect in ALL applications immediately.\n\nYou may have to restart X or your window manager to\nsee the timezone change in clocks on your taskbar or system tray.\n\nFor IceWM users, you can just click 'Start' menu,\nthen select 'Logout',\nthen'Restart IceWM',\nor re-apply your current IceWM theme.\n\nFor best results, you really should reboot your computer, or at\nleast re-start X. Some applications may appear to be 'stuck'\nin the old timezone until the system is rebooted.\nRunning 'xrefresh' will NOT be enough. If you are running\nmission critical or time-sensitive applications,\nyou should reboot immediately. You have been warned."))

    def applyZone(self,*args):
        self.applyDate(1)

    def applyQuit(self,*args):
        self.applyDate(0)
        doQuit()

    def doAbout(self,*args):
        global CLOCK_VERSION
        commonAbout("PhrozenClock gtk "+CLOCK_VERSION,"PhrozenClock gtk version "+CLOCK_VERSION+"\n\n"+_("A simple Windows-like or KDE-like clock management application\nsuitable for managing the clock in most window managers' system trays.\nPhrozenClock was orginally created for use with the system tray clock in IceWM,\nand was designed to be lightweight and less memory intensive than the KDE clock.\n\nPhrozenClock gtk is open source under the General Public License (GPL).")  )
        

def doQuit(*args):
    gtk.mainquit()
    sys.exit(0)

def run() :
    global TZW
    TZW=0
    if str(sys.argv).find("TZ_WARN") > -1: TZW=1
    clockwin()
    hideSplash()
    gtk.mainloop()

if __name__ == "__main__" :
    run()





