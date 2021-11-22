# CrankNicolson

!syntax description /Executioner/TimeIntegrator/CrankNicolson

The Crank Nicholson time integration scheme is a second order implicit method. It is unconditionally stable.

## Description

With $U$ the vector of non linear variables and $A$ a non linear operator
describing the PDE of interest below:

!equation
\dfrac{\partial U(t)}{\partial t} = A(t, U(t))

Using $t+dt$ for the current time step and $t$ for the previous step,
the implicit Euler time integration scheme can be written:

!equation
U(t+dt) = U(t) + dt \dfrac{1}{2} \left( A(t, U(t)) + A(t+dt, U(t+dt)) \right)

!syntax parameters /Executioner/TimeIntegrator/CrankNicolson

!syntax inputs /Executioner/TimeIntegrator/CrankNicolson

!syntax children /Executioner/TimeIntegrator/CrankNicolson
