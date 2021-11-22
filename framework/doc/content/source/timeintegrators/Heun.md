# Heun

!syntax description /Executioner/TimeIntegrator/Heun

Heun's (aka improved Euler) time integration method is second order accurate in time. It is a two step explicit
method and a special case of the 2nd order Runge-Kutta method.

## Description

With $U$ the vector of non linear variables and $A$ a non linear operator
describing the PDE of interest below:

!equation
\dfrac{\partial U(t)}{\partial t} = A(t, U(t))

Using $t+dt$ for the current time step and $t$ for the previous step,
Heun's method can be written:

!equation
U(t+dt) = U(t) + \dfrac{dt}{2} \left(A(t, U(t)) +  A(t,U(t) + dt A(t, U(t)) ) \right)

The Butcher tableau of the quadrature weights for this method is:

!table
0   | 0
1   | 1    0
---------------------
    | 1/2  1/2

!alert warning
All kernels except time-(derivative)-kernels should have the parameter `implicit=false` to use this
time integrator.

!alert warning
ExplicitRK2-derived TimeIntegrators (ExplicitMidpoint, Heun, Ralston) and other multistage
TimeIntegrators are known not to work with Materials/AuxKernels that accumulate 'state' and
should be used with caution.

!syntax parameters /Executioner/TimeIntegrator/Heun

!syntax inputs /Executioner/TimeIntegrator/Heun

!syntax children /Executioner/TimeIntegrator/Heun
