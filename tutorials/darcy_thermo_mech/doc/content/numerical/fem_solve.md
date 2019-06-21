# Numerical Implementation

!---

## Numerical Integration

The only remaining non-discretized parts of the weak form are the integrals. First, split the domain
integral into a sum of integrals over elements:

!equation id=ref_elems
\int_{\Omega} f(\vec{x}) \;\text{d}\vec{x} = \sum_e \int_{\Omega_e} f(\vec{x}) \;\text{d}\vec{x}

Through a change of variables, the element integrals are mapped to integrals over the "reference"
elements $\hat{\Omega}_e$.

!equation
\sum_e \int_{\Omega_e} f(\vec{x}) \;\text{d}\vec{x} =
\sum_e \int_{\hat{\Omega}_e} f(\vec{\xi}) \left|\mathcal{J}_e\right| \;\text{d}\vec{\xi},

where $\mathcal{J}_e$ is the Jacobian of the map from the physical element to the reference element.

!---

## Reference Element (Quad9)

!media fem_quad9_ref.png style=width:100%;margin-left:auto;margin-right:auto;display:block;

!---

### Quadrature

Quadrature, typically "Gaussian quadrature", is used to approximate the reference element integrals
numerically.

!equation
\int_{\Omega} f(\vec{x})\;d\vec{x} \approx \sum_q w_q f(\vec{x}_q),

where $w_q$ is the weight function at quadrature point $q$.

Under certain common situations, the quadrature approximation is exact. For example, in 1 dimension,
Gaussian Quadrature can exactly integrate polynomials of order $2n-1$ with $n$ quadrature points.

!---

Quadrature applied to [ref_elems] yields an equation that can be analyzed numerically:

!equation
\sum_e \int_{\hat{\Omega}_e} f(\vec{\xi}) \left|\mathcal{J}_e\right| \;\text{d}\vec{\xi} \approx
\sum_e \sum_{q} w_{q} f( \vec{x}_{q}) \left|\mathcal{J}_e(\vec{x}_{q})\right|,

where $\vec{x}_{q}$ is the spatial location of the $q^\textrm{th}$ quadrature point and $w_{q}$ is its
associated weight.

MOOSE handles multiplication by the Jacobian ($\mathcal{J}_e$) and the weight ($w_{q}$)
automatically, thus your `Kernel` object is only responsible for computing the $f(\vec{x}_{q})$
part of the integrand.


!---

Sampling $u_h$ at the quadrature points yields:

!equation
\begin{aligned}
u(\vec{x}_{q}) &\approx u_h(\vec{x}_{q}) = \sum u_j \phi_j(\vec{x}_{q}) \\
\nabla u (\vec{x}_{q}) &\approx \nabla u_h(\vec{x}_{q}) = \sum u_j \nabla \phi_j(\vec{x}_{q})
\end{aligned}

Thus, the weak form of [example_weak_form2] becomes:

!equation id=example_weak_residual
\begin{aligned}
\vec{R}_i(u_h) &= \sum_e \sum_{q} w_{q} \left|\mathcal{J}_e\right|\underbrace{\left[ \nabla\psi_i\cdot k \nabla u_h + \psi_i \left(\vec\beta\cdot \nabla u_h \right) - \psi_i f \right](\vec{x}_{q})}_{\textrm{Kernel Object(s)}} \\
&- \sum_f \sum_{q_{face}} w_{q_{face}} \left|\mathcal{J}_f\right|\underbrace{\left[\psi_i k \nabla u_h \cdot \vec{n} \right](\vec x_{q_{face}})}_{\textrm{BoundaryCondition Object(s)}}
\end{aligned}

The second sum is over boundary faces, $f$. MOOSE `Kernel` or `BoundaryCondition` objects provide
each of the terms in square brackets (evaluated at $\vec{x}_{q}$ or $\vec x_{q_{face}}$ as
necessary), respectively.

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

## Jacobian Free Newton Krylov (JFNK)

$\mathbf{J}(\vec{u}_n)\delta \vec{u}_{n+1} = -\vec{R}(\vec{u}_n)$ is a linear system solved
during each Newton step.

In MOOSE an iterative Krylov method is used to produce a sequence of iterates
$(\delta \vec{u}_{n+1})_k \rightarrow \delta \vec{u}_{n+1}$, $k=1,2,\ldots$

