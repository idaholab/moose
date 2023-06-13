# DiracKernels System

Dirac Kernels are used to apply sources/loads at desired point locations in the mesh. The value of the loads is a user-input
value. It is applied at the user-defined location and is effectively null outside of that location, hence the `Dirac` name.

The contribution to the residual is computing by evaluating, at each source location, the product of the test function value at
that location by the value of the source/load.

## Notable parameters

By default, the locations of the sources may not change during the simulation. If a source is detected as moving,
the system will error and prompt the user to provide the `allow_moving_sources` parameter.

By default, only one source may provided at each location in the mesh, any additional source at the same location
is automatically dropped. If multiple sources should share the same location, the user should provide the
`drop_duplicate_points`.

## Example input syntax

In this example, three `ConstantPointSource` are being applied to variable `u` with values 0.1 / -0.1 and -1
at position (0.2 0.3 0.0) / (0.2 0.8 0) and (0.8 0.5 0.8) respectively. `u` is solution to a diffusion equation
with those three sources.

!listing test/tests/dirackernels/constant_point_source/3d_point_source.i block=DiracKernels

!syntax list /DiracKernels objects=True actions=False subsystems=False

!syntax list /DiracKernels objects=False actions=False subsystems=True

!syntax list /DiracKernels objects=False actions=True subsystems=False
