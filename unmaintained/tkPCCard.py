#! /usr/bin/python
from Tix import *
import os
import sys
import tkMessageBox
import time

#set translation support
from icewmcp_common import *
_=translateCP   # from icewmcp_common.py

###############################################
# "Phrozen's tkPCCard Manager" v.0.3
#  Copyright 2002-2003 by Erica Andrews
#  PhrozenSmoke@yahoo.com
#  http://icesoundmanager.sourceforge.net
#  
#  License: General  Public License (GPL).
#  Written: February 2002
#  Updated: February 2003
#  Monitor and control your PCMCIA cards
#  A frontend to the cardctl program.
#  License: Open Source for non-commercial use
#  Please leave these credits intact.
#
#  This program requires Python, TCL, and Tix
#  Tested only on: Mandrake 8.2, Kernel 2.4.8, 
#  Glibc 2.2, Python 2.2, TCL 8.3.4 
###############################################

class myPanel(Frame):
  def __init__(self, *args, **params):
    apply(Frame.__init__, (self,) + args, params)
    self._parent = None
    if len(args) != 0: self._parent = args[0]
    self.socket_num=0
    self.addWidgets()

  def showMessage(self,mess):
    tkMessageBox.showinfo(_("Slot ")+str(self.socket_num),str(mess))

  def addWidgets(self):
    self._widgets = {}
    self.bgcolor="#B8C9EF"
    self["bg"]=self.bgcolor
    self["relief"]="sunken"

    self._widgets['title'] = Label(self,background=self.bgcolor,height=1,width=8, text=_("Slot  ")+str(self.socket_num))
    self._widgets['title'].grid(column=1, row=1,sticky='new',pady=5)

    self._widgets['listb'] = ScrolledListBox(self,height=140,width=120,background="#FFFFFF")
    self._widgets['listb'].children["listbox"]["background"]="#FFFFFF"
    self._widgets['listb'].grid(column=1, row=2,sticky='ew')
    
    self._widgets['buttframe'] = Frame(self,background=self.bgcolor,relief="raised")
    self._widgets['buttframe'].grid(column=1, row=3,sticky='s',pady=7)

    self._widgets['eject'] = Button(self._widgets['buttframe'],background=self.bgcolor,text=_("Eject"),command=self.eject)
    self._widgets['eject'].grid(column=1, row=1,sticky='ew')

    self._widgets['insert'] = Button(self._widgets['buttframe'],background=self.bgcolor,text=_("Insert"),command=self.insert)
    self._widgets['insert'].grid(column=2, row=1,sticky='ew')

    self._widgets['reset'] = Button(self._widgets['buttframe'],background=self.bgcolor,text=_("Reset"),command=self.reset)
    self._widgets['reset'].grid(column=3, row=1,sticky='ew')

    self._widgets['suspend'] = Button(self._widgets['buttframe'],background=self.bgcolor,text=_("Suspend"),command=self.suspend)
    self._widgets['suspend'].grid(column=1, row=2,sticky='ew')

    self._widgets['resume'] = Button(self._widgets['buttframe'],background=self.bgcolor,text=_("Resume"),command=self.resume)
    self._widgets['resume'].grid(column=2, row=2,sticky='ew')




  def insertFact(self,mess):
    self._widgets['listb'].children["listbox"].insert(0,str(mess))

  def setSocketNum(self,num):
    self.socket_num=num
    self._widgets['title']["text"]=_("Slot  ")+str(num)

  def loadFacts(self,num):
    try:  
      i=0    
      while i < 40:
        self._widgets['listb'].children["listbox"].delete(0)
        i=i+1
      i=0
      while i < 40:
        self._widgets['listb'].children["listbox"].delete(0)
        i=i+1
    except:
      pass
    has_facts=1
    try:
      f=os.popen("cardctl ident "+str(self.socket_num))
      i=1
      while i:
        s=f.readline().strip()
        if s=="": 
          break
        elif s.find("no card") > -1:
          has_facts=0
          self.insertFact(_("Empty"))
          break
        elif (s.find("product info:") > -1) and (s.find(",") > -1 ):
          ss=s[s.find("product info:")+13:len(s)].strip()
          sss=ss.split(",")
          for y in sss:
            self.insertFact(y.strip())
        else:
          self.insertFact(s)
      f.close()
      if has_facts:
        f=os.popen("cardctl status "+str(self.socket_num))
        i=1
        while i:
          s=f.readline().strip()
          if s=="": break
          self.insertFact(s)
    except:
      self.insertFact(_("Error loading card data"))

  def eject(self):
    try:
      f=os.popen("cardctl eject "+str(self.socket_num))
      f.close()
      time.sleep(2)
      self.showMessage(_("This card has been ejected.\nYou may safely remove this device."))
      self.loadFacts(self.socket_num)
    except:
      self.showMessage("An error occurred while ejecting this card.")


  def suspend(self):
    try:
      f=os.popen("cardctl suspend "+str(self.socket_num))
      time.sleep(2)
      f.close()
      self.showMessage(_("This card's activity has been suspended"))
      self.loadFacts(self.socket_num)
    except:
      self.showMessage(_("An error occurred while suspending this card."))

  def resume(self):
    try:
      f=os.popen("cardctl resume "+str(self.socket_num))
      time.sleep(2)
      f.close()
      self.showMessage(_("This card's activity has been resumed."))
      self.loadFacts(self.socket_num)
    except:
      self.showMessage(_("An error occurred while resuming this card."))

  def insert(self):
    try:
      f=os.popen("cardctl insert "+str(self.socket_num))
      time.sleep(2)
      f.close()
      self.showMessage(_("This card has been inserted."))
      self.loadFacts(self.socket_num)
    except:
      self.showMessage(_("An error occurred while inserting this card."))


  def reset(self):
    try:
      f=os.popen("cardctl eject "+str(self.socket_num))
      time.sleep(2)
      f.close()
      f=os.popen("cardctl insert "+str(self.socket_num))
      time.sleep(1)
      f.close()
      self.showMessage(_("This card has been reset."))
      self.loadFacts(self.socket_num)
    except:
      self.showMessage(_("An error occurred while resetting this card."))





