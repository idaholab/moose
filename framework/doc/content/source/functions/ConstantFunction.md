# ConstantFunction

!syntax description /Functions/ConstantFunction

The `value` parameter of the `ConstantFunction` is controllable, so it may modified
during the simulation using the [Controls system](syntax/Controls/index.md).

## Example input syntax

This example uses a `ConstantFunction` to feed into vectorized spatial samplers, which
sample the function in specified locations. These samples are then compared using
another postprocessor. The `ConstantFunction` sampled anywhere in time and space will
return its constant value.

!listing test/tests/postprocessors/vector_postprocessor_comparison/vector_postprocessor_comparison.i block=Functions

!syntax parameters /Functions/ConstantFunction

!syntax inputs /Functions/ConstantFunction

!syntax children /Functions/ConstantFunction
