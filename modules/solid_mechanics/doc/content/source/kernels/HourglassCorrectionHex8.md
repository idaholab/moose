# HourglassCorrectionHex8

!syntax description /Kernels/HourglassCorrectionHex8

## Description

`HourglassCorrectionHex8` is the 3D companion of [HourglassCorrectionQuad4.md]:
it applies hourglass stabilization to underintegrated `HEX8` elements for a
single displacement component. The best-fit affine (linear) part of the nodal
displacement is removed via a least-squares fit over the element, and the
remaining non-affine content is projected onto the four classical
Flanagan-Belytschko hourglass base vectors (in libMesh `HEX8` node ordering)

\begin{equation}
\begin{aligned}
\boldsymbol{\Gamma}^{(1)} &= [1,-1,1,-1,1,-1,1,-1] && (\xi\eta)\\
\boldsymbol{\Gamma}^{(2)} &= [1,1,-1,-1,-1,-1,1,1] && (\eta\zeta)\\
\boldsymbol{\Gamma}^{(3)} &= [1,-1,-1,1,-1,1,1,-1] && (\xi\zeta)\\
\boldsymbol{\Gamma}^{(4)} &= [-1,1,-1,1,1,-1,1,-1] && (\xi\eta\zeta)
\end{aligned}
\end{equation}

and penalized with the rotation-invariant scale
\begin{equation}
c = \texttt{penalty}\times\mu\times\frac{V}{h^2},\qquad
h^2 = \tfrac{1}{3}\operatorname{tr}\mathbf{A},\qquad
V = 8\det\mathbf{B},
\end{equation}
where $\mathbf{A} = \sum_i \mathbf{d}_i\mathbf{d}_i^T$ is the second-moment
matrix of the (displaced) nodes about their average and $\mathbf{B}$ is the
Jacobian of the trilinear map at the element center, so $V$ equals the
one-point-quadrature integration weight (the exact volume for
parallelepipeds). Using the same $V$ definition keeps this kernel consistent
at machine precision with the batched [NEML2HourglassCorrection.md].

This kernel is intended for explicit dynamics with constant one-point
quadrature; apply one instance per displacement component.

!syntax parameters /Kernels/HourglassCorrectionHex8

!syntax inputs /Kernels/HourglassCorrectionHex8
