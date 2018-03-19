# Documenting MOOSE

MOOSE and MOOSE-based applications generally focus on creating C++ objects (e.g., Kernels, BCs,
etc.), so it is important that these object are documented to allow other developers and users to
understand the purpose of these objects. The [MooseDocs System](MooseDocs/index.md) aims to make
documenting objects straightforward and natural. Moreover, accessing the object documentation is
simple and allows for documentation to remain up-to-date as the code continues to advance.

In order to ensure that MOOSE documentation continues to improve requirements exist for
documenting new and changing code within MOOSE:

1. Any new MooseObject must have all parameters and a class description.
2. Any new MooseObject must also include an associated markdown description page.
2. Developers who modify existing classes should update the existing markdown documentation file.
3. All new tests must contain a 'requirement' and link to 'design' and 'issues'.

## C++ Documentation

The first step is to add documentation for your application in the `validParams` function. This is
done by adding a and parameter documentation strings and calling the class description method.

A description of each parameter should also be provided when they are added with the various add
methods of the `InputParameters` object. For example, in the
[FunctionIC](/ICs/FunctionIC.md) the following parameter documentation is
added, which is then present in the parameter summary table of the generated site.

!listing framework/src/ics/FunctionIC.C line=addRequiredParam

The string supplied in this function will appear in the parameter tables within the documentation
that is generated.  For example, see the parameter table for the
[DirichletBC object](/DirichletBC.md).

Secondly, a short description should be supplied in each `addParam`, `addPrivateParam`, etc. function
in your code. For example, in the [Diffusion](/Diffusion.md) object the following class description.

!listing framework/src/kernels/Diffusion.C line=addClassDescription

When the documentation for this object is generated this string is added to the first portion of the
page and the system overview table. For example, the [Kernels overview](systems/Kernels/index.md)
includes a table with each object listed; the table includes the class description from the source
code.

## Markdown Documentation

Clear and consistent documentation is a necessary component of code developement within MOOSE.  It is
expected that developers submitting new MooseObjects (e.g., `Kernel` or `BoundaryCondition`) to the
MOOSE and MOOSE module repository will create a corresponding markdown file, using
[MooseDocs/specification.md], to document the new classes.  The created
file must be stored in the documentation for the framework or associated module.

If you are writing a documentation for a new class with MOOSE or the modules, you can use the
MooseDocs executable to build a documentation stub for your new class; the stub includes a markdown
documentation template.  The markdown documentation template stub adheres to the documentation page
standards described in the [MooseDocs/standards.md] and
includes examples of common MooseDocs extensions, including equations and references.

Template stubs for the systems and objects in MOOSE-based applications can be generated using the
"moosedocs.py" script, using the command as follows:

```bash
cd ~/projects/moose/docs
./moosedocs.py check --generate
```

This generate command needs to be run only when you edit source code of your application:

- When you add a new object (e.g., Kernel, BoundaryCondition, etc.) to your application,
- When you add or modify the class description of an existing object, or
- When you modify the input parameters of an existing object with the `validParams` function.

It is possible to check the status of your applications documentation. This check will examine your
code and provide a list of errors for un-documentented items including a missing class description or
a missing object or system details file. To perform the check utilize the "moosedocs.py" script with
the check command as follows:

```text
cd ~/projects/moose/docs
./moosedocs.py check
```

After the stub files have been created the a local, live version of the website should
be used to add content (see the [Live Website](#live-website) section below).

## Live Website

When adding documentation it is useful to create a create a live version of the MOOSE website. To
create the site MOOSE modules executable must exist, this is accomplished by using the following
commands.

```bash
cd ~/projects/moose/modules
make -j16 # 16 should be replaced by the number of cores on your system
```

Next, the website content must be built and served, this is done by running the following commands.
When this command completes, which takes several minutes, a message will be printed and the site will
be hosted at [http://127.0.0.1:8000](http://127.0.0.1:8000).

```bash
cd ~/projects/moose/docs
./moosedocs build --serve
```

Once the server is running the markdown files within the repository may be modified, when the
changes are saved the local server will automatically update the content.
The content added or modified should follow the
[Standards for Documentation Pages](MooseDocs/standards.md) guidelines.

## Requirement, Design, and Issues

MOOSE follows strict software quality standards, to meet these standards every test within MOOSE
must provide three items with each test of the test specification.

1. +requirement+: A sort description that explains what the "requirement" that the test is
   testing. The text for the requirement must be listed in the test specification file ("tests" file).

1. +design+: A list of markdown files associated with the test that explain the systems, objects,
   and design documents. The file paths do not need to be complete, but must be unique.

1. +issues+: A list of [github issues](https://github.com/idaholab/moose/issues/) that are
   associated with the test and items being tested.
