# FunctionDiracSource

!syntax description /DiracKernels/FunctionDiracSource

This applies a load in a single location in the mesh. The intensity of the source is controlled by a
[`Function`](syntax/Functions/index.md), which can have both spatial and temporal variations.

## Example input syntax

In this example of a time-dependent diffusion problem, the source is provided by a `FunctionDiracSource`
at the point `(0.1 0.2 0)` and its intensity is controlled by the `switch_off` function.

!listing test/tests/dirackernels/function_dirac_source/function_dirac_source.i block=Functions DiracKernels

!syntax parameters /DiracKernels/FunctionDiracSource

!syntax inputs /DiracKernels/FunctionDiracSource

!syntax children /DiracKernels/FunctionDiracSource
