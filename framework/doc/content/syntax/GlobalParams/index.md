# GlobalParams System

## Overview

The global parameters system is used to define global parameter values in an
input file. Every parameter in the `GlobalParams` block will be inserted into
every block/sub-block where that parameter name is defined. This can be a
useful mechanism of reducing duplication in input files.

!alert warning title=Unintended substitutions
Be very careful when using the `GlobalParams` block that you do not accidentally
insert parameter values where you do not intend, as these errors can be
difficult to find. Be particularly wary of parameter names that seem like they
could be very general to a number of different objects and systems.

## Example Input File Syntax

Suppose you have a number of similar objects that share a common parameter
`my_common_parameter`. Then instead of having to list this parameter for each
of your objects:

```
[SomeSystem]
  [objA]
    type = SomeClass
    my_common_parameter = some_value
  []
  [objB]
    type = SomeClass
    my_common_parameter = some_value
  []
[]
```

you can instead list the parameter in the `GlobalParams` block, thus eliminating
some duplication in the input file:

```
[GlobalParams]
  my_common_parameter = some_value
[]

[SomeSystem]
  [objA]
    type = SomeClass
  []
  [objB]
    type = SomeClass
  []
[]
```

Note that the parameter need not belong to the same class or even the same
system; the `GlobalParams` block will insert its parameters into every possible
occurrence of that parameter name in the input file.

If any block/sub-block ever needs a different value than the global value,
then it can simply locally override the value:

```
[GlobalParams]
  my_common_parameter = some_value
[]

[SomeSystem]
  [objA]
    type = SomeClass
  []
  [objB]
    type = SomeClass
    my_common_parameter = some_different_value
  []
[]
```

!syntax list /GlobalParams objects=True actions=False subsystems=False

!syntax list /GlobalParams objects=False actions=False subsystems=True

!syntax list /GlobalParams objects=False actions=True subsystems=False
