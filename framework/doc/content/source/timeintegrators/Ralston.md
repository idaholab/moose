# Ralston

!syntax description /Executioner/TimeIntegrator/Ralston

Ralston's time integration method is second order accurate in time. It is a two step explicit
method and a special case of the 2nd order Runge-Kutta method. It is obtained through an error minimization
process and has been shown to outperform other 2nd order explicit Runge Kunta methods, see [!cite](ralston1962).

## Description

With $U$ the vector of non linear variables and $A$ a non linear operator
describing the PDE of interest below:

!equation
\dfrac{\partial U(t)}{\partial t} = A(t, U(t))

Using $t+dt$ for the current time step and $t$ for the previous step,
Ralston's method can be written:

!equation
U(t+dt) = U(t) + \dfrac{dt}{4} \left(A(t, U(t)\right) +  \dfrac{3dt}{4} A \left(t + \dfrac{2dt}{3},U(t) + \dfrac{2dt}{3} A(t, U(t)) \right)

The Butcher tableau of the quadrature weights for this method is:

!table
| $c_i$ | $a_{i1}$ | $a_{i2}$ |
| - | - | - |
| 0 | 0 |
| 2/3 | 2/3 | 0 |
| $b_{j}$ | 1/4 | 3/4 |

!alert warning
All kernels except time-(derivative)-kernels should have the parameter `implicit=false` to use this
time integrator.

!alert warning
ExplicitRK2-derived TimeIntegrators [ExplicitMidpoint.md], [Heun.md], [Ralston.md]) and other multistage
TimeIntegrators are known not to work with Materials/AuxKernels that accumulate 'state' and
should be used with caution.

!syntax parameters /Executioner/TimeIntegrator/Ralston

!syntax inputs /Executioner/TimeIntegrator/Ralston

!syntax children /Executioner/TimeIntegrator/Ralston
