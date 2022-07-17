# Solving FEM/FVM Discretizations

!---

## Newton's Method

Newton's method is a "root finding" method with good convergence properties, in "update form",
for finding roots of a scalar equation it is defined as:
$f(x)=0$, $f(x): \mathbb{R} \rightarrow \mathbb{R}$ is given by

!equation
\begin{aligned}
f'(x_n) \delta x_{n+1} &= -f(x_n) \\
x_{n+1} &= x_n + \delta x_{n+1}
\end{aligned}

!---

## Newton's Method in MOOSE

The residual, $\vec{R}_i(u_h)$, as defined by [example_weak_residual] is a nonlinear system of equations,

!equation
\vec{R}_i(u_h)=0, \qquad i=1,\ldots, N,

that is used to solve for the coefficients $u_j, j=1,\ldots,N$.

For this system of nonlinear equations Newton's method is defined as:

!equation id=newton
\begin{aligned}
\mathbf{J}(\vec{u}_n) \delta\vec{u}_{n+1} &= -\vec{R}(\vec{u}_n) \\
\vec{u}_{n+1} &= \vec{u}_n + \delta\vec{u}_{n+1}
\end{aligned}

where $\mathbf{J}(\vec{u}_n)$ is the Jacobian matrix evaluated at the current iterate:

!equation
J_{ij}(\vec{u}_n) = \frac{\partial \vec{R}_i(\vec{u}_n)}{\partial u_j}

!---

## MOOSE Solve Types

The solve type is specified in the `[Executioner]` block within the input file:

```
[Executioner]
  solve_type = NEWTON
```

Available options include:

- +PJFNK+: Preconditioned Jacobian Free Newton Krylov (default)
- +JFNK+: Jacobian Free Newton Krylov
- +NEWTON+: Performs solve using exact Jacobian for preconditioning (recommended
  for most FV simulations)
- +FD+: PETSc computes terms using a finite difference method (debug)

!---

## Automatic Jacobian Calculation

MOOSE uses forward mode automatic differentiation from the MetaPhysicL package.

Moving forward, the idea is for application developers to be able to develop entire apps without
writing a single Jacobian statement. This has the potential to +decrease application development
time+.

In terms of computing performance, presently AD Jacobians are slower to compute than hand-coded
Jacobians, but they parallelize extremely well and can benefit from using a `NEWTON` solve, which
often results in decreased solve time overall.

!---
