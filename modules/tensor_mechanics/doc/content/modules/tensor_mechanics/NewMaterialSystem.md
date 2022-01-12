# New Material System for the Lagrangian Kernels

## Purpose and Implementations

The material system for the new Lagrangian kernels consists of:

- The strain calculator for the new system, [`ComputeLagrangianStrain`](ComputeLagrangianStrain.md)
- A stress calculator, implemented as a derived class based on either
  [`ComputeLagrangianStressCauchy`](ComputeLagrangianStressCauchy.md) or
  [`ComputeLagrangianStressPK1`](ComputeLagrangianStressPK1.md)
- Optionally, additional material objects part of the [homogenization system](tensor_mechanics/Homogenization.md).


The objective of these objects is to provide the stress update needed by *both* the
[`TotalLagrangianStressDivergence`](TotalLagrangianStressDivergence.md) and the
[`UpdatedLagrangianStressDivergence`](UpdatedLagrangianStressDivergence.md) kernels.
The user only needs to define the "main" stress measure and an associated derivative, listed in [stressoptions]
below.
The base class code then takes care of translating that stress and associated derivative into the
"missing" stress and derivative.
Similarly, subclasses that derive from either [`ComputeLagrangianStressCauchy`](ComputeLagrangianStressCauchy.md)
or [`ComputeLagrangianStressPK1`](ComputeLagrangianStressPK1.md) can provide an interface for
defining the constitutive model in terms of some other stress measure and associated derivative.

To summarize: to implement a new material model make a class that inherits from [`ComputeLagrangianStressCauchy`](ComputeLagrangianStressCauchy.md)
or [`ComputeLagrangianStressPK1`](ComputeLagrangianStressPK1.md) (or an associated subclass) and implement the required
stress update and associated algorithmic tangent.  The resulting model can be used with either kernel
formulation.

## Using existing MOOSE materials

The kernels are compatible with "current" MOOSE materials derived from [StressUpdateBase](Stresses.md) through use
of the [ComputeLagrangianWrappedStress](ComputeLagrangianWrappedStress.md) wrapper.  The user can just define their material
using the existing MOOSE material system and also supply the wrapper material object in the input file.  There are some limitations
to this wrapper, discussed in more detailed in the [wrapper documentation](ComputeLagrangianWrappedStress.md).

New materials could be derived from one of several base classes provided by the new material system, described here, to remove 
these limitations.

## Strain Calculator

It does not matter what kinematic quantities you use to define the stress update, so long as in the end the material 
model returns the required stress and associated algorithmic tangent derivative.  The Lagrangian kernel
strain calculation, [`ComputeLagrangianStrain`](ComputeLagrangianStrain.md), provides a common set of kinematic
quantities that the material model can use, listed in [kinematics].  Some of the material subclasses providing interfaces for
alternative stress measures provide additional, common kinematic quantities, described in [stressoptions].

### Stabilization

As described in the [description of the stabilization system](/tensor_mechanics/Stabilization.md) and
in the [strain calculator](ComputeLagrangianStrain.md), these standard kinematic quantities and all other kinematic
measures derived from them are altered by the `stabilize_strain` option to stabilize problems with incompressible and 
near-incompressible deformation using linear quad or hex elements.

### Eigenstrains

The strain calculator takes the sum of any eigenstrains, $\Delta \varepsilon_{ij}^{(eigen)}$, for example from [ComputeThermalExpansionEigenstrain](/ComputeThermalExpansionEigenstrain.md), 
and subtracts them from the incremental total strain.  As such, material models can use the incremental mechanical strain to define
constitutive models that include the effects of thermal strains and other eigenstrains.

!alert note
The eigenstrains do not affect the deformation gradient, the inverse deformation gradient, the inverse incremental deformation gradient,
and any kinematic measures derived from these quantities.

### Homogenization system

The strain calculator adds any extra homogenization gradient supplied by [ComputeHomogenizedLagrangianStrain](ComputeHomogenizedLagrangianStrain.md)
to the  deformation gradient defined directly from the displacement gradients before forming any of the derived kinematic measures.
As such, the homogenization system affects constitutive models defined in terms of any kinematic tensor (unlike the eigenstrain modification).

### Standard Kinematic Quantities

The strain calculator uses the spatial velocity gradient, defined as:
\begin{equation}
      \Delta l_{ij} = \delta_{ij} - f^{-1}_{ij}
\end{equation}
for large deformations and
\begin{equation}
      \Delta l_{ij} = F_{ij}^{(new)} - F_{ij}^{(old)}
\end{equation}
for small deformations, to define the incremental strains.  [kinematics] provides the definition of the other standard kinematic variables.