- $\mathbf{J}(\vec{u}_n)$ and $-\vec{R}(\vec{u}_n)$ remain fixed during the iterative process.

The "linear residual" at step $k$ is defined as:

!equation id=jfnk_linear_residual
\vec{\rho}_k \equiv \mathbf{J}(\vec{u}_n)(\delta \vec{u}_{n+1})_k + \vec{R}(\vec{u}_n)

MOOSE prints the norm of this vector, $\|\vec{\rho}_k\|$, at each linear iteration and the
"nonlinear residual" printed by MOOSE is $\|\vec{R}(\vec{u}_n)\|$.

!---

Krylov methods construct a subspace ($\mathcal{K}_k$) for the iterate $k$:

!equation
\mathcal{K}_k = \text{span}\{ \vec{b}, \mathbf{A}\vec{b}, \mathbf{A}^2\vec{b}, \ldots, \mathbf{A}^{k-1}\vec{b}\},

where $\mathbf{A} \equiv \mathbf{J}(\vec{u}_n)$ and $\vec{b} \equiv -\vec{R}(\vec{u}_n)$.

Different Krylov methods produce the $(\delta \vec{u}_{n+1})_k$ iterates in different ways:

- Conjugate Gradients: $\vec{\rho}_k$ orthogonal to $\mathcal{K}_k$.
- GMRES/MINRES: $\vec{\rho}_k$ has minimum norm for $(\delta \vec{u}_{n+1})_k$ in $\mathcal{K}_k$.
- Biconjugate Gradients: $\vec{\rho}_k$ is orthogonal to $\mathcal{K}_k((\mathbf{J}(\vec{u}_n))^T)$

The important part is that $\mathbf{J}$ is never explicitly needed to construct the subspace, only
the action of $\mathbf{J}$ on a vector is required.

!---

This action can be approximated by:

!equation id=jfnk_jacobian
\mathbf{J}\vec{v} \approx \frac{\vec{R}(\vec{u} + \epsilon\vec{v}) - \vec{R}(\vec{u})}{\epsilon}

This form has many advantages:

- No need to do analytic derivatives to form $\mathbf{J}$
- No time needed to compute $\mathbf{J}$ (just residual computations)
- No space needed to store $\mathbf{J}$

!---

## Preconditioned JFNK

Krylov methods need preconditioning to be efficient (or even effective!),
see [citet:knoll2004jacobian].

- Reduces the total number of linear iterations
- Krylov methods, in theory, converge in the number of linear iterations equal to the number of unknowns in the system

Even though the Jacobian is never formed, JFNK methods still require preconditioning.

!---

When using right preconditioning:

!equation
\mathbf{J} (\vec{u}_i) \mathbf{M}^{-1} (\mathbf{M}\delta \vec{u}_{i+1}) = -\vec{R}(\vec{u}_i)

$\mathbf{M}$ symbolically represents the preconditioning matrix or process, and with GMRES only
the action of $\mathbf{M}^{-1}$ on a vector is required.

When right-preconditioned, [jfnk_linear_residual] becomes:

!equation id=pjfnk_linear_residual
\vec{\rho}_k \equiv \mathbf{J} (\vec{u}_i) \mathbf{M}^{-1} (\mathbf{M}\delta \vec{u}_{i+1})_k + \vec{R}(\vec{u}_i),

and [jfnk_jacobian] becomes:

!equation id=pjfnk_jacobian
\mathbf{J} (\vec{u}_i) \mathbf{M}^{-1}\vec{v} \approx \frac{\vec{R}(\vec{u}_i + \epsilon \mathbf{M}^{-1}\vec{v}) - \vec{R}(\vec{u}_i)}{\epsilon}

!---

## MOOSE Solve Types

The solve type is specified in the `[Executioner]` block within the input file:

```
[Executioner]
  solve_type = PJFNK
```

Available options include:

- +PJFNK+: Preconditioned Jacobian Free Newton Krylov (default)
- +JFNK+: Jacobian Free Newton Krylov
- +NEWTON+: Performs solve using exact Jacobian for preconditioning
- +FD+: PETSc computes terms using a finite difference method (debug)

!---

## +JFNK+

The `Kernel` method `computeQpResidual` is called to compute
$\vec{R}(\vec{u}_n)$ during the nonlinear step ([newton]).

During each linear step of [jfnk_linear_residual] the `computeQpResidual` method is called to
compute $\mathbf{J}\vec{v}$ using [jfnk_jacobian].

