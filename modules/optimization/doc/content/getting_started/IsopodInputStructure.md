# Input File Structure

## Basic Syntax

The input file used by Isopod (or any MOOSE application) is broken into
sections or blocks identified with square brackets. The type of input block is
placed in the opening brackets, and empty brackets mark the end of the block.

```text
[BlockName]
    <block lines and subblocks>
[]
```

Each block may contain an arbitrary number of line commands to define
parameters related to that block. They can also optionally contain one
or more subblocks, which may in turn contain their own nested subblocks.
Subblocks are opened and closed with the following syntax:

```text
[./subblock_name]
    <line commands>
[../]
```

Note that all subblocks contained within a given block must have unique
names (within the opening [] brackets).

Parameters are defined with line commands, and are given as key/value
pairs separated by an equals sign (=). They specify parameters to be
used by the object being described.  The key is a string (no
whitespace), and the value may be a string, an integer, a real number,
or a list of strings, integers, or real numbers.  Lists are given
in single quotes and are separated by whitespace.

The following are examples of line commands for a single parameter and
for a list of parameters:

```text
single_parameter = 2.3
list_of_parameters = '1.0 2.3 3.7'
```

Blocks and subblocks at any level can contain line commands, which must
be appropriate for the scope of the block containing them. Most of the
time, blocks are used to create instances of MOOSE objects, and contain
a ```type = ``` parameter to specify the type of MOOSE object to be
created. The name of the MOOSE object specified in the parameter
corresponds to the name of the class in the C++ source code.

Each object type has a unique set of input parameters that are valid for
specifying the behavior of that object. Some parameters are required,
and some are optional, and revert to default behavior if they are not
specified.  An error message is generated if a line command does not
apply within the scope in which it is provided. Repeating a line within
a block also results in an error.

In this document, line commands are shown with the keyword, an equal
sign, and, in angle brackets, the value. If a default value exists for
that line command, it is shown in parentheses.

In the initial description of a block, line commands common to all
subblocks will be described. Those line commands are then omitted from
the description of the subblocks but are nonetheless valid line commands
for those subblocks.

The name of a subblock (`[./<name>]`) is arbitrary. However, these
names should be chosen to be meaningful because they can be used to
refer to those entities elsewhere in the input file. Not every created
entity is referenced elsewhere, but a name must be created for every
entity regardless.

## Summary of MOOSE Object Types

MOOSE is an objected-oriented system with well-defined interfaces for
applications to define their own physics-specific code modules. The
following is a listing of the major types of MOOSE objects used by Isopod:

- Variable
- Kernel
- AuxVariable
- AuxKernel
- Material
- BoundaryCondition
- Function
- Postprocessor
- VectorPostprocessor
- Constraint
- Damper

Specialized versions of these object types are implemented to provide
the functionality needed to model physics of interest for Isopod.

## Isopod Syntax Page

A complete listing of all input syntax options in MOOSE is available on
[the MOOSE Documentation page](http://mooseframework.org/documentation/).
See the section on Input File Documentation. Note also that you can run

```text
isopod-opt --dump
```

to get a list of valid input options for Isopod.

## Units

Isopod can be run using any unit system preferred by the
user. Empirical models within Isopod that depend on a specific unit
system are noted in this documentation.
