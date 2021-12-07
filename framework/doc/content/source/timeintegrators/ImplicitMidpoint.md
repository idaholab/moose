# ImplicitMidpoint

!syntax description /Executioner/TimeIntegrator/ImplicitMidpoint

The implicit midpoint method is second-order accurate. As a Gauss-Legendre method it is A-stable.

## Description

With $U$, the vector of nonlinear variables, and $A$, a nonlinear operator,
we write the PDE of interest as:

!equation
\dfrac{\partial U}{\partial t} = A(t, U(t))

Using $t+\Delta t$ for the current time step, and $t$ for the previous step,
the implicit midpoint integration scheme can be written:

!equation
U(t+\Delta t) = U(t) + \Delta t A \left(t+\Delta t/2, \dfrac{\Delta t}{2} \left( U(t) +  U(t+\Delta t) \right) \right)


This method can be expressed as a Runge-Kutta method with the following Butcher Tableau:

!equation
\begin{array}{c|c}
  1/2 & 1/2 \\
\hline
    &  1
\end{array}

!syntax parameters /Executioner/TimeIntegrator/ImplicitMidpoint

!syntax inputs /Executioner/TimeIntegrator/ImplicitMidpoint

!syntax children /Executioner/TimeIntegrator/ImplicitMidpoint
