# MOOSE Input File Syntax

MOOSE uses [WASP](https://code.ornl.gov/neams-workbench/wasp), developed at Oak Ridge National Laboratory,
to process input files. Input files use a hierarchical syntax, with square brackets `[something]` to delineate levels.

```
                       # These are comments
[Variables]            # system name
  [u]                  # object name, opening sub-block
    family = LAGRANGE  # a parameter for this object
    order = FIRST      # another parameter for this object
  []                   # closing object sub-block
[]                     # closing system block
```

More info on input syntax on [this page](input_syntax.md).

!---

## Comments

Comments are preceded by `#`. They are ignored by the input file parsing.
They can be written on their own line:

```
# Let's define some variables!
[Variables]
  ...
```

or after valid syntax:

```
[Variables]  # Let's define some variables!
  ...
```

!---

## Input file variables

Variables can be defined in the input file, and substituted anywhere below their definition.

```
diff = 3

[Variables]
  [u]
    family = LAGRANGE
    initial_condition = ${diff}
  []
[]
```

!---

## Input file parsed expressions for basic math

We can have the parser perform simple math for us using the `${fparser <some math>}` syntax.
The available syntax can be found [on this page](http://warp.povusers.org/FunctionParser/fparser.html).

```
diff = 3
scale_diff = 2
offset_diff = 1

[Variables]
  [u]
    family = LAGRANGE
    initial_condition = ${fparse offset_diff + scale_diff * diff}  # = 1 + 2 * 3
  []
[]
```

!---

### Unit conversions in the input file

MOOSE supports both S.I. and imperial units. But everything must be consistent. Your input file,
your mesh, and your material properties must use the same unit system. To help adapt your input file,
we provide unit conversion capabilities.

```
length_in = 25
length_m = ${units 23 in -> m}

# we can still use input file variables too
length_meters = ${units ${length_in} in -> m}
```

A table of unit and units names is provided at this [link](utils/Units.md).

!---

## Global parameters

Global parameters are defined in a special block and are substituted everywhere in the input they can be.
This lets us reduce the size of the input file, at the cost these more implicit substitutions.

```
[GlobalParams]
  second_order = true # this will go in the [Mesh] block
  order = SECOND      # this will go in the each variable object
[]

[Variables]
  [u]
  # the order is being changed to SECOND without writing it down here
  []
  [v]
    # we don't want v to be second order
    order = FIRST
  []
[]
```

!alert warning
Be careful with global parameters with common parameter names such as `block` or `variable`. They
will apply to every single object, even the ones you did not think of.

!---

## Combining input files using "!include"

Input files can be concatenated using the `!include <other input file.i>` syntax.
This is helpful to:

- share common input syntax between multiple inputs (for example a steady and a transient simulations, sharing the same mesh)
- build long input files in multiple steps
