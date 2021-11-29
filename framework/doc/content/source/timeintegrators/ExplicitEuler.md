# ExplicitEuler

!syntax description /Executioner/TimeIntegrator/ExplicitEuler

The explicit Euler method is only first order accurate in time.

## Description

With $U$ the vector of non linear variables and $A$ a non linear operator
describing the PDE of interest below:

!equation
\dfrac{\partial U(t)}{\partial t} = A(t, U(t))

Using $t+dt$ for the current time step and $t$ for the previous step,
the explicit Euler method can be written:

!equation
U(t+dt) = U(t) + dt A(t, U(t))

The Butcher tableau of the quadrature weights for this method is:
!table
| 0 | 0 |
| - | - |
| - | 1 |

!alert note
The [ActuallyExplicitEuler.md] implements the same algorithm but without forming the non linear system,
making it faster and use less memory.

!alert warning
All kernels except time-(derivative)-kernels should have the parameter `implicit=false` to use this
time integrator.

!syntax parameters /Executioner/TimeIntegrator/ExplicitEuler

!syntax inputs /Executioner/TimeIntegrator/ExplicitEuler

!syntax children /Executioner/TimeIntegrator/ExplicitEuler
