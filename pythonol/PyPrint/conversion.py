# -*- coding: ISO-8859-1 -*-

########################
# PyPrint / PySpool# Image 
# Conversion
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

import os
global cversion
cversion="v. 0.1b-1"
global read_formats
read_formats=["8BIM Photoshop resource format","AVS AVS X image","BMP Microsoft Windows bitmap image","BMP24 Microsoft Windows 24bit bitmap image","DIB Microsoft Windows bitmap image","CACHE Magick Persistent Cache image format","CMYK Raw cyan, magenta, yellow, and black bytes","DCM Digital Imaging and Communications in Medicine image","DPS Display Postscript","FAX Group 3 FAX","G3 Group 3 FAX","FITS Flexible Image Transport System","FPX FlashPix Format","GIF CompuServe graphics interchange format","GIF87 CompuServe graphics interchange format (version 87a)","GRADIENT Gradual passing from one shade to another","GRADATION Gradual passing from one shade to another","GRAY Raw gray bytes","HDF Hierarchical Data Format","ICO Microsoft icon","ICON Microsoft icon","IPTC IPTC Newsphoto","BIE Joint Bilevel Image experts Group interchange format","JBG Joint Bilevel Image experts Group interchange format","JBIG Joint Bilevel Image experts Group interchange format","JPEG24 Joint Photographic Experts Group JFIF format","JPEG Joint Photographic Experts Group JFIF format","JPG Joint Photographic Experts Group JFIF format","LABEL Text image format","GRANITE Granite texture","LOGO ImageMagick Logo","NETSCAPE Netscape 216 color cube","MAP Colormap intensities and indices","MIFF Magick image format","MONO Bilevel bitmap in leastsignificantbyte first order","MTV MTV Raytracing image format","MVG Magick Vector Graphics","NULL Constant image of uniform color","PCD Photo CD","PCDS Photo CD","DCX ZSoft IBM PC multipage Paintbrush","PCX ZSoft IBM PC Paintbrush","PDB Pilot Image Format","EPDF Encapsulated Portable Document Format","PDF Portable Document Format","PCT Apple Macintosh QuickDraw/PICT","PICT Apple Macintosh QuickDraw/PICT","PICT24 24bit Apple Macintosh QuickDraw/PICT","PIX Alias/Wavefront RLE image format","PLASMA Plasma fractal image","MNG Multipleimage Network Graphics","PNG Portable Network Graphics","P7 Xv thumbnail format","PBM Portable bitmap format (black and white)","PGM Portable graymap format (gray scale)","PNM Portable anymap","PPM Portable pixmap format (color)","EPI Adobe Encapsulated PostScript Interchange format","EPS Adobe Encapsulated PostScript","EPSF Adobe Encapsulated PostScript","EPSI Adobe Encapsulated PostScript Interchange format","PS Adobe PostScript","PSD Adobe Photoshop bitmap","PWP Seattle Film Works","RGB Raw red, green, and blue bytes","RGBA Raw red, green, blue, and matte bytes","RLA Alias/Wavefront image","RLE Utah Run length encoded image","SCT Scitex HandShake","SFW Seattle Film Works","SGI Irix RGB image","STEGANO Steganographic image","RAS SUN Rasterfile","SUN SUN Rasterfile","SVG Scalable Vector Gaphics","ICB Truevision Targa image","TGA Truevision Targa image","VDA Truevision Targa image","VST Truevision Targa image","PTIF Pyramid encoded TIFF","TIF Tagged Image File Format","TIFF Tagged Image File Format","TIFF24 24bit Tagged Image File Format","TILE Tile image with a texture","TIM PSX TIM","PAL 16bit/pixel interleaved YUV","UYVY 16bit/pixel interleaved YUV","VICAR VICAR rasterfile format","VID Visual Image Directory","VIFF Khoros Visualization image","XV Khoros Visualization image","WBMP Wireless Bitmap (level 0) image","WPG Word Perfect Graphics","XBM X Windows system bitmap (black and white)","XC Constant image uniform color","PM X Windows system pixmap (color)","XPM X Windows system pixmap (color)","XWD X Windows system window dump (color)","YUV CCIR 601 4:1:1","WMF Windows Metafile"        , "PALM Pixmap Format","PICON Personal Icon", "ROSE 70x64 Truecolor Test Image","CMYKA Raw cyan, magenta, yellow, and matte bytes","APP1 Photoshop resource format","CUT Dr Hallo ", "ART PF1 1st Publisher","H Internal Format","ICC Color Profile","JP2 JPEG-2000 JP2 File Format Syntax","JPC JPEG-2000 JPC Code Stream Syntax","XCF Gimp Image","VID Visual Image Directory","DPX Digital Moving Picture Exchange","ILBM Amiga IFF","RAD Radiance Image File","MNG Multipleimage Network Graphics"]

