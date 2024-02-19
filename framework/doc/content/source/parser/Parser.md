# Parser

The MOOSE Parser object is responsible for interacting with the HIT input file
parser. It is important to note that +this+ object is not responsible for the raw
file-base I/O. The underlying structure of the MOOSE input file is dictated by
HIT and information on the format can be found
[here](/application_usage/input_syntax.md optional=True). The parser abstraction
expects information to be organized into a hierarchy of blocks with zero or more
children at each level. It performs as much error checking as early as possible before
building any MOOSE objects.

## Includes

See [Includes](input_syntax.md optional=True)

## Multiple inputs

When multiple inputs are supplied to a MOOSE application, the parser will read
them in successively and add merge them into a single block hierarchy. Later
inputs add to and override parameters to previous inputs. This permits the user
to factor out common parts of a set of inputs and reuse them in multiple
simulations.
