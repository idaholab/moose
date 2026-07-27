# HourglassCorrectionHex8

!syntax description /Kernels/HourglassCorrectionHex8

## Description

`HourglassCorrectionHex8` is the three-dimensional companion of
[HourglassCorrectionQuad4.md]. It stabilizes a single displacement component
on an underintegrated `HEX8` element. Use one kernel per displacement component
and constant, one-point quadrature. The kernel is intended for explicit
dynamics; it contributes a residual but no Jacobian.

In libMesh `HEX8` node ordering, the four classical mode vectors are

\begin{equation}
\begin{aligned}
\boldsymbol{\gamma}^{(1)} &= [1,-1,1,-1,1,-1,1,-1],\\
\boldsymbol{\gamma}^{(2)} &= [1,1,-1,-1,-1,-1,1,1],\\
\boldsymbol{\gamma}^{(3)} &= [1,-1,-1,1,-1,1,1,-1],\\
\boldsymbol{\gamma}^{(4)} &= [-1,1,-1,1,1,-1,1,-1].
\end{aligned}
\end{equation}

For each mode, the kernel removes its component in the affine displacement
space:

\begin{equation}
\boldsymbol{A}=\sum_i\boldsymbol{d}_i\boldsymbol{d}_i^T,\qquad
\boldsymbol{p}^{(a)}=\sum_i\gamma_i^{(a)}\boldsymbol{d}_i,\qquad
\widehat{\gamma}_i^{(a)}
=\gamma_i^{(a)}-{\boldsymbol{p}^{(a)}}^T\boldsymbol{A}^{-1}\boldsymbol{d}_i,
\end{equation}

where $\boldsymbol{d}_i$ is the current nodal position relative to the element
centroid. The hourglass amplitudes and integrated nodal residual are

\begin{equation}
H_a=\sum_i\widehat{\gamma}_i^{(a)}u_i,\qquad
R_i=\texttt{penalty}\,\mu\,\frac{V}{h^2}
\sum_{a=1}^4\widehat{\gamma}_i^{(a)}H_a,\qquad
h^2=\frac{1}{3}\operatorname{tr}\boldsymbol{A}.
\end{equation}

The kernel returns the pointwise residual without $V$; normal MOOSE kernel
assembly supplies the one-point integration measure exactly once. Projection
of both the amplitude and residual direction prevents hourglass forces from
doing work on translations or linear displacement fields, including on
distorted elements.

## Example Syntax

!listing modules/solid_mechanics/test/tests/hourglass/hex_residual_mode.i block=Kernels

## Parameters

!syntax parameters /Kernels/HourglassCorrectionHex8

## Inputs

!syntax inputs /Kernels/HourglassCorrectionHex8

## Child Objects

!syntax children /Kernels/HourglassCorrectionHex8
