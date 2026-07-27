# Custom Constitutive Models

## Overview id=overview

A constitutive model supplies a stress measure and its algorithmic tangent to the
[stress-divergence kernels](BalanceOfLinearMomentum.md).  To add a new model, derive from one of the
Lagrangian stress base classes and implement the stress update and its tangent in whichever stress and
strain measure is most convenient -- the base class converts your output into the measures each kernel
needs, so the same model works with both the total and updated Lagrangian formulations.

## Base Classes id=base-classes

Choose the base class whose stress and strain measures match your constitutive law:

| Base class                                                                | Stress to implement                                                           | Tangent to implement          |
| ------------------------------------------------------------------------- | ----------------------------------------------------------------------------- | ----------------------------- |
| [`ComputeLagrangianStressCauchy`](ComputeLagrangianStressCauchy.md)       | Cauchy stress $\sigma_{ij}$                                                   | $d\sigma_{ij}/d\Delta l_{kl}$ |
| [`ComputeLagrangianStressPK1`](ComputeLagrangianStressPK1.md)             | 1st Piola-Kirchhoff stress $P_{iJ}$                                           | $dP_{iJ}/dF_{kL}$             |
| [`ComputeLagrangianStressPK2`](ComputeLagrangianStressPK2.md)             | 2nd Piola-Kirchhoff stress $S_{IJ}$ (Green-Lagrange strain $E_{IJ}$ supplied) | $dS_{IJ}/dE_{KL}$             |
| [`ComputeLagrangianObjectiveStress`](ComputeLagrangianObjectiveStress.md) | small (engineering) stress $s_{ij}$                                           | $ds_{ij}/d\varepsilon_{kl}$   |

The first three are the natural choice for models written directly in a finite-strain measure
(hyperelasticity, finite-strain plasticity).  The last, `ComputeLagrangianObjectiveStress`, lets a
small-deformation, rate-form model run in large deformation by integrating an
[objective stress rate](ObjectiveStressRates.md).

## Reference Implementations id=reference-implementations

The module ships several constitutive models built on these base classes; they serve as worked
examples to model a new implementation after:

- [`ComputeHypoelasticStVenantKirchhoffStress`](ComputeHypoelasticStVenantKirchhoffStress.md)
- [`ComputeLagrangianLinearElasticStress`](ComputeLagrangianLinearElasticStress.md)
- [`ComputeNeoHookeanStress`](ComputeNeoHookeanStress.md)
- [`ComputeSimoHughesJ2PlasticityStress`](ComputeSimoHughesJ2PlasticityStress.md)
- [`ComputeStVenantKirchhoffStress`](ComputeStVenantKirchhoffStress.md)

## Porting Legacy Models id=wrapper

Some existing, legacy constitutive models in the solid mechanics module (`StressUpdateBase` descendant) can be
used through [`ComputeLagrangianWrappedStress`](ComputeLagrangianWrappedStress.md), which maps its output into the
format the kernels expect.  See that page for the limitations of the wrapper.

!alert tip
For a large library of ready-made constitutive models, see the
[NEML2 constitutive model library](NEML2Models.md).
