# ExplicitEuler

!syntax description /Executioner/TimeIntegrator/ExplicitEuler

The explicit Euler method is only first-order accurate in time.

## Description

With $U$, the vector of nonlinear variables, and $A$, a nonlinear operator,
we write the PDE of interest as:

!equation
\dfrac{\partial U}{\partial t} = A(t, U(t))

Using $t+\Delta t$ for the current time step, and $t$ for the previous step,
the explicit Euler method can be written:

!equation
U(t+\Delta t) = U(t) + \Delta t A(t, U(t))

This method can be expressed as a Runge-Kutta method with the following Butcher Tableau:

!equation
\begin{array}{c|c}
  0 & 0 \\
\hline
    &  1
\end{array}

!alert note
The [ActuallyExplicitEuler.md] implements the same algorithm but without forming the non linear system,
making it faster and use less memory.

!alert warning
All kernels except time-(derivative)-kernels should have the parameter `implicit=false` to use this
time integrator.

!syntax parameters /Executioner/TimeIntegrator/ExplicitEuler

!syntax inputs /Executioner/TimeIntegrator/ExplicitEuler

!syntax children /Executioner/TimeIntegrator/ExplicitEuler
