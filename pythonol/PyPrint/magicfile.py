# -*- coding: ISO-8859-1 -*-

########################
# PyPrint / PySpool: File Type 
# Detection
#
# Copyright 2002-2004 
# Erica Andrews
# PhrozenSmoke ['at'] yahoo.com
# http://pythonol.sourceforge.net
#
# (C)opyright 2000 Jason Petrone 
# <jp@demonseed.net>
#
# This software is 
# distributed under the 
# terms of the GNU 
# General Public License.
########################

'''
magic.py
 determines a file type by its magic number

 (C)opyright 2000 Jason Petrone <jp@demonseed.net>
 All Rights Reserved

 Command Line Usage: running as `python magic.py file` will print
                     a description of what 'file' is.

 Module Usage:
     magic.whatis(data): when passed a string 'data' containing 
                         binary or text data, a description of
                         what the data is will be returned.

     magic.file(filename): returns a description of what the file
                           'filename' contains.
'''
#  Edited and adapted by Erica Andrews
#  PhrozenSmoke@yahoo.com
#  9/1/2002

import re, struct, string

__version__ = '0.1'

magic = [
  [0L, 'string', '=', 'FiLeStArTfIlEsTaRt', 'txt'],
  [0L, 'string', '=', '//', 'txt'],
  [0L, 'string', '=', '#!/bin/sh', 'txt'],
  [0L, 'string', '=', '#! /bin/sh', 'txt'],
  [0L, 'string', '=', '#! /bin/sh', 'txt'],
  [0L, 'string', '=', '#!/bin/csh', 'txt'],
  [0L, 'string', '=', '#! /bin/csh', 'txt'],
  [0L, 'string', '=', '#! /bin/csh', 'txt'],
  [0L, 'string', '=', '#!/bin/ksh', 'txt'],
  [0L, 'string', '=', '#! /bin/ksh', 'txt'],
  [0L, 'string', '=', '#! /bin/ksh', 'txt'],
  [0L, 'string', '=', '#!/bin/tcsh', 'txt'],
  [0L, 'string', '=', '#! /bin/tcsh', 'txt'],
  [0L, 'string', '=', '#! /bin/tcsh', 'txt'],
  [0L, 'string', '=', '#!/usr/local/tcsh', 'txt'],
  [0L, 'string', '=', '#! /usr/local/tcsh', 'txt'],
  [0L, 'string', '=', '#!/usr/local/bin/tcsh', 'txt'],
  [0L, 'string', '=', '#! /usr/local/bin/tcsh', 'txt'],
  [0L, 'string', '=', '#! /usr/local/bin/tcsh', 'txt'],
  [0L, 'string', '=', '#!/usr/local/bin/zsh', 'txt'],
  [0L, 'string', '=', '#! /usr/local/bin/zsh', 'txt'],
  [0L, 'string', '=', '#! /usr/local/bin/zsh', 'txt'],
  [0L, 'string', '=', '#!/usr/local/bin/ash', 'txt'],
  [0L, 'string', '=', '#! /usr/local/bin/ash', 'txt'],
  [0L, 'string', '=', '#! /usr/local/bin/ash', 'txt'],
  [0L, 'string', '=', '#!/usr/local/bin/ae', 'txt'],
  [0L, 'string', '=', '#! /usr/local/bin/ae', 'txt'],
  [0L, 'string', '=', '#! /usr/local/bin/ae', 'txt'],
  [0L, 'string', '=', '#!/bin/nawk', 'txt'],
  [0L, 'string', '=', '#! /bin/nawk', 'txt'],
  [0L, 'string', '=', '#! /bin/nawk', 'txt'],
  [0L, 'string', '=', '#!/usr/bin/nawk', 'txt'],
  [0L, 'string', '=', '#! /usr/bin/nawk', 'txt'],
  [0L, 'string', '=', '#! /usr/bin/nawk', 'txt'],
  [0L, 'string', '=', '#!/usr/local/bin/nawk', 'txt'],
  [0L, 'string', '=', '#! /usr/local/bin/nawk', 'txt'],
  [0L, 'string', '=', '#! /usr/local/bin/nawk', 'txt'],
  [0L, 'string', '=', '#!/bin/gawk', 'txt'],
  [0L, 'string', '=', '#! /bin/gawk', 'txt'],
  [0L, 'string', '=', '#! /bin/gawk', 'txt'],
  [0L, 'string', '=', '#!/usr/bin/gawk', 'txt'],
  [0L, 'string', '=', '#! /usr/bin/gawk', 'txt'],
  [0L, 'string', '=', '#! /usr/bin/gawk', 'txt'],
  [0L, 'string', '=', '#!/usr/local/bin/gawk', 'txt'],
  [0L, 'string', '=', '#! /usr/local/bin/gawk', 'txt'],
  [0L, 'string', '=', '#! /usr/local/bin/gawk', 'txt'],
  [0L, 'string', '=', '#!/bin/awk', 'txt'],
  [0L, 'string', '=', '#! /bin/awk', 'txt'],
  [0L, 'string', '=', '#! /bin/awk', 'txt'],
  [0L, 'string', '=', '#!/usr/bin/awk', 'txt'],
  [0L, 'string', '=', '#! /usr/bin/awk', 'txt'],
  [0L, 'string', '=', '#! /usr/bin/awk', 'txt'],
  [0L, 'string', '=', 'BEGIN', 'txt'],
  [0L, 'string', '=', '#!/bin/perl', 'txt'],
  [0L, 'string', '=', '#! /bin/perl', 'txt'],
  [0L, 'string', '=', '#! /bin/perl', 'txt'],
  [0L, 'string', '=', 'eval "exec /bin/perl', 'txt'],
  [0L, 'string', '=', '#!/usr/bin/perl', 'txt'],
  [0L, 'string', '=', '#! /usr/bin/perl', 'txt'],
  [0L, 'string', '=', '#! /usr/bin/perl', 'txt'],
  [0L, 'string', '=', 'eval "exec /usr/bin/perl', 'txt'],
  [0L, 'string', '=', '#!/usr/local/bin/perl', 'txt'],
  [0L, 'string', '=', '#! /usr/local/bin/perl', 'txt'],
  [0L, 'string', '=', '#! /usr/local/bin/perl', 'txt'],
  [0L, 'string', '=', 'eval "exec /usr/local/bin/perl', 'txt'],
  [0L, 'string', '=', '#!/bin/python', 'txt'],
  [0L, 'string', '=', '#! /bin/python', 'txt'],
  [0L, 'string', '=', '#! /bin/python', 'txt'],
  [0L, 'string', '=', 'eval "exec /bin/python', 'txt'],
  [0L, 'string', '=', '#!/usr/bin/python', 'txt'],
  [0L, 'string', '=', '#! /usr/bin/python', 'txt'],
  [0L, 'string', '=', '#! /usr/bin/python', 'txt'],
  [0L, 'string', '=', 'eval "exec /usr/bin/python', 'txt'],
  [0L, 'string', '=', '#!/usr/local/bin/python', 'txt'],
  [0L, 'string', '=', '#! /usr/local/bin/python', 'txt'],
  [0L, 'string', '=', '#! /usr/local/bin/python', 'txt'],
  [0L, 'string', '=', 'eval "exec /usr/local/bin/python', 'txt'],
  [0L, 'string', '=', '#!/usr/bin/env python', 'txt'],
  [0L, 'string', '=', '#! /usr/bin/env python', 'txt'],
  [0L, 'string', '=', '#!/bin/rc', 'txt'],
  [0L, 'string', '=', '#! /bin/rc', 'txt'],
  [0L, 'string', '=', '#! /bin/rc', 'txt'],
  [0L, 'string', '=', '#!/bin/bash', 'txt'],
  [0L, 'string', '=', '#! /bin/bash', 'txt'],
  [0L, 'string', '=', '#! /bin/bash', 'txt'],
  [0L, 'string', '=', '#!/usr/local/bin/bash', 'txt'],
  [0L, 'string', '=', '#! /usr/local/bin/bash', 'txt'],
  [0L, 'string', '=', '#! /usr/local/bin/bash', 'txt'],
  [0L, 'string', '=', '#! /', 'txt'],
  [0L, 'string', '=', '#! /', 'txt'],
  [0L, 'string', '=', '#!/', 'txt'],
  [0L, 'string', '=', '#! txt', 'txt'],
  [0L, 'string', '=', 'gimp xcf', 'image/xcf'],
  [0L, 'string', '=', 'FORM', 'image/ilbm'],
  [0L, 'string', '=', 'P1', 'image/pbm'],
  [0L, 'string', '=', 'P2', 'image/pgm'],
  [0L, 'string', '=', 'P3', 'image/ppm'],
  [0L, 'string', '=', 'P4', 'image/pbm'],
  [0L, 'string', '=', 'P5', 'image/pgm'],
  [0L, 'string', '=', 'P6', 'image/ppm'],
  [0L, 'string', '=', 'IIN1', 'image/tiff'],
  [0L, 'string', '=', 'MM\000*', 'image/tiff'],
  [0L, 'string', '=', 'II*\000', 'image/tiff'],
  [0L, 'string', '=', '\211PNG', 'image/png'],
  [1L, 'string', '=', 'PNG', 'image/png'],
  [0L, 'string', '=', 'GIF8', 'image/gif'],
  [0L, 'string', '=', '\361\000@\273', 'image/ras'],
  [0L, 'string', '=', 'id=ImageMagick', 'image/miff'],
  [1L, 'string', '=', 'PC Research, Inc', 'image/g3'],
  [0L, 'beshort', '=', 65496L, 'image/jpeg'],
  [0L, 'string', '=', 'hsi1', 'image/jpeg'],
  [0L, 'string', '=', 'BM', 'image/bmp'],
  [0L, 'string', '=', 'IC', 'image/ico'],
  [0L, 'string', '=', '/* XPM */', 'image/xpm'],
  [0L, 'leshort', '=', 52306L, 'image/rle'],
  [0L, 'string', '=', 'Imagefile version-', 'image/iblm'],
  [0L, 'belong', '=', 1504078485L, 'image/ras'],
  [0L, 'beshort', '=', 474L, 'image/sgi'],
  [2048L, 'string', '=', 'PCD_IPI', 'image/pcd'],
  [0L, 'string', '=', 'PCD_OPA', 'image/pcd'],
  [0L, 'string', '=', 'SIMPLE  =', 'image/fits'],
  [53L, 'string', '=', 'yyprevious', 'txt'],
  [21L, 'string', '=', 'generated by flex', 'txt'],
  [0L, 'string', '=', '%{', 'txt'],
  [0L, 'string', '=', 'Begin3', 'txt'],
  [0L, 'string', '=', ';;', 'txt'],
  [0L, 'string', '=', 'Relay-Version:', 'txt'],
  [0L, 'string', '=', '#! rnews', 'txt'],
  [0L, 'string', '=', 'N#! rnews', 'txt'],
  [0L, 'string', '=', 'Forward to', 'txt'],
  [0L, 'string', '=', 'Pipe to', 'txt'],
  [0L, 'string', '=', 'Return-Path:', 'txt'],
  [0L, 'string', '=', 'Path:', 'txt'],
  [0L, 'string', '=', 'Xref:', 'txt'],
  [0L, 'string', '=', 'From:', 'txt'],
  [0L, 'string', '=', 'Article', 'txt'],
  [0L, 'string', '=', 'BABYL', 'txt'],
  [0L, 'string', '=', 'Received:', 'txt'],
  [0L, 'string', '=', 'MIME-Version:', 'txt'],
  [4L, 'string', '=', 'Research,', 'image/g3'],
  [0L, 'short', '=', 256L, 'image/g3'],
  [0L, 'short', '=', 5120L, 'image/g3'],
  [0L, 'string', '=', '%PDF-', 'image/pdf'],
  [0L, 'string', '=', '%!', 'ps'],
  [0L, 'string', '=', '\004%!', 'ps'],
  [0L, 'string', '=', '<!DOCTYPE HTML', 'html'],
  [0L, 'string', '=', '<!doctype html', 'html'],
  [0L, 'string', '=', '<HEAD', 'html'],
  [0L, 'string', '=', '<head', 'html'],
  [0L, 'string', '=', '<TITLE', 'html'],
  [0L, 'string', '=', '<title', 'html'],
  [0L, 'string', '=', '<html', 'html'],
  [0L, 'string', '=', '<HTML', 'html'],
  [0L, 'string', '=', 'This is Info file', 'txt'],
  [0L, 'string', '=', 'LBLSIZE=', 'image/vicar'],
]

