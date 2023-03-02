# VectorConstantPointSource

!syntax description /DiracKernels/VectorConstantPointSource

This applies a load in a single location in the mesh. The value field is controllable,
so the [Control system](syntax/Controls/index.md) may be leveraged to control the load during the
simulation.

## Example input syntax

In this example, three `VectorConstantPointSource` are being applied to variable
`u` with values (0.1 0.1 0.1) / (-0.1 -0.1 -0.1) and (-1 -1 -1)
at position (0.2 0.3 0.0) / (0.2 0.8 0) and (0.8 0.5 0.8) respectively. `u` is
solution to a vector diffusion equation
with those three sources.

!listing test/tests/dirackernels/constant_point_source/vector_3d_point_source.i block=DiracKernels

!syntax parameters /DiracKernels/VectorConstantPointSource

!syntax inputs /DiracKernels/VectorConstantPointSource

!syntax children /DiracKernels/VectorConstantPointSource
