# NEML2DeformationGradient

!if! function=hasCapability('neml2')

Large-deformation companion of [NEML2SmallStrain.md]. For a batch of material points, calculate the deformation gradient from displacement gradients on the reference configuration:

!equation
F = I + \nabla_0 u

The full (non-symmetrized) second-order tensor is passed to the NEML2 model, so total-Lagrangian constitutive chains (e.g. Green-Lagrange strain, multiplicative decompositions, objective rates) can be composed inside NEML2. Pair with [NEML2StressDivergence.md] and a model that outputs the first Piola-Kirchhoff stress: the weak form $\psi^\alpha_{,J} P_{iJ}$ integrated with the cached reference-configuration quadrature is exactly the total-Lagrangian residual.

## Limitations

- Cartesian kinematics only; the axisymmetric hoop component $F_{\theta\theta} = 1 + u_r / r$ is not yet included.
- Requires 1 to 3 displacement variables; 2D meshes yield plane-strain kinematics ($F_{zz} = 1$).

## Syntax

!syntax parameters /UserObjects/NEML2DeformationGradient

## Example input files

!syntax inputs /UserObjects/NEML2DeformationGradient

!if-end!

!else

!include neml2/neml2_warning.md