global IMAGE_MAGICK
IMAGE_MAGICK="undefined"
global im_synonyms
im_synonyms={"ras":"sun","ppm":"pnm","jpg":"jpeg","gif87":"gif","bmp24":"bmp","dib":"bmp","jpeg24":"jpeg","pict24":"pict","pct":"pict","tif":"tiff","tiff24":"tiff","ptif":"tiff","icb":"tga","vst":"tga","vda":"tga","bie":"jbig","jbg":"jbig","g3":"fax","dcx":"pcx","pcds":"pcd","pal":"uyvy","xv":"viff","app1":"8bim","cmyka":"cmyk","rgba":"rgb"}

global valid_extensions
valid_extensions=im_synonyms.keys()+im_synonyms.values()
for ghg in read_formats:
	valid_extensions.append(ghg[0:ghg.find(" ")].strip().lower())

def isValidExtension(myext):
	global valid_extensions
	return str(myext).strip().lower() in valid_extensions

def isOnPath(binary):
  if os.environ.has_key("PATH"):
    paths=os.environ["PATH"].split(":")
    for i in paths:
      p=i
      if not p.endswith("/"): p=p+"/"
      if os.path.exists(p+str(binary)):
        return 1
  return 0  

def setHaveImageMagick(imval):
  global IMAGE_MAGICK
  IMAGE_MAGICK=imval

def haveImageMagick():
  global IMAGE_MAGICK
  if  IMAGE_MAGICK==1: return 1
  IMAGE_MAGICK=isOnPath("convert")
  return IMAGE_MAGICK

def changeType(TOTYPE):
  global im_synonyms
  if im_synonyms.has_key(TOTYPE): return im_synonyms[TOTYPE]
  return TOTYPE

def doConversion(from_file,from_type,to_file,to_type,special=0):
  if not haveImageMagick(): return [0,"The 'convert' binary from the ImageMagick program is NOT on your PATH\nImageMagick's 'convert' utility is required for all image conversions and must be on your PATH.\nImageMagick 5.2.3 or better is required. Version 5.4.6.1 or better is recommended.\nYou can find versions of ImageMagick on speakeasy.rpmfind.net or www.imagemagick.org ."]
  if not str(from_file).strip(): return [0,"No file specified to convert FROM"]
  if not str(to_file).strip(): return [0,"No file specified to convert TO"]
  if not str(from_type).strip(): return [0,"No FORMAT specified to convert FROM"]
  if not str(to_type).strip(): return [0,"No FORMAT specified to convert TO"]
  if str(to_file).strip().endswith("/"): return [0,"The file to convert TO is a directory, not a file"]
  if str(from_file).strip().endswith("/"): return [0,"The file to convert FROM is a directory, not a file"]
  if not os.path.exists(str(from_file).strip()): return [0,"The file you are trying to convert FROM does not exist: "+str(from_file).strip()]
  TOTYPE=str(to_type).lower().strip()
  if TOTYPE.find(" ") > -1: TOTYPE=TOTYPE[0:TOTYPE.find(" ")].strip()
  TOTYPE=changeType(TOTYPE)
  FROM_TYPE=str(from_type).lower().strip()  
  if FROM_TYPE.find(" ") > -1: FROM_TYPE=FROM_TYPE[0:FROM_TYPE.find(" ")].strip()
  FROM_TYPE=changeType(FROM_TYPE)
  addon=""
  if special==1: addon=FROM_TYPE+":"
  try: # remove the to_file if it exists
    os.popen("rm -f "+str(to_file).strip())
    os.popen("rm -f core") # Delete any core dumps
    os.popen("rm -f core.*") # Delete any core dumps
  except:
    pass
  try:
    fmessage=""
    f=os.popen("convert "+addon+str(from_file).strip().replace(" ","\\ ")+" "+TOTYPE+":"+str(to_file).strip().replace(" ","\\ ") )
    fmessage=f.readline().strip()
    f.close()
    if fmessage: fmessage="ImageMagick Warnings: "+fmessage
    tofile=str(to_file).strip()
    if TOTYPE=="otb": 
       if not os.path.exists(tofile): tofile=tofile+".0" # This happens when we convert from a multi-framed image (animated gif, etc)
    if os.path.exists(tofile): 
      return [1,fmessage,tofile,os.path.getsize(tofile)]
    else: return [0,fmessage+"\nConversion failed: Image '"+str(to_file).strip()+"' was not created by ImageMagick.\nImageMagick may require a supplemental library for this conversion.\nPlease check the ImageMagick documentation for the requirements for converting this format."]
  except:
    return [0,"Unknown error creating '"+str(to_file).strip()+"' with ImageMagick: Conversion failed."]
  return [0,"Unknown error creating '"+str(to_file).strip()+"' with ImageMagick: Conversion failed."]