magicNumbers = []

def strToNum(n):
  val = 0
  col = long(1)
  if n[:1] == 'x': n = '0' + n
  if n[:2] == '0x':
    # hex
    n = string.lower(n[2:])
    while len(n) > 0:
      l = n[len(n) - 1]
      val = val + string.hexdigits.index(l) * col
      col = col * 16
      n = n[:len(n)-1]
  elif n[0] == '\\':
    # octal
    n = n[1:]
    while len(n) > 0:
      l = n[len(n) - 1]
      if ord(l) < 48 or ord(l) > 57: break
      val = val + int(l) * col
      col = col * 8
      n = n[:len(n)-1]
  else:
    val = string.atol(n)
  return val
       
def unescape(s):
  # replace string escape sequences
  while 1:
    m = re.search(r'\\', s)
    if not m: break
    x = m.start()+1
    if m.end() == len(s): 
      # escaped space at end
      s = s[:len(s)-1] + ' '
    elif s[x:x+2] == '0x':
      # hex ascii value
      c = chr(strToNum(s[x:x+4]))
      s = s[:x-1] + c + s[x+4:]
    elif s[m.start()+1] == 'x':
      # hex ascii value
      c = chr(strToNum(s[x:x+3]))
      s = s[:x-1] + c + s[x+3:]
    elif ord(s[x]) > 47 and ord(s[x]) < 58:
      # octal ascii value
      end = x
      while (ord(s[end]) > 47 and ord(s[end]) < 58):
        end = end + 1
        if end > len(s) - 1: break
      c = chr(strToNum(s[x-1:end]))
      s = s[:x-1] + c + s[end:]
    elif s[x] == 'n':
      # newline
      s = s[:x-1] + '\n' + s[x+1:]
    else:
      break
  return s

