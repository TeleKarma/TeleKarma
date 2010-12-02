
===============================================================================================================================
                                            CIS 422/522 Software Methodologies
===============================================================================================================================


===============================================================================================================================
                                           Instructor: Prof. Anthony Hornof
===============================================================================================================================


===============================================================================================================================
                                           Project 2 - Group 1 - TeleKarma
===============================================================================================================================


===============================================================================================================================
                        Team Members: Michael Volk, Thomas Stellard, Peter Batzel, Nikhil Tripathi
                                                  Date: 12/1/2010
===============================================================================================================================

-------------------------------------------------------------------------------------------------------------------------------
                                                         Tutorials:
-------------------------------------------------------------------------------------------------------------------------------

1. Tutorial for using Microsoft Visual Studio:
   (i)   TeleKarma is implemented using the C++ programming language.
   (ii)  A user with no prior experience with Microsoft Visual Studio will find the below mentioned link very useful.
   (iii) This link also features a collection of books which give in-depth knowledge about Microsoft Visual Studio:
         http://msdn.microsoft.com/en-us/vstudio/cc136611.aspx

2. Tutrial for using Subversion:
   (1)  A guide for using Subversion can be found here: http://svnbook.red-bean.com/

3. Tutorial for using OPAL:
   (i)  All the queries regarding OPAL are answered in the form of frequently asked questions.
   (ii) After reading the below link, user gets thorough knowledge about OPAL telephony.
        http://www.voxgratia.org/docs/faq.html#10_1

4. Tutorial for using Portable Tools Library:
   (i)   Portable Tools Library includes many classes that assist in writing complete
   multi-platform applications.
   (ii)  The Introduction, Apologies, CVS Access, Building PTLib,
   Using PTLib, IPv6 issues, Platform Specific Issues, Conlusion and Licensing can be found in
   the link mentioned below:
         https://opalvoip.svn.sourceforge.net/svnroot/opalvoip/ptlib/trunk/ReadMe.txt

         *TeleKarma makes extensive use of Portable Tools Library.

4. Tutorial for using wxWidgets:
   (i)   wxWidgets is a cross-platform windowing toolkit.
   (ii)  wxWidgets-specific build configuration for the wxWidgets-based view in TeleKarma NG is 
         essentially identical to that for OPAL's OpenPhone sample. See the OPAL distribution,
         samples subdirectory, openphone subdirectory, and look for the readme. More information
         about wxWidgets is available online at:
         https://www.wxwidgets.org


-------------------------------------------------------------------------------------------------------------------------------
                                 Platform Dependencies (Linux / Mac OS X / Windows)
-------------------------------------------------------------------------------------------------------------------------------
1. Mac OS X:
	TeleKarma requires Mac OS X v10.5.  Other versions may work but have
	not been tested.

2. Linux:
	TeleKarma requires Gentoo Linux running kernel v2.6.36.  Other
	distributions of and kernel versions of linux may work but have not
	been tested.

3. Microsoft Windows:
	TeleKarma runs in 32-bit Windows Vista and Windows XP, although sound quality in Windows Vista is poor due to known issues
	with the PTlib and OPAL libraries. The Windows Vista build may also run in Windows 7, although this has not been tested.
	The Windows Vista build may not work if used in Windows XP.

-------------------------------------------------------------------------------------------------------------------------------
                                 Build Dependencies (Linux and Mac OS X)
-------------------------------------------------------------------------------------------------------------------------------

1. GCC v4.2.1 (Other versions may work but have not been tested)
	Instructions for installing GCC can be found here:
	http://gcc.gnu.org/install/

2. GNU Make v3.81 (Other versions may work but have not been tested)
	Instructions for installing GNU Make can be found here:
	http://www.gnu.org/software/make/

3. Subversion (svn) v1.6.*
	Instructions for installing Subversion can be found here:
	http://subversion.apache.org/packages.html
	
NOTE: For the following dependencies: WxGTK (Linux Only), PTLib, and OPAL make
sure they are all installed using the same install prefix.  The default prefix
for all three of these libraries is /usr/local/, so if you decide to use a
different prefix, make sure you use the same one for all of the libraries.

4. WxGTK v2.8.10 (Linux Only)
	Instructions for installing WxGTK can be found here:
	http://www.wxwidgets.org/downloads/