!table id=kinematics caption=Standard kinematic quantities provided by `ComputeLagrangianStrain`
| Quantity name                            | Definition, `large_kinematics=true`                                           | Definition, `large_kinematics=false`                       |
|------------------------------------------|-------------------------------------------------------------------------------|------------------------------------------------------------|
| Deformation gradient                     | $F_{iJ} = \delta_{iJ} + \frac{\partial u_i}{\partial X_J}$                    | $F_{ij} = \delta_{ij} + \frac{\partial u_i}{\partial x_j}$ |
| Inverse deformation gradient             | $F^{-1}_{Ji}$                                                                 | $\delta_{ji}$                                              |
| Inverse incremental deformation gradient | $f^{-1}_{ij} = F^{(old)}_{iK} F^{(new) -1}_{Kj}$                              | $f_{ij} = \delta_{ij}$                                     |
| Volume change                            | $J = \det F$                                                                  | $J = 1$                                                    |
| Total strain increment                   | $\Delta d_{ij} = \frac{1}{2} \left( l_{ij} + l_{ji} \right)$                  | Same                                                       |
| Mechanical strain increment              | $\Delta \varepsilon_{ij} = \Delta d_{ij} - \Delta \varepsilon_{ij}^{(eigen)}$ | Same                                                       |
| Total strain                             | $d_{ij}^{(new)} = d_{ij}^{(old)} + \Delta d_{ij}$                             | Same                                                       |
| Mechanical strain                        | $\varepsilon_{ij} = \varepsilon_{ij}^{(old)} + \Delta \varepsilon_{ij}$       | Same                                                       | 

Except for the deformation gradient itself, all the large deformation quantities used to map configurations are set to the identity 
for `large_kinematics = false`, leaving only the strain tensors defined.

The strains are defined incrementally in terms of the inverse incremental deformation gradient (or the increment in the gradient for small displacement kinematics).
The calculator [stabilizes](/tensor_mechanics/Stabilization.md) the deformation gradient adds any homogenization gradient from the [homogenization system](tensor_mechanics/Homogenization.md)
before calculating the incremental and accumualted strains, and so these modifications propogate through all the available kinematic measures.
However, the eigenstrains are currently defined incrementally and so they will not affect the deformation gradient, the inverse deformation gradient,
and the incremental deformation gradient.

## The `large_kinematics` Option

The [`ComputeLagrangianStrain`](ComputeLagrangianStrain.md) class uses the `large_kinematics` option to switch between
large strain kinematics and small strain kinematics.  This option must be set consistently between the strain 
calculator and the kernels.  The [TensorMechanics/MasterAction](/Modules/TensorMechanics/Master/index.md) can
be used to automatically setup the kernels and strain calculator with consistent options by using the `strain=SMALL` or
`strain=FINITE` options.

Material models do not *need* to use the `large_kinematics` option and there is no requirement it be set consistently
with the strain calculator and the kernel.
However, material models can use this option to provide variations of the model, often a linearized version using the
for `large_kinematics = false` and a nonlinear version for `large_kinematics = true`.
Oftentimes it makes sense to keep the `large_kinematics` option consistent between the stress calculator, strain 
calculator, and kernels.  The [TensorMechanics/MasterAction](/Modules/TensorMechanics/Master/index.md) cannot
set the option for the stress calculator automatically, so it is up to the user to specify it in the input file.

## Base Class Options

[stressoptions] lists the base classes available for implementing constitutive models and provides a link to more detailed
documentation providing the relevant conversion formula.  The table also describes any extra kinematic measures
provided by the base class, and the stress and algorithmic tangent (Jacobian) tensors the user needs to implement
to define the model. 

!table id=stressoptions caption=Base classes available for defining constitutive models
| Base class name                                                     | Extra kinematic measures                                                             | Stress measure                               | Required Jacobian                       |
|---------------------------------------------------------------------|--------------------------------------------------------------------------------------|----------------------------------------------|-----------------------------------------|
| [`ComputeLagrangianStressCauchy`](ComputeLagrangianStressCauchy.md) | None                                                                                 | Cauchy stress, $\sigma_{ij}$                 | $\frac{d \sigma_{ij}}{d \Delta l_{kl}}$ |
| [`ComputeLagrangianStressPK1`](ComputeLagrangianStressPK1.md)       | None                                                                                 | 1st Piola-Kirchhoff stress, $P_{iJ}$         | $\frac{d P_{iJ}}{d F_{kL}}$             |
| [`ComputeLagrangianStressPK2`](ComputeLagrangianStressPK2.md)       | Green-Lagrange strain, $E_{IJ} = \frac{1}{2}\left(F_{kI}F_{kJ} - \delta_{IJ}\right)$ | 2nd Piola-Kirchhoff stress, $S_{IJ}$         | $\frac{d S_{IJ}}{d E_{IJ}}$             |
| [`ComputeLagrangianObjectiveStress`](ComputeLagrangianObjectiveStress.md)   | None                                                                                 | Small stress, $s_{ij}$                       | $\frac{d s_{ij}}{d \varepsilon_{kl}}$   |



