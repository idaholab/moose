# ExplicitMidpoint

!syntax description /Executioner/TimeIntegrator/ExplicitMidpoint

The explicit midpoint method is second-order accurate in time. It is a two-step method and
a special case of the 2nd-order Runge-Kutta method.

## Description

With $U$, the vector of nonlinear variables, and $A$, a nonlinear operator,
we write the PDE of interest as:

!equation
\dfrac{\partial U}{\partial t} = A(t, U(t))

Using $t+\Delta t$ for the current time step, and $t$ for the previous step,
the explicit midpoint integration scheme can be written:

!equation
U(t+\Delta t) = U(t) + \Delta t A(t+\Delta t/2, U(t) + \dfrac{\Delta t}{2} A(t,U(t)))

This method can be expressed as a Runge-Kutta method with the following Butcher Tableau:

!equation
\begin{array}{c|cc}
  0 & 0 \\
1/2 & 1/2 & 0 \\
\hline
    &  0  & 1
\end{array}

!alert warning
All kernels except time-(derivative)-kernels should have the parameter `implicit=false` to use this
time integrator.

!alert warning
ExplicitRK2-derived TimeIntegrators ([ExplicitMidpoint.md], [Heun.md], [Ralston.md]) and other multistage
TimeIntegrators are known not to work with Materials/AuxKernels that accumulate 'state' and
should be used with caution.

!syntax parameters /Executioner/TimeIntegrator/ExplicitMidpoint

!syntax inputs /Executioner/TimeIntegrator/ExplicitMidpoint

!syntax children /Executioner/TimeIntegrator/ExplicitMidpoint
