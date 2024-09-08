# MooseDocs Setup for MOOSE-based Applications

Any MOOSE-based application can use the MooseDocs system to create custom websites. If MOOSE is
installed, the necessary dependencies exist. Also, if you have recently created an application
with the [stork script](https://github.com/idaholab/moose/blob/master/scripts/stork.sh) (after
Aug. 15, 2018) then your application will have the files necessary for a simple website already.
The sections below will aid in setting up your application if you do not have the dependencies or
the necessary documentation files.

## Create Base Documentation Files

If you have an existing application without a `doc` directory, then a few files must be created
before you can begin building a website.

First, create a `doc` directory at the root of your application code that contains the homepage for
your application.

```bash
cd ~/project/your_animal
mkdir doc
mkdir doc/content
echo "# YourAnimalApp" > doc/content/index.md
```

Second, create the MooseDocs executable and configuration file. This is most easily done by copying
the files used by the stork script.

```
cp ~/projects/moose/stork/doc/moosedocs.py.app ~/projects/your_animal/doc/moosedocs.py
cp ~/projects/moose/stork/doc/config.yml.app ~/projects/your_animal/doc/config.yml
```

Once the files are copied over, replace all mentions of "Stork" in the `config.yml` with the name
of your application.

## Install Dependencies

If you are not using [the MOOSE conda package](getting_started/installation/conda.md optional=True)
or your package is old, then we suggest that a new `moose-tools` package be installed to obtain all
needed dependencies, which can be done using conda. This command assumes that you have created and
activated a conda environment for your MOOSE development:

!versioner! code
conda install moose-tools=__VERSIONER_CONDA_VERSION_TOOLS__
!versioner-end!

## Build a Live Website

To build a live website for your application, run the following:

```bash
cd ~/projects/your_animal/doc
./moosedocs.py build --serve
```

When this command completes a message will be printed and the site will be hosted at
[http://127.0.0.1:8000](http://127.0.0.1:8000). Note, when new pages are added the build command will
need to be re-executed.

This executable contains command-line based help, which can be accessed using the "-h" flag as
follows.

```bash
cd ~/projects/your_animal/docs
./moosedocs.py -h
```

The configuration file contains information on how to build the website, additional details regarding
this file may be found at [MooseDocs/config.md].

Once you have a basic website running, it is time to document your custom code and application
objects. Information to get started follows in the next section of this documentation. In general,
applications mimic the MOOSE process, so it is best to also refer to the MOOSE instructions for
documentation (see [framework/documenting.md]).

## Generating Object Documentation Files

During the course of development, especially during the creation of new application objects (e.g., kernels,
boundary conditions, interface conditions, etc.), it is important to create documentation outlining
any new capabilities. Templates for object documents can be generated using the MooseDocs script
using the "generate" sub-command. To generate templates for new objects (and assuming your application
is named `Foo`), run:

```
cd ~/projects/foo
./moosedocs.py generate app_types FooApp
```

For example, with a new kernel object called `FooDiffusion` without documentation, the following
output will be seen:

```
% ./moosedocs.py generate app_types FooApp
Creating/updating stub page: /Users/username/projects/Foo/doc/content/source/kernels/FooDiffusion.md
CRITICAL:0 ERROR:0 WARNING:0
```

And the following template would be created:

```markdown
# FooDiffusion

!alert construction title=Undocumented Class
The FooDiffusion has not been documented. The content listed below should be used as a starting point for
documenting the class, which includes the typical automatic documentation associated with a
MooseObject; however, what is contained is ultimately determined by what is necessary to make the
documentation clear for users.

!syntax description /Kernels/FooDiffusion

## Overview

!! Replace these lines with information regarding the FooDiffusion object.

## Example Input File Syntax

!! Describe and include an example of how to use the FooDiffusion object.

!syntax parameters /Kernels/FooDiffusion

!syntax inputs /Kernels/FooDiffusion

!syntax children /Kernels/FooDiffusion
```

Note that certain items, such as the source code description, object parameters, inputs in which the
object is used, and child objects are filled in automatically using the [MooseDocs/extensions/appsyntax.md].
The lines leading with `!!` as well as the `!alert` extension command should be removed and replaced
with relevant documentation regarding how to use the object.

!alert! note title=Use good documentation practices!
When creating documentation, it is particularly important to note any design limitations or assumptions
as well as best practices to apply when using the object.
!alert-end!
