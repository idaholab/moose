# CompositeFunction

!syntax description /Functions/CompositeFunction

## Description

The `CompositeFunction` type takes an arbitrary set of functions, provided in
the `functions` parameter, evaluates each of them at the appropriate time
and position, and multiplies them together.  The function can optionally be
multiplied by a scale factor, which is specified using the `scale_factor`
parameter.

## Example Input Syntax

!listing test/tests/bcs/function_dirichlet_bc/test.i block=Functions

!syntax parameters /Functions/CompositeFunction

!syntax inputs /Functions/CompositeFunction

!syntax children /Functions/CompositeFunction
