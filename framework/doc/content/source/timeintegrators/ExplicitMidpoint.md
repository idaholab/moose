# ExplicitMidpoint

!syntax description /Executioner/TimeIntegrator/ExplicitMidpoint

The explicit midpoint method is second order accurate in time. It is a two step method and
a special case of the 2nd order Runge-Kutta method.

## Description

With $U$ the vector of non linear variables and $A$ a non linear operator
describing the PDE of interest below:

!equation
\dfrac{\partial U(t)}{\partial t} = A(t, U(t))

Using $t+dt$ for the current time step and $t$ for the previous step,
the explicit midpoint integration scheme can be written:

!equation
U(t+dt) = U(t) + dt A(t+dt/2, U(t) + \dfrac{dt}{2} A(t,U(t)))

The Butcher tableau of the quadrature weights for this method is:

!table
| $c_i$ | $a_{i1}$ | $a_{i2}$ |
| 0   | 0   | - |
| 1/2 | 1/2 | 0 |
| $b_j$ | 0 | 1 |

!alert warning
All kernels except time-(derivative)-kernels should have the parameter `implicit=false` to use this
time integrator.

!alert warning
ExplicitRK2-derived TimeIntegrators ([ExplicitMidpoint.md], [Heun.md], [Ralston.md]) and other multistage
TimeIntegrators are known not to work with Materials/AuxKernels that accumulate 'state' and
should be used with caution.

!syntax parameters /Executioner/TimeIntegrator/ExplicitMidpoint

!syntax inputs /Executioner/TimeIntegrator/ExplicitMidpoint

!syntax children /Executioner/TimeIntegrator/ExplicitMidpoint
