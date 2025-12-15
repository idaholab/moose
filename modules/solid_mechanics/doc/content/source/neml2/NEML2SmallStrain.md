# NEML2SmallStrain

!if! function=hasCapability('neml2')

For a batch of material points, calculate the small strain given displacement gradients, i.e. $\varepsilon_{ij} = \dfrac{1}{2} \left( u_{i,j} + u_{j,i} \right)$.

## Limitations

- The current formulation assumes Cartesian kinematics; axisymmetric and spherical terms (e.g., hoop strain) are not included.
- Only the current displacement gradients are used; there is no access to old values through this path.

## Syntax

!syntax parameters /UserObjects/NEML2SmallStrain

## Example input files

!syntax inputs /UserObjects/NEML2SmallStrain

!if-end!

!else

!include neml2/neml2_warning.md
