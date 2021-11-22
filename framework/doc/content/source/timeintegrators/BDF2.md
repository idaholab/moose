# BDF2

!syntax description /Executioner/TimeIntegrator/BDF2

The backwards differencing formula of second order is a linear two-step second order method. It is
A-stable.

## Description

With $U$ the vector of non linear variables and $A$ a non linear operator
describing the PDE of interest below:

!equation
\dfrac{\partial U(t)}{\partial t} = A(t, U(t))

Using $t+2dt$ for the current time step, $t+dt$ for the previous step amd $t$ for the one before that,
the second order backwards differencing time integration scheme can be written:

!equation
U(t+2dt) = \dfrac{4}{3}U(t+dt) - \dfrac{1}{3}U(t) + \dfrac{2}{3} dt A \left(t + 2dt, U(t + 2dt) \right)

!syntax parameters /Executioner/TimeIntegrator/BDF2

!syntax inputs /Executioner/TimeIntegrator/BDF2

!syntax children /Executioner/TimeIntegrator/BDF2
