# MOOSE Build System

## Overview

The MOOSE build system is a hierarchical Makefile based system.  The idea is that no matter where you
are in the hierarchy of MOOSE applications/modules/framework, simply typing `make` will rebuild
everything that needs to be rebuilt.  The basic way to build any MOOSE application is simply to do a:

```bash
make
```

within the application directory.

Consider an application named `Frog` that depends on two of the physics modules `tensor_mechanics`
and `heat_conduction`.  Typing `make` within the `frog` directory will automatically build
`framework`, `tensor_mechanics`, `heat_conduction` and `frog`.

In addition, the Makefiles in the MOOSE system are designed for parallel building.  Using the `-j`
flag when running `make` will allow you to build much faster.  For instance, if you have 8 processors
in your workstation you can do:

```bash
make -j 8
```

This will cause `make` to spawn 8 compilation processes at the same time, compiling up to 8x faster
than a serial compile.

## Controlling The Process: Environment Variables

The MOOSE build system relies on environment variables for control over the build process.  Here are
some of the environment variables you can set and their effects:

- `METHOD`: Set this to `opt`, `dbg`, `oprof` or `devel` to build executables with differing amounts
  of optimization and debugging capabilities.  See the Method section below for more information.
- `METHODS`: A set of space separated `METHOD` options.  This should enumerate all of the ways you
  ever want to set `METHOD`
- `MOOSE_DIR`: _Optional_.  The directory where the MOOSE repository is.  Usually the Makefile can
  find it unless you have very non-standard paths.
- `MOOSE_UNITY`: defaults to `true`, set to `false` to turn off unity builds.  For more information
  see the Build System Optimization section below.
- `MOOSE_HEADER_SYMLINKS`: defaults to `true`, set to `false` to turn off header symlinking. This is
  an advanced option and there are very few reasons to disable this. You will likely notice a
  dramatic slowdown if you set this to false.

## Build Methods (Creating "debug" Builds)

As mentioned above there are two environment variables that control what "type" of executable gets
created: `METHOD` and `METHODS`.  The executable that is ultimately created will have the `$METHOD`
as a suffix like: `something-$METHOD`.  The valid options are:

- `opt`: (The default) Builds an optimized executable suitable for running calculations.
- `dbg`: An executable meant for use in a debugger (like `gdb` or `lldb`).  Will be very slow and not
  meant to be used on large problems
- `devel`: Something in-between `opt` and `dbg`.  Attempts to be faster but also does some more
  run-time checks (asserts and mooseAsserts)
- `oprof`: Not normally used.  Can be used for "code profiling": running your code with a utility
  like `oprof`, `gprof`, `vtune`, etc.

`METHODS` is used to specify the full set (space separated) of the above options that you will _ever_
want to used.  It is used in the build of `libMesh` to build a `libMesh` library for each of the
`METHODS`.

## Build System Optimization

This is advanced information meant to inform you of optimizations that happen within the MOOSE build
system.  This is not necessary to understand how to use and build MOOSE.

There are two main build optimizations: "header symlinking" and "unity builds":

### Header Symlinking

MOOSE and MOOSE-based application's include directories are organized into sub-directories for each
system (such as `kernels`, `bcs`, `auxkernels`, etc.).  There are currently ~30 different MOOSE
systems, meaning that each application could potentially have 30+ directories to search for include
files.  In addition, applications can compose/use eachother and use the physics modules.  Each time
you add another application/module you're gaining another 30+ include directories.

This can lead to an "explosion" of include directory paths (application + 4 modules + framework = 180
paths!).  This expansion of paths can lead to the compiler slowing down as each path needs to be
searched at compile time to find a header.

To combat this, during the build phase a directory named `build/header_symlinks` is created for each
application/module.  All of the header files within that application/module are then symlinked into
that directory.  This means that each application/module you add only adds _one_ include directory
(instead of 30), greatly speeding up the compilation process.

