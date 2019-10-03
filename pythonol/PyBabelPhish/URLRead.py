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

import httplib, sys
import urllib
import string
import urlparse
from Cookie import Cookie
from threading import RLock

global __cookieJar
global __monitor
global __content
global __BROWSER
__cookieJar={}
__monitor=RLock()
__content=""

# The user-agent below has been disabled, because Google is blocking
# user-agents with PyBabelPhish in the string, so now, we are pretending 
# to be regular IE on XP, if they want to block something, let them block IE *smile*

#__BROWSER="Pythonol PyBabelPhish/2.0 (Pythonol; Pythonol PyBabelPhish for "+str(sys.platform)+")"

__BROWSER="Mozilla/4.0 (compatible; MSIE 6.0; Windows XP)"


def __showMessage(mess):
  pass

def __parseUrl(my_url,allowWarnings=1):
  if my_url:
    try:
      hdata=[]
      if not urlparse.urlparse(my_url)[0].lower().strip()=="http":
        if allowWarnings: __showMessage("Error: The '"+str(urlparse.urlparse(my_url)[0])+"' protocol is not yet supported.")
        return None

      hinfo=str(urlparse.urlparse(my_url)[1])
      if hinfo.find(":") == -1:
        hdata.append(hinfo.strip())
        hdata.append(80)
        hdata.append(urlparse.urlparse(my_url)[2])
        hdata.append(urlparse.urlparse(my_url)[4])
      else:  #another port was declared
        hdata.append(hinfo[0:string.find(hinfo,":")].strip())
        hdata.append(string.atoi(hinfo[string.find(hinfo,":")+1:len(hinfo)].strip()))
        hdata.append(urlparse.urlparse(my_url)[2])
        hdata.append(urlparse.urlparse(my_url)[4])
      return hdata
    except:
      if allowWarnings:
        __showMessage("Error: URL parsing error.")
      return None
  else:
    return None


def __insertCookies(httpc,userName=None):
  global __cookieJar
  if userName:
    if __cookieJar.has_key(userName):
      #print "Available COOKIES:  "+str(__cookieJar[userName])
      httpc.putheader("Cookie",str(__cookieJar[userName]))


def __collectCookies(hdrs,next_url,userName=None):
  global __cookieJar
  nURL=next_url
  cooky=Cookie()
  if hdrs:
    for i in hdrs:
      hd=string.split(i,": ")
      hd_name=hd[0].lower().strip()
      hd_value=hd[1].replace("\r","")
      hd_value=hd_value.replace("\n","").strip()
      if hd_name=="set-cookie":
        try:
          cooky.load(hd_value)
        except:
	  pass
          #print "Cookie problem"

      if hd_name=="location":
        nURL=hd_value

  if len(cooky) and userName:
    #print "COOKIES:    "+str(cooky)
    if not __cookieJar.has_key(userName):
      __cookieJar[userName]=""
    for cname in cooky.keys():
      if not cname.lower().strip()=="path":
        try:
          cval=cooky.data[cname]
          __cookieJar[userName]=str(__cookieJar[userName])+str(cval.OutputString())
        except:
	  pass
          #print "cookie error"

  #print "nURL  :  "+nURL
  return nURL


