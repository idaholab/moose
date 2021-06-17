## Modules id=modules

Modules allow users to control what libraries and binaries are being made available within that terminal session. It is worth mentioning that module commands, only affect the terminal they are in. It is not global. This is why we routinely ask users to operate in a single terminal while troubleshooting issues.

Users who have installed one of our moose-environment packages, will have access to modules. Please familiarize yourself with some commonly used module commands:

| Command | Command Arg | Usage |
| - | - | - |
| module | list | List currently loaded modules |
| module | avail | List available modules |
| module | load <module module module> | Load a space separated list of modules |
| module | purge | Remove all loaded modules |

To begin working with modules manually, it is best to start clean (especially for the duration of this FAQ). You can do so by purging any current modules loaded:

```bash
module purge
```

Load the two default modules pertaining to your operating system:

- Linux:

  - `module load moose-dev-gcc moose-tools`

- Macintosh:

  - `module load moose-dev-clang moose-tools`

Loading these two modules will in-turn load other necessary modules. The correct modules that should be loaded will approximately resemble the following list:

- Linux:

  !package! code max-height=400
  module list

  Currently Loaded Modulefiles:
  1) moose/.gcc-__GCC__                             5) moose/.cppunit-__CPPUNIT__-gcc-__GCC__
  2) moose/.mpich-__MPICH__-gcc-__GCC__                   6) moose-dev-gcc
  3) moose/.petsc-__PETSC_DEFAULT__-mpich-3.2_gcc-__GCC__-opt   7) miniconda
  4) moose/.tbb-__TBB__                           8) moose-tools
  !package-end!

- Macintosh:

  !package! code max-height=400
  module list

  Currently Loaded Modulefiles:
  1) moose/.gcc-__GCC__                               6) moose/.cppunit-__CPPUNIT__-clang-__LLVM__
  2) moose/.clang-__LLVM__                             7) moose-dev-clang
  3) moose/.mpich-__MPICH__-clang-__LLVM__                   8) miniconda
  4) moose/.petsc-__PETSC_DEFAULT__-mpich-__MPICH__-clang-__LLVM__-opt   9) moose-tools
  5) moose/.tbb-__TBB__
  !package-end!

If your terminal mirrors the above (version numbers may vary slightly), then you have a proper environment. Please return from whence you came, and continue troubleshooting.

!alert! note title=Ughh! None of this is working!
If you find yourself looping through our troubleshooting guide, unable to solve your issue, there is still another attempt you can perform. Start over. But this time, perform the following before starting over:

!package! code max-height=400
env -i bash
export PATH=/usr/bin:/usr/sbin:/sbin
source /opt/moose/Modules/__MODULES__/init/bash
!package-end!

These three commands will start a new command interpreter without any of your default environment. This is important because for most errors we end up solving, it was due to *something* in the users environment.

Do note, if *this* ends up solving your issue, then there is *something* in one of possibly many bash profiles getting in the way. At this point, you will want to contact the [MOOSE Discussion forum](https://github.com/idaholab/moose/discussions) and ask for help tracking this down. Keep in mind, depending on the situation you may be asked to contact the administrators of the machine in which you are operating on (HPC clusters for example are beyound our control).
!alert-end!

!alert note
The modules contained in the moose-environment package are built in a hierarchal directory structure (some modules may not be visible until other modules are loaded).
