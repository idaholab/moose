# NEML2StressDivergenceRZ

!if! function=hasCapability('neml2')

!syntax description /UserObjects/NEML2StressDivergenceRZ

## Description

This object is the axisymmetric (RZ) version of
[NEML2StressDivergence.md]. For a batch of material points, it calculates the
residual at each point. Compared with the Cartesian weak form, the radial
residual also contains the hoop term

!equation
\psi^\alpha \dfrac{\sigma_{\theta\theta}}{r}

where the volume element already contains the coordinate transformation factor
$2\pi r$. The stress may be a symmetric small/Cauchy stress in Mandel notation
(SR2) or a full stress such as the first Piola-Kirchhoff stress (R2) from a
total-Lagrangian model paired with [NEML2DeformationGradientRZ.md].

The kernel then assembles the integrated residual into the global residual vector.

## Limitations

- Requires an axisymmetric (RZ) coordinate system and exactly two displacement
  variables in coordinate-axis order: one radial and one axial.
- No torsion: the circumferential displacement is assumed to be zero.
- The weak form is integrated on the reference configuration, consistent with
  the small-strain assumption and cached assembly geometry.
  [StressDivergenceRZTensors.md] defaults to the displaced mesh; set
  `use_displaced_mesh = false` there for a matched comparison.
- This object assembles residuals only; no Jacobian contributions are produced.
  This system currently targets only explicit solves.
- Pair this with matching, block-restricted `NEML2Assembly` and
  `NEML2FEInterpolation` objects if the mesh has mixed element types or orders.

## Parameters

!syntax parameters /UserObjects/NEML2StressDivergenceRZ

## Inputs

!syntax inputs /UserObjects/NEML2StressDivergenceRZ

## Child Objects

!syntax children /UserObjects/NEML2StressDivergenceRZ

!if-end!

!else

!include neml2/neml2_warning.md