class myPCCard(Frame):
  def __init__(self, *args, **params):
    apply(Frame.__init__, (self,) + args, params)
    self._parent = None
    if len(args) != 0: self._parent = args[0]
    self.socket_count=0
    self.addWidgets()


  def showMessage(self,mess):
    tkMessageBox.showinfo("tkPCCard",_(str(mess)))

  def getSocketCount(self):
    try:
      f=os.popen("cardctl status")
      i=1
      while i:
        s=f.readline().strip()
        if s.startswith("Socket"):
          self.socket_count=self.socket_count+1
        if s=="": break
    except:
      self.socket_count=0
    return self.socket_count


  def addWidgets(self):
    if not self.getSocketCount():
      self.showMessage(_("No PCMCIA slots detected on this computer."))
      #sys.exit(0)

    self._widgets = {}
    self.bgcolor="#DEE2EE"
    self["bg"]=self.bgcolor

    self._widgets['spacer'] = Label(self,background="#000000",foreground="#FFFFFF",height=2,font='-*-MS Sans Serif-Bold-R-Normal-*-*-220-*-*-*-*-*-*',text=" tkPCCard Manager v.0.3",justify="center")
    self._widgets['spacer'].grid(column=1, row=1,sticky='new')


    self._widgets['cardframe'] = Frame(self,background=self.bgcolor)
    self._widgets['cardframe'].grid(column=1, row=2,sticky='new')


    self._widgets['bframe'] = Frame(self,background=self.bgcolor)
    self._widgets['bframe'].grid(column=1, row=3,sticky='new')


    self._widgets['exitb'] = Button(self._widgets['bframe'],background=self.bgcolor,text=_("   EXIT   "),command=self.shutdown)
    self._widgets['exitb'].grid(column=1, row=1,sticky='new',pady=10,padx=10)

    self._widgets['about'] = Button(self._widgets['bframe'],background=self.bgcolor,text=_("   About   "),command=self.about)
    self._widgets['about'].grid(column=2, row=1,sticky='new',pady=10,padx=10)

    i=-1
    while i < self.socket_count-1:
      i=i+1
      p=myPanel(self._widgets['cardframe'])
      p.setSocketNum(i)
      p.loadFacts(i)
      p.grid(column=i+1,row=1,padx=8)

  def shutdown(self):
    sys.exit(0)

  def about(self):
    self.showMessage("PhrozenSmoke's tkPCCard Manager v.0.3\nCopyright 2003 by Erica Andrews\nPhrozenSmoke@yahoo.com\nhttp://icesoundmanager.sourceforge.net\nLicense: Open Source under the  General Public License (GPL).\nPlease leave author's credits intact.")

def run():
  root=Tk()
  p=myPCCard(root)
  p.pack(side=TOP, fill=BOTH, expand=1)
  root.title("PhrozenSmoke's tkPCCard Manager")
  root["bg"]="#DEE2EE"
  root.mainloop()


if __name__ == "__main__":
  run()
