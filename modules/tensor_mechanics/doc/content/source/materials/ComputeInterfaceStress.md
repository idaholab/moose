# Compute Interface Stress

!syntax description /Materials/ComputeInterfaceStress

## Description

The resulting stress tensor has the property of having one Eigenvector in the
direction of the order parameter gradient, with an Eigenvalue of zero, and two
Eigenvectors perpendicular to that direction, with an Eigenvalue $\sigma_i\cdot|\nabla\eta|$,
where the scalar $\sigma_i$ (`stress`) is a supplied parameter and $\eta$ (`v`) is a given order
parameter.
\begin{equation}
\vec e_1 = \frac{\nabla\eta}{|\nabla\eta|}
\end{equation}

Then two more linearly independent vectors $\vec e_2$ and $\vec e_3$ are generated
by determining the component $m$ of $\vec e_1$ with the largest magnitude. $\vec e_2$
and $\vec e_3$ are then set to unit vectors from the set
\begin{equation}
\vec u_1 = (1,0,0), \vec u_2 = (0,1,0), \vec u_3 = (0,0,1)
\end{equation}
such that
\begin{equation}
\vec e_2 = \vec u_{(m+1) \backslash{3}}, \vec e_3 = \vec u_{(m+2)\backslash{3}},
\end{equation}
where the $\backslash$ operator represents integer modulo. The basis $\vec e_i$
is then orthonormalized using the modified Gram-Schmidt procedure,
holding $\vec e_1$ constant. We construct two matrices
\begin{equation}
\boldsymbol{M} = \left( \begin{matrix}
  0 & 0 & 0 \\
  0 & \sigma_i & 0 \\
  0 & 0 & \sigma_i
  \end{matrix}  \right),
\boldsymbol{S}= \left( \begin{matrix}
  \vec e_1 & \vec e_2 & \vec e_3
  \end{matrix}
  \right),
\end{equation}
and set the stress tensor $\boldsymbol{\sigma}$ to
\begin{equation}
\boldsymbol{\sigma} = \left(S\cdot M\cdot S^{-1}\right)\cdot|\nabla\eta|,
\end{equation}
which is a basis transformation from the Eigenvector basis into the cartesian basis.
The $|\nabla\eta|$ factor causes the integral over the stress tensor across the interface
to evaluate the same value, regardless of interfacial width (provided the order parameter
range is well defined - commonly 0 and 1 on the two sides of the interface).

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/interface_stress/test.i block=Materials/interface

!syntax parameters /Materials/ComputeInterfaceStress

!syntax inputs /Materials/ComputeInterfaceStress

!syntax children /Materials/ComputeInterfaceStress