class magicTest:
  def __init__(self, offset, t, op, value, msg, mask = None):
    if t.count('&') > 0:
      mask = strToNum(t[t.index('&')+1:])  
      t = t[:t.index('&')]
    if type(offset) == type('a'):
      self.offset = strToNum(offset)
    else:
      self.offset = offset
    self.type = t
    self.msg = msg
    self.subTests = []
    self.op = op
    self.mask = mask
    self.value = value
      

  def test(self, data):
    if self.mask:
      data = data & self.mask
    if self.op == '=': 
      if self.value == data: return self.msg
    elif self.op ==  '<':
      pass
    elif self.op ==  '>':
      pass
    elif self.op ==  '&':
      pass
    elif self.op ==  '^':
      pass
    return None

  def compare(self, data):
    #print str([self.type, self.value, self.msg])
    try: 
      if self.type == 'string':
        c = ''; s = ''
        for i in range(0, len(self.value)+1):
          if i + self.offset > len(data) - 1: break
          s = s + c
          [c] = struct.unpack('c', data[self.offset + i])
        data = s
      elif self.type == 'short':
        [data] = struct.unpack('h', data[self.offset : self.offset + 2])
      elif self.type == 'leshort':
        [data] = struct.unpack('<h', data[self.offset : self.offset + 2])
      elif self.type == 'beshort':
        [data] = struct.unpack('>H', data[self.offset : self.offset + 2])
      elif self.type == 'long':
        [data] = struct.unpack('l', data[self.offset : self.offset + 4])
      elif self.type == 'lelong':
        [data] = struct.unpack('<l', data[self.offset : self.offset + 4])
      elif self.type == 'belong':
        [data] = struct.unpack('>l', data[self.offset : self.offset + 4])
      else:
        #print 'UNKNOWN TYPE: ' + self.type
        pass
    except:
      return None
  
