# Finite Volume Method (FVM)

!---

## Advantages of Finite Volume Method

+(1) Flexible:+ Works with arbitrary mixed cell types (even if they are mixed in the same problem)

+(2) Inherently conservative:+ Uses the integral form of the PDEs. This makes it perfect for
      applications where conservation laws need to be respected (fluid flows).

+(3) Accurate:+ Can be second-order accurate in space. Higher order possible, but not common.

+(4) Robust:+ It is the current standard for most commercial (STAR-CCM+, ANSYS-Fluent, etc.) and major open-source (OpenFOAM, etc.) CFD codes.

+(5) Rich literature:+ Has been researched for decades.

!---

## Approximating the Solution

!row!
!col! width=50%

!style fontsize=80%
First, domain $\Omega$ is split into $N_c$ cells ($\Omega_{i},~i=1,...,N_c$).

!media darcy_thermo_mech/fvm_cell.png style=width:100%;background:white;

!col-end!

!col! width=50%

In MOOSE, the cell-centered finite volume approximation ([!citet](moukalled2016finite) and [!citet](jasak1996error)) is used
due to the fact that it supports +both unstructured and structured meshes+:

- Constant over the cells (value at cell centroid): $u \approx u_C$ if $r \in \Omega_C$
- Constant over the faces (value at face centroid): $u \approx u_f$ if $r \in \partial\Omega_f$

Error analysis is done using Taylor expansions, we prefer second-order discretization/interpolation schemes.

!col-end!

!row-end!

!---

## FVM using an Advection-Diffusion-Reaction Problem

+(1)+ Write the strong form of the equation:

!equation
-\nabla\cdot k\nabla u + \vec{\beta} \cdot \nabla u + cu - f = 0

+(2)+ Integrate the equation over the domain $\Omega$:

!equation
\int_\Omega \left(-\nabla\cdot k\nabla u + \vec{\beta} \cdot \nabla u +cu -f \right) dV = 0

+(3)+ Split the integral into different terms:

!equation
-\int_\Omega \nabla\cdot k\nabla u dV + \int_\Omega \vec{\beta} \cdot \nabla u dV + \int_\Omega cu dV -\int_\Omega f dV = 0

!---

## Approximating the Reaction and Source Terms

Split into cell-wise integrals:

!equation
\int_\Omega cu ~dV = \sum_i^{N_c}\int_{\Omega_i} c u ~dV, \quad \text{and} \quad \int_\Omega f ~dV = \sum_i^{N_c}\int_{\Omega_i} f ~dV

Use the finite volume approximation (on cell C):

!equation
\int_{\Omega_C} c u ~dV\approx c_Cu_CV_C, \quad \text{and} \quad \int_{\Omega_C} f ~dV \approx f_C V_C

!---

## Approximating the Reaction and Source Terms

For a finite volume variable, there is only one quadrature point and it is the cell centroid. This form
is very similar to the one used in the FEM routines (We always use a test function of 1!).

!listing framework/src/fvkernels/FVReaction.C re=ADReal\sFVReaction::computeQpResidual.*?^}

!---

## Approximating the Diffusion Term

!row!
!col! width=40%

!style fontsize=80%
Othogonal scenario: ($S_f$ and $\delta_{NP}$ are parallel)

!media darcy_thermo_mech/fvm_orthogonal.png style=width:100%;background:white;

!style fontsize=80%
Non-othogonal scenario:

!media darcy_thermo_mech/fvm_nonorthogonal.png style=width:100%;background:white;

!col-end!

!col! width=60%

!style fontsize=80%
Split into cell-wise integrals and use the divergence theorem:

!style fontsize=80%
!equation
\int_\Omega\nabla\cdot k\nabla u dV = \sum_i^{N_c}\int_{\Omega_i}\nabla\cdot k\nabla u dV = \sum_i^{N_c}\int_{\partial\Omega_i} k\nabla u \cdot \hat{n} dS

