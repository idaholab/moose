# Macintosh OS

!include docs/content/getting_started/minimum_requirements.md

---
## Pre-Reqs
* Xcode Command Line Tools
    * To install Command Line Tools [CLT] on your machine, simply open a terminal and run the following command: `xcode-select`
    * If you do not have CLT installed, you will be presented with a dialog box allowing you to install CLT.
* [XQuartz 2.7.9](https://dl.bintray.com/xquartz/downloads/XQuartz-2.7.9.dmg)
* MOOSE Environment package (choose one):
    * Sierra 10.12: !moosepackage arch=osx10.12 return=link!
    * El Capitan 10.11: !moosepackage arch=osx10.11 return=link!

!!! note
    If you have any opened terminals at this point, you will need to close them, and re-open them in order to use the MOOSE environment. The following instructions will ultimately fail if you do not.

!include docs/content/getting_started/installation/clone_moose.md
!include docs/content/getting_started/installation/build_libmesh.md
!include docs/content/getting_started/installation/conclusion.md
