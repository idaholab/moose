# NEML2DeformationGradientRZ

!if! function=hasCapability('neml2')

!syntax description /UserObjects/NEML2DeformationGradientRZ

## Description

This object is the axisymmetric (RZ) version of
[NEML2DeformationGradient.md]. For a batch of material points, it calculates
the deformation gradient from displacement gradients on the reference
configuration. In addition to the in-plane components, it includes the
out-of-plane hoop stretch:

!equation
F_{\theta\theta} = 1 + \dfrac{u_r}{r}

Pair with [NEML2StressDivergenceRZ.md] and a total-Lagrangian NEML2 model that outputs the first Piola-Kirchhoff stress.

## Limitations

- Requires an axisymmetric (RZ) coordinate system and exactly two displacement
  variables in coordinate-axis order: one radial and one axial.
- No torsion: the circumferential displacement is assumed to be zero.

## Parameters

!syntax parameters /UserObjects/NEML2DeformationGradientRZ

## Inputs

!syntax inputs /UserObjects/NEML2DeformationGradientRZ

## Child Objects

!syntax children /UserObjects/NEML2DeformationGradientRZ

!if-end!

!else

!include neml2/neml2_warning.md
