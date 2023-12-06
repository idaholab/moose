# Builder

The MOOSE Builder object is responsible for constructing the necessary [Actions](Action.md) that build the objects which compose a complete MOOSE based simulation.
It is important to note that +this+ object is not responsible for the raw
file-base I/O and parsing. The builder abstraction expects information to be parsed first and organized into a hierarchy of blocks with zero or more children at each level. All of the name/value pairs at each level are used to construct one or more complete objects.

## Associating parser blocks

The Builder works by using a list of registered syntax blocks with C++ types. Each
time the Builder is handed a block, it will consult the registered syntax for a list
of objects to build. MOOSE is delivered with a very low-level list of syntax where
each type of object is exposed directly to the input file by system type (e.g.
Kernels, BCs, Outputs, etc). Each application that builds upon MOOSE can augment
or replace this syntax completely. The registered list of syntax can be viewed
on the mooseframework.org website, or can be dumped on the command-line in a number
of different formats. The registration itself occurs in [Moose.C](Moose.md).

## Parameter Substitution

See [Brace Expressions](input_syntax.md optional=True)

## Active/Inactive block parameters

The Builder supports the ability to selectively activate or deactivate individual blocks
without the use of block comment characters through the use of the special "active" and
"inactive" parameter blocks. Either of these parameters (but not both) can be added
to any block (including the top level) to selectively turn individual blocks on/off.
