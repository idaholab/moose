# NEML2DeformationGradient

!if! function=hasCapability('neml2')

!syntax description /UserObjects/NEML2DeformationGradient

## Description

Large-deformation companion of [NEML2SmallStrain.md]. For a batch of material
points, this object calculates the deformation gradient from displacement
gradients on the reference configuration:

!equation
F = I + \nabla_0 u

The full, non-symmetrized second-order tensor is passed to the NEML2 model.
Total-Lagrangian constitutive chains, such as Green-Lagrange strain,
multiplicative decompositions, and objective rates, can therefore be composed
inside NEML2. Pair this object with [NEML2StressDivergence.md] and a model that
outputs the first Piola-Kirchhoff stress. The weak form
$\psi^\alpha_{,J}P_{iJ}$, integrated with the cached reference-configuration
quadrature, is the total-Lagrangian residual.

## Limitations

- This object uses Cartesian kinematics. For axisymmetric kinematics, including
  $F_{\theta\theta} = 1 + u_r / r$, use [NEML2DeformationGradientRZ.md].
- Requires 1 to 3 displacement variables; 2D meshes yield plane-strain kinematics ($F_{zz} = 1$).

## Parameters

!syntax parameters /UserObjects/NEML2DeformationGradient

## Inputs

!syntax inputs /UserObjects/NEML2DeformationGradient

## Child Objects

!syntax children /UserObjects/NEML2DeformationGradient

!if-end!

!else

!include neml2/neml2_warning.md
