# ImplicitMidpoint

!syntax description /Executioner/TimeIntegrator/ImplicitMidpoint

The implicit midpoint method is second order accurate. As a Gauss Legendre method it is A-stable.

## Description

With $U$ the vector of non linear variables and $A$ a non linear operator
describing the PDE of interest below:

!equation
\dfrac{\partial U(t)}{\partial t} = A(t, U(t))

Using $t+dt$ for the current time step and $t$ for the previous step,
the implicit midpoint integration scheme can be written:

!equation
U(t+dt) = U(t) + dt A \left(t+dt/2, \dfrac{dt}{2} \left( U(t) +  U(t+dt) \right) \right)


The Butcher tableau of the quadrature weights for this method is:
!table
1/2 | 1/2 |
---------------------
    |  1

!syntax parameters /Executioner/TimeIntegrator/ImplicitMidpoint

!syntax inputs /Executioner/TimeIntegrator/ImplicitMidpoint

!syntax children /Executioner/TimeIntegrator/ImplicitMidpoint
