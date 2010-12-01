TeleKarma NG - The Final 24 Hours


Core Functionality Bugs & Status:
==================================

1) X-platform play sound
   -> Possible fix submitted 11/29, untested on windows??

2) Speakers cannot be muted

3) Microphone is not being turned off during hold state(s).

4) PCSS audio quality is marginal to poor, depending on platform



wxWidgets GUI-specific Bugs & Status:
======================================

1) Menu items do not enable/disable appropriately on linux
   -> Possible fix submitted 11/30, untested

2) Human detected sound plays inappopriately upon manual retrieve.
   -> Probable fix submitted 11/30, untested

3) Pressing enter while the destination text control has focus
   does not invoke dialing.
   -> Fix submitted 11/30, tested on WIN32 only

4) Pressing enter in the registration dialog does not invoke registration.
   -> Fix submitted 11/30, tested on WIN32 only

5) Mouse icon should be hourglass during "busy" processes
   -> Fix submitted 11/30, lightly tested on WIN32 only

6) Cannot use the keyboard to enter touchtones

7) Trace window doesn't scroll down; text gets hidden
   -> Fix submitted 11/30, tested on WIN32, needs X-platform testing

8) State messages sometimes come through garbled; for example, reason 
   for remote disconnection. Inconsistent.
   -> Improved string conversion methods, submitted 11/30, not confident
      that the problem is fixed

9) Dialpad doesn't close automatically when a connection ends or enters hold

10) With the dialpad open, select Call -> Touch Tones... (robbing dialpad of focus),
    and the dialpad (already open) does not get focus (as I would expect).

11) Garbage input to register dialog crashed GUI
    -> Reported by Nikhil; don't have a specific reproducible case to work with



Console-specific Bugs & Status:
================================

1) Quit does not work

2) All possible menu options always displayed

3) Menu of currently available options not printed

4) Command input required enter




Tasks To Go
============

1) Implement rich account persistance feature set. [ low priority ]

2) Add an icon to the window & for the EXE. [ very low priority ]

3) Eliminate SMS from code and views

4) Document code, compile, extract class diagram for static diagram

5) Package all files, excluding unnecessary ones, into a zip



Documentation Tasks To Go
==========================

1) Finish agendas and minutes for final few meetings.

2) Document efforts made on SMS.

3) Document recording only records remote endpoint.

4) Project Plan

5) SRS

6) SDS

7) Readme

8) User Guide

9) Installation Guide

10) Code documentation (see Tasks To Go)



Final Package Checklist
========================

___ Project Plan

___ SRS

___ SDS

___ Readme file

___ User Guide

___ Installation Guide (tested!!)

___ Compiled code docs

___ Source code

___ Compiled binaries for target platforms

___ Audio files

___ Appropriate subdirectories (logs, recordings)

___ accounts.txt

___ More ??? (this list is a work in progress)