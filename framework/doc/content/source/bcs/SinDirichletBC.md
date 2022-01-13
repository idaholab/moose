# SinDirichletBC

!syntax description /BCs/SinDirichletBC

## Description

`SinDirichletBC` is a `NodalBC` which imposes a time-varying boundary value of the form

\begin{equation}
  g(t) = \left\{
  \begin{array}{ll}
    g_0 + (g_T - g_0) \sin \left( \frac{\pi t}{2T} \right), & 0 < t \leq T \\
    g_T, & t \geq T
  \end{array}
  \right.
\end{equation}

where $g_0$ and $g_T$ are the initial and final values of $g$, respectively,
and $T$ is the final time. These values are controlled by the
`initial`, `final`, and `duration` parameters, respectively.
The value is applied on one or more sidesets specified
by the `boundary` parameter and does not vary in space. This type of boundary
condition is applicable to time-varying PDEs, for example:

\begin{equation}
\begin{aligned}
  \frac{\partial u}{\partial t} -\nabla^2 u &= f && \quad \in \Omega \\
  u &= g(t) && \quad \in \partial \Omega
\end{aligned}
\end{equation}

and is frequently used to "ramp" a difficult boundary condition to its
final value over a short time interval, rather than imposing it
instantaneously at time $t=0$.  This approach can make nonlinear
solvers more robust by improving the initial guess used by the Newton
iterations, as well as preventing the solver from converging to
non-physical solutions in nonlinear PDEs.

## Example Input Syntax

!listing test/tests/bcs/sin_bc/sin_dirichlet_test.i block=BCs

!syntax parameters /BCs/SinDirichletBC

!syntax inputs /BCs/SinDirichletBC

!syntax children /BCs/SinDirichletBC