!style fontsize=80%
On an internal cell (let's say on cell C):

!style fontsize=80%
!equation
\int_{\partial\Omega_C} k\nabla u \cdot \hat{n} dS = \sum_f^{N_{f,C}}\int_{\partial\Omega_{C,f}}k\nabla u \cdot \hat{n} dS \approx \sum_f^{N_{f,C}}k_f\left(\nabla u\right)_f \cdot \hat{n}_f |S_f|

!style! fontsize=80%
- $\left(\nabla u\right)_f \cdot \hat{n}$ for orthogonal scenario: $\frac{u_N-u_C}{\delta_{CN}}$ (cheap and accurate)
- $\left(\nabla u\right)_f \cdot \hat{n}$ for nonorthogonal scenario: $\frac{u_N-u_C}{\delta_{CN}}|\Delta_f|+\hat{k}_f\cdot \left(\nabla u\right)_f$

  - The surface normal ($\hat{n}$) is expanded into a $\delta_{CN}$-parallel ($\hat{\Delta}_f$) and a remaining vector ($\hat{k}_f$): $\hat{n}_f=\hat{\Delta}_f + \hat{k}_f$
  - $\left(\nabla u\right)_f$ on the right hand side is computed using the interpolation of cell gradients (next slide)

!style-end!

!col-end!

!row-end!

!---

## Interpolation and Gradient Estimation

!row!
!col! width=40%

!style fontsize=80%
Basic interpolation scheme:

!media darcy_thermo_mech/fvm_interpolation.png style=width:100%;background:white;

!equation
u_f = \left(\frac{\delta_{fN}}{\delta_{CN}}\right) u_C + \left(1- \frac{\delta_{fN}}{\delta_{CN}}\right) u_N

!col-end!

!col! width=60%

!style fontsize=80%
Cell gradients can be estimated using the Green-Gauss theorem:

!equation
\int_{\Omega_i}\nabla u ~dV \approx (\nabla u)_i V_i

!style fontsize=80%
At the same time:

!equation
\int_{\Omega_i}\nabla u ~dV = \int_{\partial\Omega_i}u \hat{n} dS \approx \sum_f u_f \hat{n}_f |S_f|

!style fontsize=80%
From the two equations:

!equation
(\nabla u)_i = \frac{1}{V_i} \sum_f u_f \hat{n}_f |S_f|

!style fontsize=80%
(+Alternative approaches:+ least-squares, node-based interpolation, skewness-corrected interpolation)

!col-end!

!row-end!


!---

## Approximating the Diffusion Term

!listing framework/src/fvkernels/FVDiffusion.C re=ADReal\sFVDiffusion::computeQpResidual.*?^}

!---

## Approximating the Advection Term

!row!
!col! width=60%

!style fontsize=90%
Split to integral to cell-wise integrals and use the divergence theorem:

!equation
\int_\Omega \vec{\beta} \cdot \nabla u~ dV = \sum_i^N\int_{\Omega_i}\vec{\beta} \cdot \nabla u~ dV = \sum_i^N\int_{\partial\Omega_i} (\vec{\beta}~u) \cdot \hat{n}~ dS

!style fontsize=90%
On an internal cell (let's say on cell C), assuming that $\beta$ is constant:

!equation
\int_{\partial\Omega_i} (\vec{\beta}~u) \cdot \hat{n}~ dS = \sum_f^{N_{f,C}} \int_{\partial\Omega_{C,f}} (\vec{\beta}~u) \cdot \hat{n}~ dS \approx \sum_f^{N_{f,C}} \vec{\beta}_f~ u_f \hat{n}_f |S_f|

!col-end!

!col! width=40%

Common interpolation method for $u_f$:

- Upwind
- (Weighted) Average
- TVD limited schemes: van Leer, Minmod

!col-end!

!row-end!

!---

## The MOOSE Functor System

!style! fontsize=70%

- Unifies the evaluation of Functions, Variables, and certain Materials on faces, elements, quadrature points, etc.
- Can be used to easily evaluate gradients on elements/faces as well.
- They require an argument that contains information on the way the value should be obtained:

  - `ElemArg`: used to evaluate on an element, contains a link to the underlying element in `libMesh`
  - `FaceArg`: used to evaluate on a face, has to contains information about the face, surrounding elements and the interpolation method

!listing unit/src/MooseFunctorTest.C line=auto elem_arg

!listing unit/src/MooseFunctorTest.C re=auto face[\s\S]*?\;

!listing unit/src/MooseFunctorTest.C line=test_gradient(face)

!style-end!

!---

## Approximating the Advection Term

!listing framework/src/fvkernels/FVAdvection.C re=ADReal\sFVAdvection::computeQpResidual.*?^}

!---

## Example Input File

!listing test/tests/fvkernels/mms/non-orthogonal/advection-diffusion-reaction.i

!---

## Application in the Navier Stokes Module of MOOSE

!row!
!col! width=60%

- Focuses on coarse-mesh applications

- Supports free and porous medium flows

- Supports incompressible, weakly-compressible and (some) compressible simulations

- Under intense development

+To the right:+ cooling of a spent fuel cask with natural circulation (Results by Sinan Okyay)

!col-end!

!col! width=40%

!media darcy_thermo_mech/fvm_cask.gif style=width:100%;background:white;

!col-end!

!row-end!
