# ADConservativeAdvectionBC

## Description

The `ADConservativeAdvectionBC` boundary condition pairs with the
[ADConservativeAdvection.md] kernel. It can be used for both Dirichlet and
"implicit" boundary conditions which use only information from the equation
systems solution vectors. Velocity must be provided, either through the
`velocity_mat_prop` parameter (implicit information) or through the
`velocity_function` parameter (Dirichlet information). Similarly, for Dirichlet
conditions, a `primal_dirichlet_value` should be supplied. Otherwise the
advected quantity will be determined from implicit information either through
the supplied `advected_quantity` or if that is not supplied, the `variable`
solution. If `primal_dirichlet_value` is supplied, then a `primal_coefficient`
material property name may be supplied which will multiply the
`primal_dirichlet_value`.

An example of this boundary condition's use is shown in the listing below for
both an inlet and outlet condition. At the inlet (`boundary = left`) both the velocity and primal
value (the `variable` `u` in this case) are prescribed. At the outlet
(`boundary= right`) due to the absence of `primal_dirichlet_value`, the current
solution value of `u` is used. Additionally, the velocity is also determined
implicitly through `velocity_mat_prop`.

!listing test/tests/dgkernels/passive-scalar-channel-flow/test.i block=BCs

!syntax parameters /BCs/ADConservativeAdvectionBC

!syntax inputs /BCs/ADConservativeAdvectionBC

!syntax children /BCs/ADConservativeAdvectionBC
