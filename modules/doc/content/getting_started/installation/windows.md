# Windows

!alert! warning
Using MOOSE on Windows 10 and 11 is experimental and not fully supported.

Caveats:

- Peacock does not work correctly (artifacts during rendering: surface normals are flipped).
- Different flavors of Linux are available, but *be sure to choose a distribution which we support*.
  While MOOSE will ultimately work on just about every flavor of Linux, this document assumes you
  have chosen Ubuntu 20.04, which is the Windows Subsystem for Linux (WSL) default as of December 2022.
!alert-end!

!include installation/wsl.md

## Close the WSL terminal

With the above complete, close the WSL terminal, and re-open it. Then proceed to our [Conda Install Instructions](getting_started/installation/conda.md).

!alert! tip
Your Download's folder, while using WSL, is located at: `/mnt/c/Users/<Your User Name>/Downloads`
!alert-end!
