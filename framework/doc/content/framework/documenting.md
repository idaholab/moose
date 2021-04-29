# Documenting MOOSE

MOOSE and MOOSE-based applications generally focus on creating C++ objects (e.g., Kernels, BCs,
etc.), so it is important that these objects are documented to allow other developers and users to
understand the purpose of these objects. The [MooseDocs System](MooseDocs/index.md optional=True) aims to make
documenting objects straightforward and natural. Moreover, accessing the object documentation is
simple and allows for documentation to remain up-to-date as the code continues to advance.

In order to ensure that MOOSE documentation continues to improve, requirements exist for
documenting new and changing code within MOOSE:

1. Any new MooseObject must have descriptions for the class and all parameters.
1. Any new MooseObject must also include an associated markdown description page.
1. Developers who modify existing classes should update the existing markdown documentation file.
1. All new tests must contain a 'requirement' and link to 'design' and 'issues'.

## MooseObject C++ Documentation

The first step is to add documentation for your application in the `validParams` function. This is
done by adding parameter documentation strings and calling the class description method.

A description of each parameter should also be provided when they are added with the various add
methods of the `InputParameters` object. For example, in the
[/BoxMarker.md] the following parameter documentation is
added, which is then present in the parameter summary table of the generated site.

!listing framework/src/markers/BoxMarker.C
         start=params.addRequiredParam
         end=bottom_left
         include-end=True

The string supplied in this function will appear in the parameter tables within the documentation
that is generated.  For example, see the parameter table for [/BoxMarker.md].

Secondly, a short description of the class should be supplied in the `addClassDescription`
function. For example, the [Diffusion](/Diffusion.md) object has the following class description:

!listing framework/src/kernels/Diffusion.C
        start=addClassDescription
        end=form
        include-end=True

When the documentation for this object is generated, this string is added to the first portion of the
page and the system overview table. For example, the [Kernels overview](syntax/Kernels/index.md)
includes a table with each object listed; the table includes the class description from the source
code.

## Markdown Documentation

Clear and consistent documentation is a necessary component of code development within MOOSE.  It is
expected that developers submitting new MooseObjects (e.g., `Kernel` or `BoundaryCondition`) to the
framework or modules will create a corresponding markdown file to document the new classes.

The documentation for the classes within MOOSE and the modules are located within the "doc"
directory where the class is registered: "framework/doc" contains all core MOOSE level objects,
"modules/tensor_mechanics/doc" contains objects for the tensor mechanics modules, etc.

When adding documentation, the MOOSE modules executable must exist; this is accomplished by using the
following commands:

```bash
cd ~/projects/moose/modules
make -j16 # 16 should be replaced by the number of cores on your system
```

If you are writing a documentation page for a new class with MOOSE or the modules, you can use the
MooseDocs executable to build a documentation stub for your new class. However, the executable
for the module must be used. For example, if you add a new class to the tensor mechanics
module:

```bash
cd ~/projects/moose/modules/tensor_mechanics
make -j16
cd doc
./moosedocs.py generate TensorMechanicsApp
```

To generate pages for the framework, the moose test application can be used as follows.

```bash
cd ~/projects/moose/test
make -j16
cd doc
./moosedocs.py generate MooseApp
```

This generate command needs to be run only when you add a new object (e.g., Kernel,
BoundaryCondition, etc.) to your application.

After the stub files have been created, a local, live version of the website should
be used to add content (see the [Live Website](#live-website) section below).

## Live Website

A local website can be created and served; this is done by running the following commands.  When
this command completes, which can take several minutes, a message will be printed and the site will
be hosted at [http://127.0.0.1:8000](http://127.0.0.1:8000):

```bash
cd ~/projects/moose/modules/doc
./moosedocs.py build --serve
```

Once the server is running, the markdown files within the repository may be modified. When
changes are saved, the local server will automatically update the content.
The content added or modified should follow the
[Standards for Documentation Pages](MooseDocs/standards.md optional=True) guidelines.

## Media

In general, media files should be placed within the `content/media` directory within the
framework or module directories. However, if a large file (>2MB), such as a movie, is needed then
the "large media" repository should be used. Within MOOSE there is a folder, `large_media`, which
is a submodule. The large file should be added to this repository and the submodule should be
updated in MOOSE to reflect this change.

## Requirement, Design, and Issues

MOOSE follows strict software quality standards. To meet these standards, every test within MOOSE
must provide three items with each test of the test specification.

1. +requirement+: A description of the "requirement" that the test is
   testing. The text for the requirement must be listed in the test specification file ("tests" file).

1. +design+: A list of markdown files associated with the test that explain the systems, objects,
   and design documents. The file paths do not need to be complete, but must be unique.

1. +issues+: A list of [github issues](https://github.com/idaholab/moose/issues/) that are
   associated with the test and items being tested.

For more information regarding writing the requirement test see [what_is_a_requirement.md].

These items are provided in the associated "tests" file. For example,

```
[Tests]
  [my_test]
    type = 'CSVDiff'
    input = 'my_test.i'
    csvdiff = 'my_test.csv'

    requirement = "MyObject shall do some kind of thing. Maybe this description has "
                  "multiple lines."
    design = 'MyObject.md some_relevant_file.md'
    issues = '#1234 #1235 #1236'
  []
[]
```

It is also possible to provide a common value for 'design' and 'issues' at the top level of a test
specification as shown below. If 'design' or 'issues' appear again within a block the top level
values are overridden.

```
[Tests]
  design = 'MyObject.md'
  issues = '#12345'
  [one]
    type = CSVDiff
    input = one.i
    csvdiff = one_out.csv
    requirement = "The system shall must compute a value."
  []
  [two]
    type = RunException
    input = two.i
    expect_err = "You can not do that"
    requirement = "The system shall produce an error when the 'foo' parameter is wrong."
    issues = "#54321"
  []
[]
```

Multiple tests may be associated with a single requirement that is tested in
multiple ways, such as in 1D, 2D, and 3D. For this case, the tests within a
specification may be grouped into a single requirement with each test within the group providing
additional details via the "detail" parameter. It is important that the requirement text
and each of the details are a complete sentence when combined, as shown in the following example.

```
[Tests]
  [group] # this is an arbitrary name that is not used
    issues = "#1235"
    design = "MyObject.md"
    requirement = "The system shall solve the Laplace equation in:"
    [1D]
      type = Exodiff
      input = input_1D.i
      exodiff = input_1D_out.e

      detail = "in 1D,"
    []
    [2D]
      type = Exodiff
      input = input_2D.i
      exodiff = input_2D_out.e

      detail = "in 2D, and"
    []
    [3D]
      type = Exodiff
      input = input_3D.i
      exodiff = input_3D_out.e

      detail = "in 3D."
    []
  []
[]
```
