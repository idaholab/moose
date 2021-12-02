# Ralston

!syntax description /Executioner/TimeIntegrator/Ralston

Ralston's time integration method is second-order accurate in time. It is a two-step explicit
method and a special case of the 2nd-order Runge-Kutta method. It is obtained through an error minimization
process and has been shown to outperform other 2nd-order explicit Runge-Kutta methods, see [!cite](ralston1962).

## Description

With $U$, the vector of nonlinear variables, and $A$, a nonlinear operator,
we write the PDE of interest as:

!equation
\dfrac{\partial U}{\partial t} = A(t, U(t))

Using $t+\Delta t$ for the current time step and $t$ for the previous step,
Ralston's method can be written:

!equation
U(t+\Delta t) = U(t) + \dfrac{\Delta t}{4} \left(A(t, U(t)\right) +  \dfrac{3\Delta t}{4} A \left(t + \dfrac{2\Delta t}{3},U(t) + \dfrac{2\Delta t}{3} A(t, U(t)) \right)

This method can be expressed as a Runge-Kutta method with the following Butcher Tableau:

!equation
\begin{array}{c|cc}
0 & 0 \\
2/3 & 2/3 & 0 \\
\hline
    &  1/4 & 3/4
\end{array}

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