def openUrl(my_url,user_name=None,doGet=1,_params={},_referer="http://babelfish.altavista.com/babelfish/tr",showWarnings=0):
  """This method takes the following parameters: (my_url,user_name=None,doGet=1,_params={},_referer="http://babelfish.altavista.com/babelfish/tr",showWarnings=0)"""
  global __monitor
  global __content
  __monitor.acquire()

  try:
    stat=302
    __content=""
    msg=""
    hc=None
    nextUrl=my_url
    do_get=doGet
    urlParams=_params
    refer=_referer
    beenHere=0
    base_url=my_url
    try:
      base_url=str(urlparse.urlparse(my_url)[0])+"://"+str(urlparse.urlparse(my_url)[1])
    except:
      pass

    while (stat > 299) and (stat < 400): #continue on 300-codes, break on 200, 400 or 500
      resp=[580,"Code or System Error in readPage",None,None] # standard feedback
      resp=__readPage(nextUrl,user_name,do_get,urlParams,refer,showWarnings)

      stat=resp[0]
      msg=resp[1]
      __content=""+str(stat)+"  "+msg  # create a minimal content text
      #print __content
      try:
        hc.getfile().close()
      except:
        pass
      hc=resp[3]
      urlParams={}
      refer=nextUrl
      do_get=1  # any redirects should be handled as GET
      if stat>499:
        break
      nextUrl=__collectCookies(resp[2],nextUrl,user_name)
      if not nextUrl.lower().startswith("http://"):
        if nextUrl.startswith("/"):
          nextUrl=str(base_url)+str(nextUrl)
        else:
          nextUrl=str(base_url)+"/"+str(nextUrl)
      # print removed
      #print "NextUrl:  "+nextUrl

      if nextUrl==refer:
        beenHere=beenHere+1  # we have been reflected back to the same Url
      if beenHere>3:  # 3 reflections is enough
        break
      # Include something here to look for 302/301 redirects
    
    if (stat>199) and (stat<300):  # we got a 200 code back, read file
      hf=None
      try:
        hf=hc.getfile()
        __content=hf.read()
      except:
        pass
      try:
        hf.close()
      except:
        pass

  finally:
    mi=__monitor.release()
    return __content

def setBrowser(bname):
  global __BROWSER
  __BROWSER=str(bname)

def __readPage(my_url,user_name=None,doGet=1,_params={},_referer="http://www.DontTrackMe.com",showWarnings=1):
  global __BROWSER
  hdata=__parseUrl(str(my_url),showWarnings)
  if hdata:
    hc=httplib.HTTP(hdata[0],hdata[1])
    meth="GET"
    if not doGet:
      meth="POST"
    if not hdata[2]:
      hc.putrequest(meth,"/")
    else:
      content=urllib.urlencode(_params)
      if not doGet:  # using post method
        hc.putrequest(meth,hdata[2])
        #print "request put"
      else:  # using GET method
        _u=hdata[2]
        if hdata[3]:
          _u=_u+"?"+hdata[3]
        if len(_params) > 0:
          if _u.find("?") == -1:
            _u=_u+"?"+content
          else:
            if _u.endswith("&"):
              _u=_u+content
            else:
              _u=_u+"&"+content
        #print _u
        hc.putrequest(meth,_u)

    hc.putheader("Accept-Language","en, sp")
    hc.putheader("Accept","text/html, text/plain, text/xml, image/gif, image/x-bitmap, image/jpeg, image/pjpeg, image/png, */* ")
    hc.putheader("Accept-Charset","iso-8859-1,*,utf-8")
    hc.putheader("Host",hdata[0])
    #hc.putheader("User","Anonymous")
    hc.putheader("Referer",str(_referer))
    hc.putheader("User-Agent",__BROWSER)
    if doGet:
      hc.putheader("Content-length","0")
    else:
      hc.putheader("Content-length",str(len(content)))
      hc.putheader("Content-type", "application/x-www-form-urlencoded")
      #print "headers put"
    __insertCookies(hc,user_name)
    hc.endheaders()
    #print "headers ended"
    if not doGet:
      ii=hc.send(content)
      #print "hc.send: "+str(ii)
    reply, msg, hdrs = hc.getreply()
    return [reply,msg,hdrs.headers,hc]
  else: 
    return [580,"URL Parsing Error",None,None]


def manualCookie(SN,cookn,cookv):
  """ This is a convenience method for setting a cookie manually. Parameters:screenName,cookieName,cookieValue """
  if SN and cookn and cookv:    
    cname=cookn.strip()
    cvalue=cookv.strip()
    if not cvalue.endswith(";"):
      cvalue=cvalue+";"
    cook=cname+"="+cvalue
    cooky=Cookie() 
    cready=1       
    try:
      cooky.load(cook)
    except:
      __showMessage("Invalid cookie: Cookie is corrupted or cannot be read properly")
      cready=0
    if cready:
      try:
        __cookieJar[SN]=str(cooky.data[cname].OutputString())
        __showMessage("Cookie successfully set.")
      except:
        __showMessage("Error setting cookie: Could not set cookie properly.\nAny OLD cookies for this name MAY have been erased in the process.")
        cready=0
    return cready 

