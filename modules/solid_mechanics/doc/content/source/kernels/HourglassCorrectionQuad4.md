# HourglassCorrectionQuad4

!syntax description /Kernels/HourglassCorrectionQuad4

## Description

`HourglassCorrectionQuad4` applies an hourglass stabilization to underintegrated
`QUAD4` elements for a single displacement component (x or y). It removes the best-fit
affine (linear) part of the displacement over each element and penalizes only the two
non-affine hourglass modes, following the spirit of Flanagan–Belytschko hourglass control
for reduced integration.

This kernel is intended to be used with constant one-point quadrature. It is geometry-aware
and uses a rotation-invariant stabilization scale.

Let the current element have nodal coordinates \(\mathbf{x}_i\) (in the displaced mesh
if `use_displaced_mesh = true`) and define
\(\mathbf{c} = \frac{1}{4}\sum_i \mathbf{x}_i\) and \(\,\mathbf{d}_i = \mathbf{x}_i-\mathbf{c}\).
Form the symmetric geometry matrix
\begin{equation}
\mathbf{A} = \sum_{i=1}^4 \mathbf{d}_i \mathbf{d}_i^T
\quad\text{and its inverse}\quad
\mathbf{M} = \mathbf{A}^{-1}.
\end{equation}
Given the nodal values of the active displacement component \(u_i\), compute the
least-squares affine fit
\begin{align}
\bar{u} &= \tfrac{1}{4}\sum_i u_i,\\
\boldsymbol{g} &= \mathbf{M} \sum_i u_i \, \mathbf{d}_i,\\
\hat{u}_i &= \bar{u} + \boldsymbol{g}\cdot\mathbf{d}_i,\\
\tilde{u}_i &= u_i - \hat{u}_i\quad (\text{hourglass part}).
\end{align}
Define the two classical hourglass vectors for QUAD4 (unnormalized)
\(\,\mathbf{g}^{(1)} = [1,-1,1,-1]\,,\; \mathbf{g}^{(2)} = [1,1,-1,-1]\,\), and compute
\begin{equation}
H_1 = \sum_i g^{(1)}_i\, \tilde{u}_i,\qquad
H_2 = \sum_i g^{(2)}_i\, \tilde{u}_i.
\end{equation}
The rotation-invariant stabilization scale is
\begin{equation}
 c = \texttt{penalty}\;\times\;\mu\;\times\;\frac{A_e}{h^2},\qquad
 h^2 = \tfrac{1}{2}\,\mathrm{tr}(\mathbf{A}),
\end{equation}
where \(A_e\) is the current element area (in 2D) and \(\mu\) is the shear modulus
provided by the user. The residual contribution at node \(i\) is
\begin{equation}
R_i = c\,\big( g^{(1)}_i\,H_1 + g^{(2)}_i\,H_2 \big).
\end{equation}
A consistent Jacobian (with geometry held fixed with respect to the active displacement
component) follows from
\begin{equation}
\mathbf{p}^{(a)} = \sum_i g^{(a)}_i\,\mathbf{d}_i,\qquad
\frac{\partial H_a}{\partial u_j} = g^{(a)}_j - {\mathbf{p}^{(a)}}^T \mathbf{M}\, \mathbf{d}_j,\quad a\in\{1,2\},
\end{equation}
leading to
\begin{equation}
K_{ij} = \frac{\partial R_i}{\partial u_j} = c\,\Big[\, g^{(1)}_i\Big(g^{(1)}_j - {\mathbf{p}^{(1)}}^T \mathbf{M}\, \mathbf{d}_j\Big)
\; + \; g^{(2)}_i\Big(g^{(2)}_j - {\mathbf{p}^{(2)}}^T \mathbf{M}\, \mathbf{d}_j\Big)\,\Big].
\end{equation}
This construction penalizes only the non-affine hourglass content and leaves rigid-body and
uniform-strain (affine) modes unpenalized.

## Example Input File Syntax

A minimal single-element test using one-point quadrature and simple anchoring can be found at

!listing modules/solid_mechanics/test/tests/hourglass/hourglass_residual_g1.i

!listing modules/solid_mechanics/test/tests/hourglass/hourglass_residual_g1.i

For a beam example that uses this kernel with a realistic shear modulus:

!listing modules/solid_mechanics/test/tests/hourglass/beam1b.i block=Kernels

## References

- D. P. Flanagan and T. Belytschko, “A Uniform Strain Hexahedron and Quadrilateral with
  Orthogonal Hourglass Control,” International Journal for Numerical Methods in Engineering,
  17(5), 679–706, 1981.

## Parameters

!syntax parameters /Kernels/HourglassCorrectionQuad4

!syntax inputs /Kernels/HourglassCorrectionQuad4

!syntax children /Kernels/HourglassCorrectionQuad4
