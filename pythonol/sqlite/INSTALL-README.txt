WINDOWS USERS:
	If you are running Python 2.3 or better, you can get PySQLite easily by 
	extracting the files in the 'PySQLite-0.5.0-for-Windows.tar.gz' package 
	to this directory/folder.  (WinZip can handle the .tar.gz file type.) 
	You can then delete everything in this directory EXCEPT for the following files:
		__init__.py
		main.py
		_sqlite.pyd
		PySQLite-0.5.0-for-Windows.tar.gz  (Keep it just in case you need it)

	You SHOULD make sure you delete the __init__.pyc and main.pyc files (notice
	the 'c' at the end of the name.).  All files ending in .so and .a are also safe to 
	delete for Windows users.


BUILDING THE PySQLite MODULE FROM SOURCE...


This directory includes the SQLite development files for Intel systems running Linux (compiled on a SuSE i686, but should work with i586, and any RedHat/SuSE/Mandrake-style system).  If you are NOT using an Intel-based system, you will have to download your own copy of the sqlite development files (see below).  If you ARE using an Intel system, just rename 'libsqlite.a.dev-lib' to 'libsqlite.a'.

To build the PySQLite module, you will need to download the SQLite development package from: 

http://www.sqlite.org/download.html

(SQLite Version 2.8.8 is what is needed to compile against the included 
PySQLite version 0.5.0;  It is NOT recommended that you attempt to 'upgrade' to a higher version of PySQLite or SQLite yourself, since Pythonol is using a modified version of the PySQLite library.  PySQLite version 0.5.0 is already included.  Even if higher versions of SQLite are available, you should get version 2.8.8, since that is the only version officially supported by this version of Pythonol.)

After you have installed the sqlite development package, place the following files in THIS directory:

libsqlite.a
libsqlite.la   (Maybe needed, but not necessarily)
sqlite.h

(The combination of SQLite 2.8.0 + PySQLite 0.4.0 (used in previous Linux/Unix/Mac versions of Pythonol) is NO LONGER SUPPORTED.  Also, the PySQLite 0.3.1 + SQLite 2.8.0 combination that was used in previous Windows versions of Pythonol is NO LONGER SUPPORTED.  The currently supported combination of software is: PySQLite 0.5.0 + SQLite 2.8.8 - That's it. If you try to use another combination of software and Pythonol fails to function properly, DO NOT bother sending in a bug report.)

NOTE: You also need the Python development package, with the Python.h header file.

Then, run the 'build.sh' script, or go to the top of the distribution directory ('pythonol') and type 'make'.

When that is done, make sure you have a file named '_sqlite.so' in THIS directory, as well as two other files: 'main.py' and '__init__.py'


WINDOWS USERS:
I *think* that a version of PySqlite is available for Windows in .dll format at:  http://pysqlite.sourceforge.net   
If it is, get the .dll file from that site, and place it in this directory.  (You can delete the '_sqlite.so' file if it exists).  Then run the 'PyInstallShield' program in the top of the distribution directory ('pythonol').


