# NEML2HourglassCorrection

!if! function=hasCapability('neml2')

!syntax description /UserObjects/NEML2HourglassCorrection

## Description

`NEML2HourglassCorrection` provides batched hourglass stabilization for
reduced-integration explicit dynamics in the NEML2 nodal-force path. It
supports `QUAD4` elements with two displacement variables, including
axisymmetric coordinates, and `HEX8` elements with three displacement
variables.

The implementation projects the single classical `QUAD4` mode or the four
classical `HEX8` modes out of each element's physical affine displacement
space. For each projected mode $\widehat{\boldsymbol{\gamma}}^{(a)}$ and
displacement component $u$,

\begin{equation}
H_a=\sum_i\widehat{\gamma}_i^{(a)}u_i,\qquad
R_i=\texttt{penalty}\,\mu\,\frac{W}{h^2}
\sum_a\widehat{\gamma}_i^{(a)}H_a,\qquad
h^2=\frac{1}{d}\operatorname{tr}\boldsymbol{A}.
\end{equation}

Here $W$ is the single-quadrature-point integration measure. It is the element
area in two dimensions, the element volume in three dimensions, and includes
$2\pi\bar r$ for axisymmetric elements. The current geometry is reconstructed
from cached reference node coordinates and current nodal displacements.

The residual is added to the requested tagged vector alongside
[NEML2StressDivergence.md]. The formulation matches
[HourglassCorrectionQuad4.md] and [HourglassCorrectionHex8.md].

## Parameters

!syntax parameters /UserObjects/NEML2HourglassCorrection

## Inputs

!syntax inputs /UserObjects/NEML2HourglassCorrection

## Child Objects

!syntax children /UserObjects/NEML2HourglassCorrection

!if-end!

!else

!include neml2/neml2_warning.md
