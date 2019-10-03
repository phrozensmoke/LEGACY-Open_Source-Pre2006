
TO INSTALL:

To build and install Pythonol, you need Python (of course) version 2.2 or above with the 'disutils' package included, the Python development package (with the Python.h header file), and the SQLite development package (http://www.sqlite.org/download.html) if you are running a NON-Intel system.   (A version of PySQLite is included with this source package, so you don't need to worry about that.)

NOTE FOR INTEL SYSTEMS: 
You only need to download the SQLite development package IF the system you are running is NOT an Intel-based system. (In which case, you need to download SQLite version 2.8.8 from  http://www.sqlite.org/download.html - exactly version 2.8.8)   If you are running an Intel-based system, and you have Python and the Python development package, you can simply build everything by typing 'make', then 'make install'.  (Make sure you rename 'libsqlite.a.dev-lib' to 'libsqlite.a' in the 'sqlite' subdirectory)

NOTE FOR NON-INTEL SYSTEMS:
If you are running a non-Intel system, you may want to delete the following files in the 'sqlite' sub-directory before running 'make':
	libsqlite.a(.dev-lib)
	libsqlite.la
	sqlite.h

See the 'INSTALL-README.txt' files in the 'dictionary' and 'sqlite' sub-directories for more information.

To build Pythonol from source:
	type 'make'
	then 'make install'


PYTHONOL DICTIONARY DATABASE FILES:
If you have serious trouble building the dictionary database files, this is most likely because the SQLite extension does not exist or could not be built.  So, check that first.  If you STILL cannot build the dictionary databases, you are just out of luck (they are too large for me to upload, sorry).
