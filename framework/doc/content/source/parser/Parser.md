# Parser

The MOOSE Parser object is responsible for interacting with the HIT input file
parser and constructing the necessary [Actions](Action.md) that ultimately
execute to build up the objects which compose a complete MOOSE based simulation.
It is important to note that +this+ object is not responsible for the raw
file-base I/O. The underlying structure of the MOOSE input file is dictated by
HIT and information on the format can be found
[here](/application_usage/input_syntax.md optional=True). The parser abstraction
expects information to be organized into a hierarchy of blocks with zero or more
children at each level. All of the name/value pairs at each level are used to
construct one or more complete objects.

## Associating parser blocks

The Parser works by using a list of registered syntax blocks with C++ types. Each
time the Parser is handed a block, it will consult the registered syntax for a list
of objects to build. MOOSE is delivered with a very low-level list of syntax where
each type of object is exposed directly to the input file by system type (e.g.
Kernels, BCs, Outputs, etc). Each application that builds upon MOOSE can augment
or replace this syntax completely. The registered list of syntax can be viewed
on the mooseframework.org website, or can be dumped on the command-line in a number
of different formats. The registration itself occurs in [Moose.C](Moose.md).

## Parameter Substitution

See [Brace Expressions](input_syntax.md optional=True)

## Active/Inactive block parameters

The Parser supports the ability to selectively activate or deactivate individual blocks
without the use of block comment characters through the use of the special "active" and
"inactive" parameter blocks. Either of these parameters (but not both) can be added
to any block (including the top level) to selectively turn individual blocks on/off.

## Multiple inputs

When multiple inputs are supplied to a MOOSE application, the parser will read
them in successively and add merge them into a single block hierarchy. Later
inputs add to and override parameters to previous inputs. This permits the user
to factor out common parts of a set of inputs and reuse them in multiple
simulations.
