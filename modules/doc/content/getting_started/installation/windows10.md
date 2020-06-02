# Windows 10

!alert! warning
Using MOOSE on Windows 10 is experimental and not fully supported.

Caveats:

- Peacock does not work correctly (artifacts during rendering: surface normals are flipped).
- Different flavors of Linux are available.

    - Be sure to choose an OS in which we support. While MOOSE will ultimately work on just about every flavor of Linux, this document assumes you have chosen Ubuntu 20.04.
!alert-end!

!include installation/wsl.md

## Close the WSL terminal

With the above complete, close the WSL terminal, and re-open it. Then proceed to our [Conda Install Instructions](getting_started/installation/conda.md).

!alert! note
Your Download's folder, while using WSL, is located at: `/mnt/c/Users/<Your User Name>/Downloads`
!alert-end!
