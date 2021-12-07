# CrankNicolson

!syntax description /Executioner/TimeIntegrator/CrankNicolson

The Crank Nicolson time integration scheme is a second-order implicit method. It is unconditionally stable.

## Description

With $U$, the vector of nonlinear variables, and $A$, a nonlinear operator,
we write the PDE of interest as:

!equation
\dfrac{\partial U}{\partial t} = A(t, U(t))

Using $t+\Delta t$ for the current time step, and $t$ for the previous step,
the Crank Nicolson time integration scheme can be written:

!equation
U(t+\Delta t) = U(t) + \Delta t \dfrac{1}{2} \left( A(t, U(t)) + A(t+\Delta t, U(t+\Delta t)) \right)

This method can be expressed as a Runge-Kutta method with the following Butcher Tableau:

!equation
\begin{array}{c|cc}
0 & 0 & 0 \\
1 & 1/2 & 1/2 \\
\hline
    &  1/2 & 1/2
\end{array}

!syntax parameters /Executioner/TimeIntegrator/CrankNicolson

!syntax inputs /Executioner/TimeIntegrator/CrankNicolson

!syntax children /Executioner/TimeIntegrator/CrankNicolson
