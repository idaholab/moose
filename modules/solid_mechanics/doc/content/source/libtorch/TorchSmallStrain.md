# TorchSmallStrain

!if! function=hasCapability('neml2')

For a batch of material points, calculate the small strain given displacement gradients, i.e. $\varepsilon_{ij} = \dfrac{1}{2} \left( u_{i,j} + u_{j,i} \right)$, and provide it as a NEML2 model input (in 6-component Mandel form).

## Limitations

- The current formulation assumes Cartesian kinematics; axisymmetric and spherical terms (e.g., hoop strain) are not included.
- Only the current displacement gradients are used; there is no access to old values through this path.

## Syntax

!syntax parameters /UserObjects/TorchSmallStrain

## Example input files

!syntax inputs /UserObjects/TorchSmallStrain

!if-end!

!else

!include neml2/neml2_warning.md
