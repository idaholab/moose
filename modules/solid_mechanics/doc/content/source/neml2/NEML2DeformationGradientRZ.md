# NEML2DeformationGradientRZ

!if! function=hasCapability('neml2')

Axisymmetric (RZ) version of [NEML2DeformationGradient.md]. For a batch of material points, calculate the deformation gradient from displacement gradients on the reference configuration. In addition to the in-plane components, the out-of-plane hoop stretch is included:

!equation
F_{\theta\theta} = 1 + \dfrac{u_r}{r}

Pair with [NEML2StressDivergenceRZ.md] and a total-Lagrangian NEML2 model that outputs the first Piola-Kirchhoff stress.

## Limitations

- Requires an axisymmetric (RZ) coordinate system and exactly two displacement variables (radial, axial), ordered consistently with the coordinate axes.
- No torsion: the circumferential displacement is assumed to be zero.

## Syntax

!syntax parameters /UserObjects/NEML2DeformationGradientRZ

## Example input files

!syntax inputs /UserObjects/NEML2DeformationGradientRZ

!if-end!

!else

!include neml2/neml2_warning.md