!---

## +PJFNK+

The `Kernel` method `computeQpResidual` is called to compute
$\vec{R}(\vec{u}_n)$ during the nonlinear step ([newton]).

During each linear step of [pjfnk_linear_residual] the `computeQpResidual` method is called to
compute $\mathbf{J} (\vec{u}_i) \mathbf{M}^{-1}\vec{v}$ using [pjfnk_jacobian]. The
`computeQpJacobian` and `computeQpOffDiagJacobian` methods are used to compute values for the
preconditioning matrix $\mathbf{M}$.

!---

## +NEWTON+

The `Kernel` method `computeQpResidual` is called to compute $\vec{R}(\vec{u}_n)$ during the
nonlinear step ([newton]).

The `computeQpJacobian` and `computeQpOffDiagJacobian` methods are used to compute the
preconditioning matrix $\mathbf{M}$. It is assumed that $\mathbf{M} = \mathbf{J}$, thus the
approximation in [pjfnk_jacobian] is =not= needed allowing for the
residual and Jacobian calculations to remain constant during the linear iterations in
[pjfnk_linear_residual].

!---

## Summary

The Finite Element Method is a way of numerically approximating the solution of PDEs.

Just like polynomial fitting, FEM finds coefficients for basis functions.

The solution is the combination of the coefficients and the basis functions, and the solution can
be sampled anywhere in the domain.

Integrals are computed numerically using quadrature.

Newton's method provides a mechanism for solving a system of nonlinear equations.

The Preconditioned Jacobian Free Newton Krylov (JFNK) method allows us to avoid explicitly forming
the Jacobian matrix while still computing its action.

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

## Manual Jacobian Calculation

The remainder of the tutorial will focus on using [!ac!AD] for computing
Jacobian terms, but it is possible to compute them manually.

It is recommended that all new Kernel objects use AD.

!---

### FEM Derivative Identities

The following relationships are useful when computing Jacobian terms.

!equation id=diff_u
\frac{\partial u_h}{\partial u_j} = \sum_k\frac{\partial }{\partial u_j}\left(u_k \phi_k\right) = \phi_j

!equation id=grad_u
\frac{\partial \left(\nabla u_h\right)}{\partial u_j} = \sum_k \frac{\partial }{\partial u_j}\left(u_k \nabla \phi_k\right) = \nabla \phi_j

!---

### Newton for a Simple Equation

Again, consider the advection-diffusion equation with nonlinear $k$, $\vec{\beta}$, and $f$:

!equation
\begin{aligned}
- \nabla\cdot k\nabla u + \vec{\beta} \cdot \nabla u = f
\end{aligned}

Thus, the $i^{th}$ component of the residual vector is:

!equation
\begin{aligned}
\vec{R}_i(u_h) = \left(\nabla\psi_i, k\nabla u_h \right) - \langle\psi_i, k\nabla u_h\cdot \hat{n} \rangle +
\left(\psi_i, \vec{\beta} \cdot \nabla u_h\right) - \left(\psi_i, f\right)
\end{aligned}

!---

Using the previously-defined rules in [diff_u] and [grad_u] for $\frac{\partial u_h}{\partial u_j}$
and $\frac{\partial \left(\nabla u_h\right)}{\partial u_j}$, the $(i,j)$ entry of the Jacobian is
then:

!equation
\begin{aligned}
J_{ij}(u_h) &= \left(\nabla\psi_i, \frac{\partial k}{\partial u_j}\nabla u_h \right) + \left(\nabla\psi_i, k \nabla \phi_j \right) - \left \langle\psi_i, \frac{\partial k}{\partial u_j}\nabla u_h\cdot \hat{n} \right\rangle \\&- \left \langle\psi_i, k\nabla \phi_j\cdot \hat{n} \right\rangle + \left(\psi_i, \frac{\partial \vec{\beta}}{\partial u_j} \cdot\nabla u_h\right) + \left(\psi_i, \vec{\beta} \cdot \nabla \phi_j\right) - \left(\psi_i, \frac{\partial f}{\partial u_j}\right)
\end{aligned}

That even for this "simple" equation, the Jacobian entries are nontrivial: they depend on the partial
derivatives of $k$, $\vec{\beta}$, and $f$, which may be difficult or time-consuming to compute
analytically.

In a multiphysics setting with many coupled equations and complicated material properties, the
Jacobian might be extremely difficult to determine.
