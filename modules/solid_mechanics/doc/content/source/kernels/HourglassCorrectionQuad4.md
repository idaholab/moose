# HourglassCorrectionQuad4

!syntax description /Kernels/HourglassCorrectionQuad4

## Description

`HourglassCorrectionQuad4` stabilizes a single displacement component on an
underintegrated `QUAD4` element. Use one kernel per displacement component and
constant, one-point quadrature. The kernel is intended for explicit dynamics;
it contributes a residual but no Jacobian.

Let the current nodal coordinates be $\boldsymbol{x}_i$, their centroid be
$\boldsymbol{c}$, and $\boldsymbol{d}_i=\boldsymbol{x}_i-\boldsymbol{c}$. The
geometry matrix and length scale are

\begin{equation}
\boldsymbol{A}=\sum_{i=1}^4\boldsymbol{d}_i\boldsymbol{d}_i^T,\qquad
\boldsymbol{M}=\boldsymbol{A}^{-1},\qquad
h^2=\frac{1}{2}\operatorname{tr}\boldsymbol{A}.
\end{equation}

Starting from the classical mode vector
$\boldsymbol{\gamma}=[1,-1,1,-1]$, the kernel removes its component in the
affine displacement space:

\begin{equation}
\boldsymbol{p}=\sum_i\gamma_i\boldsymbol{d}_i,\qquad
\widehat{\gamma}_i=\gamma_i-\boldsymbol{p}^T\boldsymbol{M}\boldsymbol{d}_i.
\end{equation}

For nodal values $u_i$, the hourglass amplitude and integrated nodal residual
are

\begin{equation}
H=\sum_i\widehat{\gamma}_i u_i,\qquad
R_i=\texttt{penalty}\,\mu\,\frac{A_e}{h^2}\widehat{\gamma}_i H,
\end{equation}

where $\mu$ is `shear_modulus` and $A_e$ is the one-point integration measure.
The kernel returns the pointwise factor
$\texttt{penalty}\,\mu\,\widehat{\gamma}_i H/h^2$; normal MOOSE kernel assembly
supplies the integration measure exactly once. In axisymmetric coordinates,
that measure also includes the coordinate transformation factor.

Projecting the mode vector makes both the hourglass amplitude and its residual
direction orthogonal to translations and linear displacement fields. This is
important for distorted elements, where the unprojected reference mode is not
generally orthogonal to the physical affine space.

This formulation follows the geometry-aware construction of Flanagan and
Belytschko (1981).

## Example Syntax

!listing modules/solid_mechanics/test/tests/hourglass/hourglass_residual_g1.i block=Kernels

## Parameters

!syntax parameters /Kernels/HourglassCorrectionQuad4

## Inputs

!syntax inputs /Kernels/HourglassCorrectionQuad4

## Child Objects

!syntax children /Kernels/HourglassCorrectionQuad4
