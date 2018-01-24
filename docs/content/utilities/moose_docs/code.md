# Documenting MOOSE Code

MOOSE and MOOSE-based applications generally focus on creating C++ objects (e.g., Kernels, BCs, etc.), so it is important that these object are documented to allow other developers and users to understand the purpose of these objects. The [MOOSE documentation system](utilities/moose_docs/index.md) aims to make documenting objects straightforward and natural. Moreover, accessing the object documentation is simple and allows for documentation to remain up-to-date as the code continues to advance.

There are two primary locations that documentation should be added for code: in the source code itself and in markdown detailed description files.


## Source Code Documentation

The first step is to add documentation for your application in the `validParams` function. This is done by adding a
and parameter documentation strings and calling the class description method.

A description of each parameter should also be provided when they are added with the various add methods
of the `InputParameters` object. For example, in the [FunctionIC](/Variables/InitialCondition/framework/FunctionIC.md)
the following parameter documentation is added, which is then present in the parameter summary table of the
generated site.

!listing framework/src/ics/FunctionIC.C line=addRequiredParam

The string supplied in this function will appear in the parameter tables within the documentation that is generated.
For example, see the parameter table for the [DirichletBC object](framework/DirichletBC.md).

Secondly, a short description should be supplied in each `addParam`, `addPrivateParam`, etc. function in your code. For
example, in the [Diffusion](framework/Diffusion.md) object the following class
description.

!listing framework/src/kernels/Diffusion.C line=addClassDescription

When the documentation for this object is generated this string is added to the first portion of the page and the
system overview table. For example, the [Kernels overview](systems/Kernels/index.md) includes a table with each object
listed; the table includes the class description from the source code.

## Object and System Descriptions

A detailed theoretical description should be provided in addition to the generated, in-code documentation for an object by creating a markdown file using [MOOSE Flavored Markdown](moose_markdown/index.md). The created file must be stored in a file named according to the
registered MOOSE syntax within the "install" directory explained in the [Configuration](moose_docs/setup.md#configuration) section. For example, the details for the [Diffision](framework/Diffusion.md) are in the `framework/docs/content/framework/systems/Kernels/Diffusion.md` file.

Stubs (minimal markdown files) for new classes can be generated using the "moosedocs.py" script; these stubs include a suggested template for new markdown documentation.
See the [Instructions on Generating Documentation](moose_docs/generate.md) for details.


## Checking Documentation

It is possible to check the status of your applications documentation. This check will examine your code and provide a list of errors for un-documentented items including a missing class description or a missing object or system details file. To perform the check utilize the "moosedocs.py" script with the check command as follows:

```text
cd ~/projects/moose/docs
./moosedocs.py check
```