3. PTLib (Portable Tools Library)
	To build TeleKarma, you will need to build and install the code from the PTlib
	subversion repository.  You can checkout the code with this command:
	
	svn co http://opalvoip.svn.sourceforge.net/svnroot/opalvoip/ptlib/trunk ptlib
	
	Telekarma requires revision number 24770 from the ptlib SVN
	repository.  More recent revisions may work but have not been
	tested.   Once you have checked out the SVN code you can update
	to revision 24770 using this command:

	svn update -r r24770

	Next you will need to build and install the PTLib code, you can find
	directions for doing this here:
	Linux: http://www.opalvoip.org/wiki/index.php?n=Main.BuildingPTLibUnix
	Mac OS X: http://www.opalvoip.org/wiki/index.php?n=Main.BuildingPTLibMacOSX

4. OPAL (Open Phone Abstraction Library)
	To build TeleKarma you will need to build and install the code from
	the OPAL subversion repository.  You can checkout the code with the command:

	svn co http://opalvoip.svn.sourceforge.net/svnroot/opalvoip/opal/trunk
	
	Telekarma requires revision number 24734 from the opal SVN repository.
	More recent revisions may work but have not been tested.  Once you
	have checked out the SVN code you can update to revision 24734 using
	this command:

	svn update -r r24734

	Next you will need to build and install the OPAL code, you can find
	directions for doing this here:
	Linux and Mac OS X: http://www.opalvoip.org/wiki/index.php?n=Main.BuildingOpalUnix
	
-------------------------------------------------------------------------------------------------------------------------------
                                 Build Dependencies (MS Windows)
-------------------------------------------------------------------------------------------------------------------------------

1. Microsoft Visual C++ 2008 Express Edition or Visual Studio 2008
	http://www.microsoft.com/

2. PTLib (Portable Tools Library)
	To build TeleKarma, you will need to build and install the code from the PTlib
	subversion repository. You can checkout the code with this command:
	
	svn co http://opalvoip.svn.sourceforge.net/svnroot/opalvoip/ptlib/trunk ptlib
	
	Telekarma requires revision number 24770 from the ptlib SVN
	repository.  More recent revisions may work but have not been
	tested. Once you have checked out the SVN code you can update
	to revision 24770 using this command:

	svn update -r r24770

	Next you will need to build and install the PTLib code, you can find
	directions for doing this here:
	Linux: http://www.opalvoip.org/wiki/index.php?n=Main.BuildingPTLibUnix
	Mac OS X: http://www.opalvoip.org/wiki/index.php?n=Main.BuildingPTLibMacOSX

3. OPAL (Open Phone Abstraction Library)
	To build TeleKarma you will need to build and install the code from
	the OPAL subversion repository. You can checkout the code with the command:

	svn co http://opalvoip.svn.sourceforge.net/svnroot/opalvoip/opal/trunk
	
	Telekarma requires revision number 24734 from the opal SVN repository.
	More recent revisions may work but have not been tested.  Once you
	have checked out the SVN code you can update to revision 24734 using
	this command:

	svn update -r r24734

	Next you will need to build and install the OPAL code, you can find
	directions for doing this here:
	Linux and Mac OS X: http://www.opalvoip.org/wiki/index.php?n=Main.BuildingOpalUnix
	
3. wxWidgets
	To build TeleKarma's windows GUI, you will need to install and build the wxWidgets
	version 2.8.11 release. You can obtain a self-installing distribution at:

	http://www.wxwidgets.org
	
	Use the Windows version, and carefully follow the build instructions. You will need
	to add WXDIR and WXVER. Also refer to the readme.txt file supplied with the OpenPhone
	sample application in the OPAL distribution. The OpenPhone wxWidgets configuration is 
	essentially identical to the configuration required to build the TeleKarma GUI view.

	Note that you will need to compile wxWidgets on your machine to ensure that setup.h
	is generated and updated to suite the specifics of your configuration.
	
-------------------------------------------------------------------------------------------------------------------------------
                                    Telekarma Build Instructions (Linux)
-------------------------------------------------------------------------------------------------------------------------------

There are two build targets available on linux:
1. TeleKarma CLI:
	To build Telekarma CLI enter the TeleKarma source directory and run:
	
	make tk

	This will produce an executable called tk.

2. TeleKarma GUI:
	To build Telekarma GUI enter the TeleKarma source directory and run:

	make tk-gui

	This will produce an executable called tk-gui.

