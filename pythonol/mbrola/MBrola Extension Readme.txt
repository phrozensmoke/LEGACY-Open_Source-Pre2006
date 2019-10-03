MBROLA TEXT-TO-SPEECH EXTENSION FOR Pythoñol

WINDOWS USERS: The MBrola text-to-speech extension is NOT AVAILABLE on Windows versions of Pythoñol.  This feature is only supported on Linux/Unix, and there are no future plans to add text-to-speech support to Windows versions of Pythoñol.


LINUX / UNIX USERS: To enable text-to-speech support, you must first download the 'MBrola-extension' package from  http://pythonol.sourceforge.net , if your copy of Pythoñol did not come with the extension. (Currently, only the RPM version of Pythoñol automatically installs the MBrola extension.)  After you have downloaded the MBrola extension, extract it to any directory of your choice, then follow the simple directions in the 'README_FIRST.txt' file for information on installing the extension.  After you have installed the extension correctly, you *may* have to edit the script named 'tts.spanish'  in the '/mbrola/' sub-directory of your Pythoñol installation directory to ensure that the MBrola extension is correctly configured to play through your sound card. (Read the script's comments at the top of the file.)  Please be aware that MBrola is third-party freeware, the binaries of which are authorized for non-commerical redistribution.  MBrola is NOT open-source and is NOT written by me.


If you have the MBrola text-to-speech extension installed and usable, you may hear the words pronounced throughout most of Pythoñol by clicking the light blue 'Play' button (with the small black arrow), which is generally located to the left of your screen.  For instance, try clicking 'Reference Section' -> 'Basic Spanish' -> 'Colors' on the Pythoñol menu.  You will see a 'Play' button next to each word, allowing you to hear a Spanish pronounciation of each color.

For text windows, such as the ones found in the 'Conjugate' and 'Dictionary' sections, you may listen to words and phrases in the text window by clicking the mouse in front of the word or phrase you wish to hear pronounced.  Then, click the light blue 'Play' button.  If you would like to type in your own words and phrases to hear them pronounced, try clicking the 'Pronounce' tab in Pythoñol.

DRAG AND DROP:
Both the 'Pronounce' and 'Translate' (PyBabelPhish) modules within Pythoñol feature drag-and-dro support.  You can drag and drop a text file onto the text windows of either of these modules. If all goes well, the file will be displayed in the text window, after which you can simply click 'Play' to hear the text read aloud. When reading longer texts, Pythoñol will only attempt to read up to 10 kilobytes at a time aloud through the MBrola speech engine.  This is for reasons of speed.


IF YOU HAVE THE MBROLA EXTENSION BUT HEAR NO PRONUNCIATIONS:
Try editing the script named 'tts.spanish'  in the '/mbrola/' sub-directory of your Pythoñol installation directory to ensure that the MBrola extension is correctly configured to play through your sound card. (Read the script's comments at the top of the file.) You should especially edit this file if your system does not play audio through /dev/audio, or you do not have Sox installed on your system. You do NOT need to re-start Pythoñol to do this. In fact, its probably better to have Pythoñol running while you make modifications to the script, so you can easily test the MBrola extension.

KNOWN LIMITATIONS:
Pronunciations through the MBrola speech engine are very usable, but not perfect. Occasionally, the MBrola engine will 'skip' or 'choke' if it encounters strange words or unfamiliar syllables.  When reading longer texts, Pythoñol will only attempt to read up to 10 kilobytes at a time aloud through the MBrola speech engine.  This is for reasons of speed.