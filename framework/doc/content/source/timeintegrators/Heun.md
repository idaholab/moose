# Heun

!syntax description /Executioner/TimeIntegrator/Heun

Heun's (or "improved Euler") time integration method is second-order accurate in time. It is a two-step explicit
method and a special case of the 2nd-order Runge-Kutta method.

## Description

With $U$, the vector of nonlinear variables, and $A$, a nonlinear operator,
we write the PDE of interest as:

!equation
\dfrac{\partial U}{\partial t} = A(t, U(t))

Using $t+\Delta t$ for the current time step, and $t$ for the previous step,
Heun's method can be written:

!equation
U(t+\Delta t) = U(t) + \dfrac{\Delta t}{2} \left(A(t, U(t)) +  A(t,U(t) + \Delta t A(t, U(t)) ) \right)

This method can be expressed as a Runge-Kutta method with the following Butcher Tableau:

!equation
\begin{array}{c|cc}
  0 & 0 \\
1 & 1 & 0 \\
\hline
    &  1/2  & 1/2
\end{array}

!alert warning
All kernels except time-(derivative)-kernels should have the parameter `implicit=false` to use this
time integrator.

!alert warning
ExplicitRK2-derived TimeIntegrators ([ExplicitMidpoint.md], [Heun.md], [Ralston.md]) and other multistage
TimeIntegrators are known not to work with Materials/AuxKernels that accumulate 'state' and
should be used with caution.

!syntax parameters /Executioner/TimeIntegrator/Heun

!syntax inputs /Executioner/TimeIntegrator/Heun

!syntax children /Executioner/TimeIntegrator/Heun