### Unity Builds

Within the subdirectories underneath the `src` directory of a typical MOOSE-based application you
will find definitions of multiple, closely-related objects in `.C` files.  For instance in
`src/kernels` you would be likely to find 10-15 Kernels that all inherit from `Kernel.h`.  However,
if we were to compile all of these files separately (as is commonly done) each of the `#include`
trees would have to be found and compiled for each one of these files individually.  In addition,
each of these `.C` files would create a separate `.o` file that would ultimately need to be linked
into the final library / application.

A different idea is to compile all of these files together, providing considerable savings for
evaluating the `#include`s and ultimately just producing _one_ `.o` file to be linked.  This can
provide a huge savings in compile time.  The following is showing timing for `make -j 16` for various
sets of applications/modules/framework on the Falcon computer at Idaho National Laboratory.  It is
plain to see how much time is saved by using both header symlinking and unity builds.

!media media/application_development/falcon_compile_speed.png
        style=width:50%;float:right;margin-left:20px;
       caption=Compile speed on Falcon.

The way we achieve this in MOOSE is using the idea of "Unity Builds".  A Unity Build compiles
multiple `.C` files together by create a `.C` file that `#include`s all of the other `.C` files.  In
MOOSE this happens in a directory called `build/unity_src` in each application / module.

By default this behavior is +on+.  To turn it off you can set `MOOSE_UNITY=false` in your
environment.  Further, an application can disable it permanently by putting `MOOSE_UNITY := false` at
the top of their Makefile.

In addition you can also keep from unity building individual directories within your project.  By
default `src` and `src/base` are not unity built because they generally contain a mix of objects that
don't benefit from being built simultaneously.  To add to that list, set the `app_non_unity_dirs`
variable in your `Makefile` before the `include` of `app.mk` like so:

```
app_non_unity_dirs = %/src/contrib/third_party_code %/src/contrib/other_non_unity_dir

include            $(FRAMEWORK_DIR)/app.mk
```

Make sure to prepend your directories with a `%` sign.

### Revision Generation

The build process can generate C++ header files containing repository versioning information for use within your application. These header files are generated by default but can be disabled by setting a Make variable in your application's Makefile:

```
# must be set before the app.mk file is included
GEN_REVISION := no
include      $(FRAMEWORK_DIR)/app.mk
```

The Revision file contains both a version string (git hash) and a revision string containing more detailed information or tagged information. Note that the
name of your app will appear instead of "MOOSE" for your individual application:
```
/* THIS FILE IS AUTOGENERATED - DO NOT EDIT */

#ifndef MOOSE_REVISION_H
#define MOOSE_REVISION_H

#define MOOSE_REVISION "git commit f4abb66afa on 2018-03-07"
#define MOOSE_VERSION "f4abb66afa"

#endif // MOOSE_REVISION_H
```

## Working with MOOSE's build system

