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

#####################################
#  A wrapper for SQLite, to allow multiple connections and 
#  opening and closing of cursors on the same connection.
#####################################

import sqlite

class Connection(sqlite.Connection):
	def __init__(self,db,mymode=0444):
		sqlite.Connection.__init__(self,db,mode=mymode,converters={},autocommit=0,encoding="iso8859-1")

	def __del__(self):
		if not self.closed:
			try:
				self.db.close()
			except:
				pass
			try:
				self.close()
			except:
				pass
			self.closed=1

	def cursor(self):
		return sqlite.Cursor(self)
		
