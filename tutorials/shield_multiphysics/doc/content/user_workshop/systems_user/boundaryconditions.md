# [Boundary Condition System](syntax/BCs/index.md)

System for computing residual contributions from boundary terms of a [!ac](PDE).

!---

A `BoundaryCondition` (BC) object computes a residual on a boundary (or internal side) of a domain.

There are two flavors of BC objects: Nodal and Integrated.

!---

## Integrated BC

Integrated BCs are integrated over a boundary or internal side. They are meant to add a
surface integral term, coming from example from the application of the divergence theorem.

This integral is computed using a surface quadrature.

!---

## Non-Integrated BC

Non-integrated BCs set values of the residual directly on a boundary or internal side.
They do not rely a quadrature-point based integration.

!---

## Dirichlet BCs

Set a condition on the `value` of a variable on a boundary:

!equation
u = g_1 \quad \text{on} \quad \partial\Omega_1

becomes

!equation
u - g_1 = 0 \quad \text{on} \quad \partial\Omega_1

!---

## Integrated BCs

Integrated BCs (including Neumann BCs) are actually integrated over the external face of an element.

!equation
\left\{
   \begin{array}{rl}
     (\nabla u, \nabla \psi_i) - (f, \psi_i) - \langle \nabla u\cdot \hat{\boldsymbol n}, \psi_i\rangle &= 0 \quad \forall i
    \\
      \nabla u \cdot \hat{\boldsymbol n} &= g_1\quad \text{on} \quad\partial\Omega
   \end{array}
\right.

becomes:

!equation
(\nabla u, \nabla \psi_i) - (f, \psi_i) - \langle g_1, \psi_i\rangle = 0 \quad \forall i

If $\nabla u \cdot \hat{\boldsymbol n} = 0$, then the boundary integral is zero
("natural boundary condition").

!---

## Periodic BCs

Periodic boundary conditions are useful for modeling quasi-infinite domains and systems with
conserved quantities.

- 1D, 2D, and 3D
- With mesh adaptivity
- Can be restricted to specific variables
- Supports arbitrary translation vectors for defining periodicity
