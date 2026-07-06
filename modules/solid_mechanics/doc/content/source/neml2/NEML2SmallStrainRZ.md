# NEML2SmallStrainRZ

!if! function=hasCapability('neml2')

Axisymmetric (RZ) version of [NEML2SmallStrain.md]. For a batch of material points, calculate the small strain given displacement gradients. In addition to the in-plane components, the out-of-plane hoop strain is included:

!equation
\varepsilon_{\theta\theta} = \dfrac{u_r}{r}

## Limitations

- Requires an axisymmetric (RZ) coordinate system and exactly two displacement variables (radial, axial), ordered consistently with the coordinate axes.
- No torsion: the circumferential displacement is assumed to be zero.
- Only the current displacement values/gradients are used; there is no access to old values through this path.

## Syntax

!syntax parameters /UserObjects/NEML2SmallStrainRZ

## Example input files

!syntax inputs /UserObjects/NEML2SmallStrainRZ

!if-end!

!else

!include neml2/neml2_warning.md
