This directory contains the source code for 3 Gyach-E encryption plugins:


	- Three encryption plugins added to provide support for encrypted instant messaging conversations, for a total of 15 supported encryption methods, including 2 variations of Blowfish encryption:

		- gyach-gpgme.c:  GPG / GPGMe encryption plugin ( provides support for sending unsigned, encrypted/ascii-armored messages, Requirements: GPGMe 0.3.16 or better; tested on gpgme  0.3.16, GPG 1.0.7)

		- gyach-mcrypt.c:  MCrypt encryption plugin (provides support for a variety of encryption/encoding methods: XTea, Blowfish, Twofish, Tripledes, Gost, Cast-128, Cast-256, RC6, Rijndael-256, ArcFour/RC4, Serpent, Safer+, and Loki97 ; Requirements: MCrypt/libMCrypt 2.5.2-22 or better, tested on MCrypt 2.5.2-22 )

		- gyach-blowfish.c:  Basic Blowfish encryption plugin (GyachE Blowfish-Internal, uses a public domain implementation of Blowfish)


TO INSTALL:
	Edit the 'PLUGDIR' value in the Makefile in this directory to suit your tastes, then type 'make', then 'make install'.


A FEW WORDS ABOUT PLUGINS:

	- Plugins are stored in the '/plugins/' sub-directory of your Gyach Enhanced installation directory, normally /usr/local/share/gyach/plugins/. Please note that encryption support is only available when you have your PMs configured to be opened in separate windows (instead of having PMs show up on the chat screen); Encryption session negotiation is handled through a series of YMSG 'Notify' packets; As of version 0.2 of the MCrypt plugin, Gyach Enhanced sends ascii-armored versions of the encrypted data: This does not change the encryption algorithm used in any way, but rather, it converts the data into a plain-text (hex-like) string of data acceptable for sending across Yahoo's network: Without this, your Yahoo connection would disconnect frequently when using encrypted instant messaging through the MCrypt plugin.  It is very likely that the encryption plugins will only work with other Gyach-E users, unless other applications choose to support the 'Notify' packets used by Gyach Enhanced.  For the GPG/GPGMe plugin, passphrases are auto-generated, and using private/public keys is not supported: For this reason, the GPG plugin is probably not very compatible with the GPG plugin used by Ayttm.    Also, be aware that use of these encryption plugins may not be legal in all jurisdictions, especially terrorist countries:  I take no responsiblity if you use these encryption plugins in violation of your local, state, or federal laws.  The encryption works for instant messages ONLY:  You cannot encrypt conferences and chat rooms!

The Blowfish-Internal Plugin:
	Has not special requirements but is still very experimental: Errors and truncated data can occur frequently, leading to incomplete, garbled, or lost messages: Use with caution.

The MCrypt Plugin:
	There are still many issues to be worked out with encryption through MCrypt: Not all of the algorithms listed in Gyach Enhanced are necessarily going to be supported on your system.  Also a number of these algorithms seem to randomly truncate data, which lead to incomplete or empty messages arriving.  Some of the MCrypt algorithms seem more stable than others: Safer+, Serpent Gost, Cast-256 (somewhat), MCrypt-Blowfish, and XTea.  The following algorithms are known to experience errors and truncation frequently: RC6, Tripledes, RC4/ArcFour, Twofish, Loki97

THE GPG / GPGMe Plugin:
	The GPG plugin sends unsigned, ascii-armored (encrypted) messages.  The encryption algorithm used to encrypt the message depends on your system's configuration.  For instance, if you want to send GPG messages using the Blowfish encryption method, you should put the following values in your ~/.gnupg/options file:

no-version
no-comment
digest-algo SHA1
cipher-algo BLOWFISH
s2k-digest-algo SHA1
s2k-cipher-algo BLOWFISH

The 'no-version' and 'no-comment' flags prevents GPG from putting a bunch of unnecessary header/version information in the messages (creating shorter messages to be sent over the network.)  Other possible values for the 'cipher-algo' and 's2k-cipher-algo' fields are: 3DES, CAST5, BLOWFISH, AES, AES192, AES256, TWOFISH.  Other possible values for the 'digest-algo' and 's2k-digest-algo' fields are: MD5, SHA1, RIPEMD160



