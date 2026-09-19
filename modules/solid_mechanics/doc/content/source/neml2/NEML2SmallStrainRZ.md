# NEML2SmallStrainRZ

!if! function=hasCapability('neml2')

!syntax description /UserObjects/NEML2SmallStrainRZ

## Description

This object is the axisymmetric (RZ) version of [NEML2SmallStrain.md]. For a
batch of material points, it calculates the small strain from displacement
gradients. In addition to the in-plane components, it includes the
out-of-plane hoop strain:

!equation
\varepsilon_{\theta\theta} = \dfrac{u_r}{r}

## Limitations

- Requires an axisymmetric (RZ) coordinate system and exactly two displacement
  variables in coordinate-axis order: one radial and one axial.
- No torsion: the circumferential displacement is assumed to be zero.
- Only the current displacement values/gradients are used; there is no access to old values through this path.

## Parameters

!syntax parameters /UserObjects/NEML2SmallStrainRZ

## Inputs

!syntax inputs /UserObjects/NEML2SmallStrainRZ

## Child Objects

!syntax children /UserObjects/NEML2SmallStrainRZ

!if-end!

!else

!include neml2/neml2_warning.md
