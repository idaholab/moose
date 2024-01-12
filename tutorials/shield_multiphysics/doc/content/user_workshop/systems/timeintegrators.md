# [Time Integrator System](syntax/Executioner/TimeIntegrator/index.md)

A system for defining schemes for numerical integration in time.

!---


The TimeIntegrator can be set using "scheme" parameter within the `[Executioner]` block, if
the "type = Transient", the following options exist:

- `implicit-euler`: Backward Euler (default)
- `bdf2`: Second order backward difference formula
- `crank-nicolson`: Second order Crank-Nicolson method
- `dirk`: Second order Diagonally-Implicit Runge-Kutta (DIRK)
- `newmark-beta`: Second order Newmark-beta method (structural dynamics)

!---

## TimeIntegrator Block

It is also possible to specify a time integrator as a separate sub-block within the input file.
This allows for additional types and parameters to be defined, including custom TimeIntegrator
objects.

!listing newmark_beta_prescribed_parameters.i block=Executioner

!---

## Convergence Rates

Consider the test problem:

!equation
\begin{array}{rl}
\frac{\partial u}{\partial t} - \nabla^2 u &= f
\\
u(t=0)&= u_0
\\
\left. u \right|_{\partial \Omega} &= u_D
\end{array}

for $t=(0,T]$, and $\Omega=(-1,1)^2$, $f$ is chosen so the exact solution is given by
$u = t^3 (x^2 + y^2)$ and $u_0$ and $u_D$ are the initial and Dirichlet boundary conditions
corresponding to this exact solution.

!---

!media darcy_thermo_mech/time_convergence_implicit.png style=width:90%
