# FVConstantScalarOutflowBC

!syntax description /FVBCs/FVConstantScalarOutflowBC

The scalar field is advected at a constant velocity. This boundary condition is used
in the equation solving for the scalar field. It is only to be defined on boundaries
that are downstream of the advecting velocity.

The order of accuracy of this boundary condition will depend on the interpolation method
for the boundary value chosen for the variable. Currently, second order accuracy may
be obtained by specifying `two_term_boundary_expansion = true` in the variable definition.

!alert note
This boundary condition assumes a constant velocity, so it may not be used in flow simulations
where the velocity is a variable, such as in the Navier Stokes module.

## Example input syntax

In this example, the `v` field is advected by a `1 0.5 0` velocity in the 2D plane. The
boundary condition for outflow is only defined on the boundaries that are downstream of
this velocity: `right` and `top`.

!listing test/tests/fvkernels/fv_constant_scalar_advection/2D_constant_scalar_advection.i block=FVBCs

!syntax parameters /FVBCs/FVConstantScalarOutflowBC

!syntax inputs /FVBCs/FVConstantScalarOutflowBC

!syntax children /FVBCs/FVConstantScalarOutflowBC
