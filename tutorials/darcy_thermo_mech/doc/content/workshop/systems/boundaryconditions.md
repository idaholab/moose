# Boundary Condition System

System for computing residual contributions from boundary terms of a [!ac](PDE).

!---

A `BoundaryCondition` (BC) object computes a residual on a boundary (or internal side) of a domain.

There are two flavors of BC objects: Nodal and Integrated.

!---

## Integrated BC

Integrated BCs are integrated over a boundary or internal side and should inherit
from `ADIntegratedBC`.

The structure is very similar to Kernels: objects must override `computeQpResidual`

!---

## ADIntegrtedBC Object Members

`_u`, `_grad_u`\\
Value and gradient of the variable this Kernel is operating on

`_test`, `_grad_test`\\
Value ($\psi$) and gradient ($\nabla \psi$) of the test functions at the quadrature points

`_phi`, `_grad_phi`\\
Value ($\phi$) and gradient ($\nabla \phi$) of the trial functions at the quadrature points

`_i`, `_j`, `_qp`\\
Current index for test function, trial functions, and quadrature point, respectively

`_normals`:\\
Outward normal vector for boundary element

`_boundary_id`\\
The boundary ID

`_current_elem`, `_current_side`\\
A pointer to the element and index to the current boundary side

!---

## Non-Integrated BC

Non-integrated BCs set values of the residual directly on a boundary or internal side and
should inherit from `ADNodalBC`.

The structure is very similar to Kernels: objects must override `computeQpResidual`.

!---

## NodalBC Object Members

`_u`\\
Value of the variable this Kernel is operating on

`_qp`\\
Current index, used for interface consistency

`_boundary_id`\\
The boundary ID

`_current_node`\\
A pointer to the current node that is being operated on.

!---

## Dirichlet BCs

Set a condition on the `value` of a variable on a boundary:

!equation
u = g_1 \quad \text{on} \quad \partial\Omega_1

becomes

!equation
u - g_1 = 0 \quad \text{on} \quad \partial\Omega_1

!---

## DirichletBC.h

!listing framework/include/bcs/DirichletBC.h

!---

## DirichletBC.C

!listing framework/src/bcs/DirichletBC.C

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

## NeumannBC.h

!listing framework/include/bcs/NeumannBC.h

!---

## NeumannBC.C

!listing framework/src/bcs/NeumannBC.C

!---

## Periodic BCs

Periodic boundary conditions are useful for modeling quasi-infinite domains and systems with
conserved quantities.

- 1D, 2D, and 3D
- With mesh adaptivity
- Can be restricted to specific variables
- Supports arbitrary translation vectors for defining periodicity
