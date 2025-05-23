# [Kernel System](syntax/Kernels/index.md)

A system for computing the residual contribution from a volumetric term within a [!ac](PDE) using
the Galerkin finite element method.

!---

## Kernel Object

A `Kernel` objects represents one or more terms in a [!ac](PDE).

A `Kernel` object computes the residual at each quadrature point on every element. When forming a
Jacobian, the `Kernel` object is also called to compute its components at each quadrature point.

!---

## Diffusion

Recall the steady-state diffusion equation on the 3D domain $\Omega$:

!equation
-\nabla \cdot \nabla u = 0 \in \Omega

The weak form of this equation includes a volume integral, which in inner-product notation,
is given by:

!equation
\left(\nabla \psi_i, \nabla u_h\right) = 0 \quad\forall  \psi_i,

where $\psi_i$ are the test functions and $u_h$ is the finite element solution.

!---

This integral is approximated by the kernel using the specified quadrature.

!equation
\left(\nabla \psi_i, \nabla u_h\right) = \sum_{qp} w_{qp} \nabla \psi_i(qp)  \nabla u_h(qp)
