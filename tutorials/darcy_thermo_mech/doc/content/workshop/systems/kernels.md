# [Kernel System](syntax/Kernels/index.md)

A system for computing the residual contribution from a volumetric term within a [!ac](PDE) using
the Galerkin finite element method.

!---

## Kernel Object

A `Kernel` objects represents one or more terms in a [!ac](PDE).

A `Kernel` object is required to compute a residual at a quadrature point, which is done by
calling the `computeQpResidual` method.

!---

## Kernel Object Members

`_u`, `_grad_u`\\
Value and gradient of the variable this Kernel is operating on

`_test`, `_grad_test`\\
Value ($\psi$) and gradient ($\nabla \psi$) of the test functions at the quadrature points

`_phi`, `_grad_phi`\\
Value ($\phi$) and gradient ($\nabla \phi$) of the trial functions at the quadrature points

`_q_point`\\
Coordinates of the current quadrature point

`_i`, `_j`\\
Current index for test and trial functions, respectively

`_qp`\\
Current quadrature point index

!---

## Kernel Base Classes

| Base | Override | Use |
| :- | :- | :- |
| Kernel\\ +ADKernel+ | computeQpResidual | Use when the term in the [!ac](PDE) is multiplied by both the test function and the gradient of the test function (`_test` and `_grad_test` must be applied) |
| KernelValue\\ +ADKernelValue+ | precomputeQpResidual | Use when the term computed in the [!ac](PDE) is only multiplied by the test function (do not use `_test` in the override, it is applied automatically) |
| KernelGrad\\ +ADKernelGrad+ | precomputeQpResidual | Use when the term computed in the [!ac](PDE) is only multiplied by the gradient of the test function (do not use `_grad_test` in the override, it is applied automatically) |

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

## ADDiffusion.h

!listing ADDiffusion.h

!---

## ADDiffusion.C

!listing ADDiffusion.C