NOTE: For either target, if you installed WxGTK, PTLib, and OPAL in a non-standard prefix, you
will need to specify the prefix you used as an argument to make.  For example:
	
	make tk PREFIX=/path/to/your/non-standard/prefix

-------------------------------------------------------------------------------------------------------------------------------
                                    Telekarma Build Instructions (Mac OS X)
-------------------------------------------------------------------------------------------------------------------------------

There is only one build target available on Mac OS X, TeleKarma CLI.  To
build it enter the TeleKarma source directory and run:

	make tk

This will produce an executable called tk.

NOTE: If you installed PTLib or OPAL in a non-standard prefix, you
will need to specify the prefix you used as an argument to make.  For example:
	
	make tk PREFIX=/path/to/your/non-standard/prefix

-------------------------------------------------------------------------------------------------------------------------------
                 Telekarma Build Instructions (MS Windows, Visual Studio or Visual C++ Express Edition)
-------------------------------------------------------------------------------------------------------------------------------

Project files for Visual Studio / Visual C++ Express Edition are not provided. Project configuration is difficult and complex.
Assuming you have build PTlib, OPAL, and wxWidgets using the Visual Studio or Visual C++ Express Edition (see each library's 
respective build instructions)...

1) Create a new Windows project, empty, and without precompiled headers.

2) Copy project properties from the OPAL OpenPhone sample project, including (but not limited to) linker input, additional
   library dependencies, etc.
   
3) Ensure that environment variable WXDIR holds the directory where wxWidgets resides.

4) Ensure that environment variable WDVER holds the "28" (short for version 2.8).

5) Change the project properties, linker input, to use the static versions of the PTlib and OPAL libraries (OpenPhone uses the
   dynamic libraries). See PTLib & OPAL documentation for details.

6) Add the TeleKarma NG files. Omit cli*.* files, which are specific to the console view.

7) Build the project in Win32 Release mode (Debug mode is very problematic).

-------------------------------------------------------------------------------------------------------------------------------
Very Important Note:

-Sound files having .wav extensions must be present before running TeleKarma.
-These files should be present in the same folder in which TeleKarma.exe is present.
-Logs and Recordings folders should also be present at the same location

-------------------------------------------------------------------------------------------------------------------------------

Only using the source code given below is not enough to run TeleKarma. The user / developer must
have sound files having .wav extension which are used on many occasions The different names of
sound files and the content that they should play is as follows:

1. pleasehold.wav: played to the remote party while on hold.

2. press1.wav: played to the remote party while in human detection hold mode.

3. assurance.wav: played when call recordings begin (and are sent as reminders when re-entering
Hold or Human Detection Hold).

4. userconnected.wav: played when a human is detected in human detection hold mode.

5. alert.wav: played upon automatic retrieval of a call that was in human detection hold mode,
shortly after a human has been detected.

6. alert2.wav: played upon entering human detection hold mode.

As mentioned above, the logs and recordings folders should be in the same folder as TeleKarma.exe.
The above mentioned .wav files should also be present.

-------------------------------------------------------------------------------------------------------------------------------
                                                        Download Links:
-------------------------------------------------------------------------------------------------------------------------------

1. TeleKarma can be compiled for Windows using Microsoft Visual C++ Express Edition 2008
available from
   Microsoft Corporation at http://www.microsoft.com.

2. GNU Bison file can be downloaded from
   http://www.voxgratia.org/docs/pwlib_windows.html#flexbison The same link has the the
   instructions about how to install GNU bison.

3. TeleKarma and the Opal telephony library depend on the Portable Tools Library {PTLib}.The
Portable Tools Library source code
   is available at http://www.opalvoip.org.  Links to comprehensive directions for building
   PTLib are available via their FAQ page.

4. User of TeleKarma would require to download and install Expat from the below link.
   Download the version 2.0.1, win32 installer (expat-win32bin-2.0.1.exe).
   http://expat.sourceforge.net/

5. TeleKarma depends on the OPAL telephony library, source code for which is available at
http://www.opalvoip.org
   Links to comprehensive directions for building OPAL are available via FAQ page on the Opal
   web site (http://www.opalvoip.org).

5. TeleKarma's window GUI depends on wxWidgets, available at http://www.wxwidgets.org.
