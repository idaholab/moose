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

  ```bash
  module list

  Currently Loaded Modulefiles:
  1) moose/.gcc-7.3.1                             5) moose/.cppunit-1.12.1_gcc-7.3.1
  2) moose/.mpich-3.2_gcc-7.3.1                   6) moose-dev-gcc
  3) moose/.petsc-3.8.3_mpich-3.2_gcc-7.3.1-opt   7) miniconda
  4) moose/.tbb-2018_U3                           8) moose-tools
  ```

- Macintosh:

  ```bash
  module list

  Currently Loaded Modulefiles:
  1) moose/.gcc-7.3.1                               6) moose/.cppunit-1.12.1_clang-6.0.1
  2) moose/.clang-6.0.1                             7) moose-dev-clang
  3) moose/.mpich-3.2_clang-6.0.1                   8) miniconda
  4) moose/.petsc-3.8.3_mpich-3.2_clang-6.0.1-opt   9) moose-tools
  5) moose/.tbb-2018_U3
  ```

If your terminal mirrors the above (version numbers may vary slightly), then you have a proper environment. Please return from whence you came, and continue troubleshooting.

!alert! warning title=That didn't work!
If you find yourself looping through our troubleshooting guide, unable to solve your issue, there is still another attempt you can perform. Start over (sorry!). But this time, perform the following before starting over:

```bash
env -i bash
export PATH=/usr/bin:/usr/sbin:/sbin
source /opt/moose/Modules/3.2.10/init/bash
```

These three commands, start a new command interpreter without any of your bash profiles.

Do note, that if *this* ends up solving your issue, then there is *something* in one of possibly many bash profiles getting in the way. At this point, you will want to reach out to our [mailing list](https://groups.google.com/forum/#!forum/moose-users) and ask for help tracking this down. Keep in mind, depending on the situation you may be asked to contact the administrators of the machine in which you are operating on (HPC clusters for example are beyound our control).
!alert-end!

!alert note
The modules contained in the moose-environment package are built in a hierarchal directory structure (some modules may not be visible until other modules are loaded).
