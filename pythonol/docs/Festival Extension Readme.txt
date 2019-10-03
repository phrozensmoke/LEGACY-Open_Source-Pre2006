[PHTITLE]FESTIVAL TEXT-TO-SPEECH EXTENSION FOR Pythoñol

[PHSECTION]IMPORTANT NOTE:
[PH-HI]Please do NOT email me with questions about how to get Festival downloaded, installed, or configured. I am NOT offering technical support to the general public for this feature.[/PH-HI]  This document IS your 'technical support'.  If you  have other questions ask the folks at the Festival web site for help: ([PH-HI]http://www.cstr.ed.ac.uk/download/festival/[/PH-HI])  It's THEIR software.

[PHSECTION]WINDOWS USERS: 
[PH-HI]The Festival text-to-speech extension is NOT SUPPORTED on Windows versions of Pythoñol.[/PH-HI]  This does not mean you cannot use or attempt to use Festival with Pythoñol.  What it does mean is that this  feature is only supported on Linux/Unix, and there are no future plans to add text-to-speech support to Windows versions of Pythoñol.  However, Windows users may freely download and modify the source code for Pythoñol (doing so in accordance with the Free Education Initiative License), and download the source code for Festival (http://www.cstr.ed.ac.uk/download/festival/), and try to implement this support yourself.  (Be sure to read both the license agreement for Festival, and the license agreement for the Festival voice files, as both are different from one another, and different from the Free Education Initiative License under which Pythoñol is distributed).

[PHSECTION]LINUX / UNIX USERS: 
To enable text-to-speech support via the Festival extenion currently used by Pythoñol, you must first do ONE of the following:

	1.  Download the 'Festival-extension' package from  [PH-HI]http://pythonol.sourceforge.net[/PH-HI] 
	    (IF you are running an Intel-compatible, i586 or better Linux system - glibc), and 
	    install the package by running 'PyInstallShield', or 'INSTALL-ME.sh'.  It's probably 
	    easiest to install the extension in the /festival/ sub-directory of your Pythoñol 
	    installation directory, however; this is not required. If you install festival in another 
	    directory, you will need to create a symbolic link from your 'festival' executable to 
	    'PYTHONOL_INSTALLATION_DIRECTORY/festival/festival'.  In addition, you will 
	    need to edit the 'tts.spanish' script in 
	    PYTHONOL_INSTALLATION_DIRECTORY/festival/, to suit your system.

	2.  Download the Festival source code and install Festival from  
	    [PH-HI]http://www.cstr.ed.ac.uk/download/festival/[/PH-HI] , and 
	    get the necessary spanish voice files:  (At last check, the voice files were at 
	    [PH-HI]http://www.cstr.ed.ac.uk/download/festival/1.4.3/[/PH-HI]  - in a file named 
	    [PH-HI]'festvox_ellpc11k.tar.gz'[/PH-HI] )   Alternatively, voice files created by 
	    by a different group of software developers is available at: 
	    [PH-HI]http://cslu.cse.ogi.edu/tts/download/[/PH-HI] .  It's probably 
	    easiest to install the extension in the /festival/ sub-directory of your Pythoñol 
	    installation directory, however; this is not required. If you install festival in another 
	    directory, you will need to create a symbolic link from your 'festival' executable to 
	    'PYTHONOL_INSTALLATION_DIRECTORY/festival/festival'.  In addition, you will 
	    need to edit the 'tts.spanish' script in 
	    PYTHONOL_INSTALLATION_DIRECTORY/festival/, to suit your system.

[PH-HI]Be sure to read both the license agreement for Festival, and the license agreement for the Festival voice files, as both are different from one another, and different from the Free Education Initiative License under which Pythoñol is distributed.  You should pay CLOSE attention to the Festival licensing agreement, particularly for the voice files.  Festival appears to licensed free of usage restrictions, however, the voice files are not, and it appears that the voice files are only licensed for non-commercial use.[/PH-HI]

After you have installed the extension correctly, you *may* have to edit the script named 'tts.spanish'  in the '/festival/' sub-directory of your Pythoñol installation directory to ensure that the Festival extension is correctly configured to play through your sound card, and that the script is correctly configured to utilize your version of Festival. (Read the script's comments at the top of the file.)  You should especially check and edit this script if you installed festival from another distribution other than Pythoñol's 'Festival-extension' package, or you installed Festival from source.   

If you have the Festival text-to-speech extension installed and usable, you may hear the words pronounced throughout most of Pythoñol by clicking the light blue [PH-HI]'Play'[/PH-HI] button (with the small black arrow), which is generally located to the left of your screen.  For instance, try clicking [PH-HI]'Reference Section' -> 'Basic Spanish' -> 'Colors'[/PH-HI] on the Pythoñol menu.  You will see a [PH-HI]'Play'[/PH-HI] button next to each word, allowing you to hear a Spanish pronounciation of each color.

For text windows, such as the ones found in the [PH-HI]'Conjugate' and 'Dictionary'[/PH-HI] sections, you may listen to words and phrases in the text window by clicking the mouse in front of the word or phrase you wish to hear pronounced.  Then, click the light blue [PH-HI]'Play'[/PH-HI] button.  If you would like to type in your own words and phrases to hear them pronounced, try clicking the [PH-HI]'Pronounce'[/PH-HI] tab in Pythoñol.

[PHSECTION]DRAG AND DROP:
Both the 'Pronounce' and 'Translate' (PyBabelPhish) modules within Pythoñol feature drag-and-drop support.  You can drag and drop a text file onto the text windows of either of these modules. If all goes well, the file will be displayed in the text window, after which you can simply click 'Play' to hear the text read aloud. When reading longer texts, Pythoñol will only attempt to read up to 30 kilobytes at a time aloud through the Festival speech engine.  This is for reasons of speed.

[PHSECTION]IF YOU HAVE THE FESTIVAL EXTENSION BUT HEAR NO PRONUNCIATIONS:
Try editing the script named [PH-HI]'tts.spanish'[/PH-HI] (a Bash script)  in the [PH-HI]'/festival/'[/PH-HI] sub-directory of your Pythoñol installation directory to ensure that the Festival extension is correctly configured to play through your sound card, and that the script is correctly configured to utilized your version of Festival. (Read the script's comments at the top of the script file for more information.)  You should especially check and edit this script if you installed festival from another distribution other than Pythoñol's 'Festival-extension' package, or you installed Festival from source.   You do NOT need to re-start Pythoñol to do this. In fact, its probably better to have Pythoñol running while you make modifications to the script, so you can easily test the Festival extension.

[PHSECTION]KNOWN LIMITATIONS:
Pronunciations through the Festival speech engine are very usable, but not perfect. Occasionally, the Festival engine will 'skip' or 'choke' if it encounters strange words or unfamiliar syllables.  Occasionally, Festival will pronounce things you wouldn't expect it to pronounce, such as punctuation marks (i.e. saying "comma" aloud).   When reading longer texts, Pythoñol will only attempt to read up to 30 kilobytes at a time aloud through the Festival speech engine.  This is for reasons of speed, so the readings don't appear to 'lag'.