# VacuumBC

!syntax description /BCs/VacuumBC

Vacuum is defined as no incoming flux from outside the boundary. This is a
common approximation in reactor physics where beyond a certain distance from a
nuclear system, we assume that while neutrons may still leave the system, they will
not come back.

The contribution to the residual is:

!equation
\int_{\partial \Omega} \alpha u(\vec{r}) \psi_t(\vec{r}) / 2. dS

where $\partial \Omega$ is the domain definition and $\psi_t$ is the test function.

## Example input syntax

In this input, a `VacuumBC` is used to impose a 0 incoming flux boundary condition in
a diffusion problem in a RZ geometry problem on the `top` boundary for variable `u`.

!listing test/tests/coord_type/coord_type_rz_integrated.i block=BCs

!syntax parameters /BCs/VacuumBC

!syntax inputs /BCs/VacuumBC

!syntax children /BCs/VacuumBC