#    print str([self.msg, self.value, data])
    return self.test(data)
    

def load(file):
  global magicNumbers
  lines = open(file).readlines()
  last = { 0: None }
  for line in lines:
    if re.match(r'\s*#', line):
      # comment
      continue
    else:
      # split up by space delimiters, and remove trailing space
      line = string.rstrip(line)
      line = re.split(r'\s*', line)
      if len(line) < 3:
        # bad line
        continue
      offset = line[0]
      type = line[1]
      value = line[2]
      level = 0
      while offset[0] == '>':
        # count the level of the type
        level = level + 1
        offset = offset[1:]
      l = magicNumbers
      if level > 0:
        l = last[level - 1].subTests
      if offset[0] == '(':
        # don't handle indirect offsets just yet
        print 'SKIPPING ' + string.join(list(line[3:]))
        pass
      elif offset[0] == '&':
        # don't handle relative offsets just yet
        print 'SKIPPING ' + string.join(list(line[3:]))
        pass
      else:
        operands = ['=', '<', '>', '&']
        if operands.count(value[0]) > 0:
          # a comparison operator is specified
          op = value[0] 
          value = value[1:]
        else:
          print str([value, operands])
          if len(value) >1 and value[0] == '\\' and operands.count(value[1]) >0:
            # literal value that collides with operands is escaped
            value = value[1:]
          op = '='

        mask = None
        if type == 'string':
          while 1:
            value = unescape(value)
            if value[len(value)-1] == ' ' and len(line) > 3:
              # last value was an escaped space, join
              value = value + line[3]
              del line[3]
            else:
              break
        else:
          if value.count('&') != 0:
            mask = value[(value.index('&') + 1):]
            print 'MASK: ' + mask
            value = value[:(value.index('&')+1)]
          try: value = strToNum(value)
          except: continue
          msg = string.join(list(line[3:]))
        new = magicTest(offset, type, op, value, msg, mask)
        last[level] = new
        l.append(new)

def whatis(data):
  for test in magicNumbers:
     m = test.compare(data)
     if m: return m
  # no matching, magic number. is it binary or text?
  for c in data:
    if ord(c) > 128:
      return 'data'
  # its ASCII, now do text tests
  if string.find('The', data, 0, 8192) > -1:
    return 'txt'
  if string.find('def', data, 0, 8192) > -1:
    return 'txt'
  return 'unknown'
      
    
def file(file):
  try:
    return whatis(open(file, 'r').read(8192))
  except Exception, e:
    if str(e) == '[Errno 21] Is a directory':
      return 'directory'
    else:
      return "unknown"


import sys
for m in magic:
  magicNumbers.append(magicTest(m[0], m[1], m[2], m[3], m[4]))
