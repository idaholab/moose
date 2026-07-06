# NEML2HourglassCorrection

!if! function=hasCapability('neml2')

!syntax description /UserObjects/NEML2HourglassCorrection

## Description

Batched hourglass stabilization for reduced-integration (single quadrature
point) explicit dynamics through the NEML2 nodal-force path. For every element
of the batch and every displacement component, the least-squares affine part
of the nodal displacement is removed and the remaining hourglass modes
(one pair for `QUAD4`, the four Flanagan-Belytschko vectors for `HEX8`) are
penalized with the rotation-invariant scale
$c = \texttt{penalty}\,\mu\,V/h^2$, $h^2 = \operatorname{tr}\mathbf{A}/d$,
operating on the displaced geometry reconstructed from the cached reference
node coordinates plus the current nodal displacements.

The result is added to the tagged residual vector alongside
[NEML2StressDivergence.md]. In axisymmetric (RZ) coordinates the integration
weight includes the $2\pi \bar r$ factor of the displaced element.

The implementation matches the per-element kernels
[HourglassCorrectionQuad4.md] and [HourglassCorrectionHex8.md] at machine
precision (see the `hourglass_*_validation` tests in
`test/tests/neml2/explicit_dynamics`).

## Syntax

!syntax parameters /UserObjects/NEML2HourglassCorrection

!syntax inputs /UserObjects/NEML2HourglassCorrection

!if-end!

!else

!include neml2/neml2_warning.md
