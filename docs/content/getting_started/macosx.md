# Macintosh OS

{!content/getting_started/minimum_requirements.md!}

---
## Pre-Reqs
* Xcode Command Line Tools
    * To install Command Line Tools [CLT] on your machine, simply open a terminal and run the following command: `xcode-select`
    * If you do not have CLT installed, you will be presented with a dialog box allowing you to install CLT.
* [XQuartz 2.7.9](https://dl.bintray.com/xquartz/downloads/XQuartz-2.7.9.dmg)
* MOOSE Environment package (choose one):
    * El Capitan 10.11: !MOOSEPACKAGE arch=osx10.11 return=link!
    * Yosemite 10.10: !MOOSEPACKAGE arch=osx10.10 return=link!

!!! note
    If you have any opened terminals at this point, you will need to close them, and re-open them in order to use the MOOSE environment. The following instructions will ultimately fail if you do not.

{!content/getting_started/clone_moose.md!}
{!content/getting_started/build_libmesh.md!}
{!content/getting_started/conclusion.md!}