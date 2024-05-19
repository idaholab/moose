# LinearFVAnisotropicDiffusion

## Description

This kernel contributes to the system matrix and the right hand side of a system
which is solved for a [linear finite volume variable](MooseLinearVariableFV.md).
The difference between this kernel and [LinearFVDiffusion.md] is that this kernel requires
a vector of diffusion coefficients, where every entry describes the diffusion coefficient for
a principal direction. This is equivalent to supplying a diagonal tensor in the
fully anisotropic diffusion case.

The implementation in this kernel is based on the derivation in [!cite](liu2015finite). The
contributions of the system matrix and right hand side can be derived using the divergence
theorem on a volumetric integral over cell $C$:

!equation
\int\limits_{V_C} \nabla \cdot \mathbb{D} \nabla u dV =
\sum\limits_f \int\limits_{S_f} \mathbb{D} \nabla u \cdot \vec{n} dS,

where $\mathbb{D}$ denotes a space dependent diagonal diffusion tensor, while the right hand side
describes the sum of the surface integrals on each side of cell $C$.
Following [!cite](liu2015finite) and using the assumption that the tensor is diagonal,
we can manipulate this expression to arrive to the following form:

!equation
\sum\limits_f \int\limits_{S_f} \mathbb{D} \vec{n} \cdot \nabla u dS,

where $mathbb{D} \vec{n}$ can be split into two contributions:

!equation
\mathbb{D} \vec{n} = (\mathbb{D}\vec{n}\cdot \vec{n}) \vec{n} + (\mathbb{D}\vec{n} - \mathbb{D}\vec{n}\cdot \vec{n} \vec{n}).

Plugging this expression back to the surface integral, we get the following:

!equation
\sum\limits_f \int\limits_{S_f} (\mathbb{D}\vec{n}\cdot \vec{n}) \vec{n} \cdot \nabla u +
(\mathbb{D}\vec{n} - \mathbb{D}\vec{n}\cdot \vec{n} \vec{n}) \cdot \nabla u dS,

where we can treat the normal projection ($\int\limits_{S_f} (\mathbb{D}\vec{n}\cdot \vec{n}) \vec{n} \cdot \nabla u dS$) the same way as described in [LinearFVDiffusion.md] with
$\mathbb{D}\vec{n}\cdot \vec{n}$ replacing the diffusion coefficient. The
second term ($\int\limits_{S_f} (\mathbb{D}\vec{n} - \mathbb{D}\vec{n}\cdot \vec{n} \vec{n}) \cdot \nabla u dS dS$) can be treated explicitly, similarly to the nonorthogonal correction in
[LinearFVDiffusion.md].

!syntax parameters /LinearFVKernels/LinearFVAnisotropicDiffusion

!syntax inputs /LinearFVKernels/LinearFVAnisotropicDiffusion

!syntax children /LinearFVKernels/LinearFVAnisotropicDiffusion
