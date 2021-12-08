# BDF2

!syntax description /Executioner/TimeIntegrator/BDF2

The backwards differencing formula of second order, BDF-2, is a linear, two-step, second-order method. It is
A-stable.

## Description

With $U$, the vector of nonlinear variables, and $A$, a nonlinear operator,
we write the PDE of interest as:

!equation
\dfrac{\partial U}{\partial t} = A(t, U(t))

Using $t+2\Delta t$ for the current time step, $t+\Delta t$ for the previous step, and $t$ for the one before that,
BDF-2 can be written:

!equation
U(t+2\Delta t) = \dfrac{4}{3}U(t+\Delta t) - \dfrac{1}{3}U(t) + \dfrac{2}{3} \Delta t A \left(t + 2\Delta t, U(t + 2\Delta t) \right)

!syntax parameters /Executioner/TimeIntegrator/BDF2

!syntax inputs /Executioner/TimeIntegrator/BDF2

!syntax children /Executioner/TimeIntegrator/BDF2