MOOSE uses the the tried and true Unix standard build tools, [GNU Make](https://www.gnu.org/software/make/)
and [GNU Autoconf](https://www.gnu.org/software/autoconf/). These tools are ubiquitous, reliable, and stable,
where many other build systems are not. We believe that usability of tool includes the build process as well
as the run-time experience and that philosophy shows in the way we've structured the Makefiles for building
MOOSE-based applications.

MOOSE now includes a standard `configure` script for changing select configuration options in MOOSE and
for finding optional libraries only necessary within MOOSE (not libMesh). If you find yourself needing to
add a new library or new variable the best way to learn is to look at what we already have and go from there.

!alert note
It's important to remember that running configure in MOOSE is completely optional. This is to maintain
the status quo of not changing the developer workflow since MOOSE has historically not had a configure script.
Additionally it means that users building a MOOSE-based applications will not have to go into the framework
directory to run configure prior to building their application.

### Building GNU Autotools

In general, we recommend that you do not install the GNU Autotools from your package manager. The reason for
this is that different versions of the tools can make drastic changes to the generated outputs. libMesh has
solved this problem by distributing the GNU Autotools with its source. To build libMesh's autotools, simply
change directory into libMesh's root (normally `moose/libmesh`) and run the bootstrap script. This will
build and install the autotools into libMesh's contrib directory:

```
moose/libmesh/contrib/autotools/bin
```

You'll probably want to add the path above to your PATH.

### Adding a new C-Preprocessor variable in MooseConfig.h

To add a new variable to MooseConfig.h, you'll first want to add the variable to configure.ac using one
of the variable definition macros such as "AC_DEFINE". After that, you'll run `autoheader` followed by
`autoconf`.

### Adding a new "make" variable

If you want to add an optional dependency to MOOSE that isn't already being handled by the libMesh build
system, you'll want likely want to use the "AC_SUBST" macros after adding the appropriate logic to define
those variables. You'll then edit the `conf_var.mk.in` file. Note: Since MOOSE doesn't use automake,
we don't generate a complete Makefile from running configure, rather we just optionally include "conf_var.mk"
into our normal Makefile when it exists. This makes it so that running "configure" is an optional step
when building MOOSE or a MOOSE-based application.

### File list

- `framework/configure.ac` - This is the input file that is processed by "autoconf" to create the configure
  script. If you modify this file you will need to rerun `autoconf` to regenerate the `configure` script.
  If you defined a new variable (e.g. with AC_DEFINE), you will need to run `autoheader` then `autoconf`.

- `framework/include/base/MooseConfig.h.in` - This file is auto-generated from running `autoheader`.

- `framework/conf_var.mk.in` - This file is where you put your new expansion expressions. Typically these
  are in the form of "@VARAIBLE@".

### Make Install

MOOSE's build system has the ability to create installed binaries suitable for use with the conda package manager, or installation on a shared computing resource such as Sawtooth. The `install` target will copy binary and required libraries to a file tree based on the prefix you configured with (set by the variable `PREFIX` or the `--prefix` argument.

```
$ cd moose
$ ./configure --prefix=<installation path>
$ cd <application_dir>
$ make [make options]
$ make install
```

Additionally each application may wish to make an one or more file structures available for end users to install into their own environment. This allows for end-users to install and run the application test suite, examples, tutorials, or other inputs of interest. To customize what is available for installation, the application developer will define the list of source directories that can be installed by end users in their application Makefile. For example:

```
INSTALLABLE_DIRS := tests examples tutorials
```

These paths should all be relative to the root of the application.

To give application developers the ability to customize the layout of the installed directories it's possible to specify the destination path as well by using a "key/value" syntax. For example:

```
INSTALLABLE_DIRS := test/tests->tests examples/01->example_01 examples/02->example_02 tutorials
```

In this case, if the user chooses to install each of these directories into a writable workspace the resulting directory structure will look like this:

```
$ ls -l
tests  example_01  example_02  tutorials
```

!alert warning
Make sure that installed paths do not include source code. This could result in an accidental export of information for those cleared for only binary access.

### Copying and Running Installed Inputs

Once an application has been installed, end users that have access to those binaries can install or copy those paths to their home directories and also run the tests using the test harness using convenient command line parameters.

Example:

```
$ cd <user writable location>
$ export PATH=$PATH:<prefix>/bin  # See instructions for HPC computers below
$ bison-opt --copy-inputs <directory>  # e.g. from the second example above: tests, example_01, example_02, tutorials, etc.
$ cd <directory>
$ bison-opt --run -j8 # Note: Command line parameters appearing after --run are passed to the TestHarness
```

For a complete list of the directories that may be copied use the `--show-copyable-inputs` flag.

Example:

```
$ bison-opt --show-copyable-inputs
```

When using INL HPC systems to run your input, you will load a module that will set your path correctly.

Example:

```
$ module load use.moose <app name> # e.g bison
```
