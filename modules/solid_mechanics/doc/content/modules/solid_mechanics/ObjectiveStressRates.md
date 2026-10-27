# Objective Stress Rates

## Overview id=overview

A constitutive model supplies a stress measure and its algorithmic tangent to the
[stress-divergence kernels](BalanceOfLinearMomentum.md).  A model can be written directly in a
large-deformation stress measure -- the Cauchy stress
([`ComputeLagrangianStressCauchy`](ComputeLagrangianStressCauchy.md)), the first Piola-Kirchhoff
stress ([`ComputeLagrangianStressPK1`](ComputeLagrangianStressPK1.md)), or the second Piola-Kirchhoff
stress ([`ComputeLagrangianStressPK2`](ComputeLagrangianStressPK2.md)) -- and the material system
converts it to whichever measure each kernel needs.

Alternatively, a model can be written in the small (engineering) stress and strain and *converted* to
a large-deformation response by integrating an **objective stress rate**.  This is the subject of this
page, implemented by [`ComputeLagrangianObjectiveStress`](ComputeLagrangianObjectiveStress.md).  It
lets an existing small-deformation constitutive model -- including many of the module's
existing stress models -- run in a large-deformation simulation without being rewritten.

## Why an Objective Rate id=why

A constitutive law written in the small (engineering) stress and strain is not frame indifferent:
applied directly in a large-deformation setting, a rigid rotation of the body would spuriously change
the stress.  An *objective* stress rate removes the rotational part of the stress evolution so that
the small-deformation law integrates correctly as the body rotates and deforms.  The conversion is
only needed for large deformation kinematics; under small kinematics the small stress is used
directly, controlled by the [`large_kinematics`](Kinematics.md#large_kinematics) option.

## Integration id=integration

The conversion integrates an objective rate of the Cauchy stress [!cite](simo2006computational).  Most
supported rates take the form
\begin{equation}
   \hat{\sigma}_{ij} = s_{ij} = \dot{\sigma}_{ij} - Q_{ik}\sigma_{kj} - \sigma_{ik}Q_{jk} + Q_{kk}\sigma_{ij}
\end{equation}
where $s_{ij}$ is the small stress supplied by the constitutive model and $Q_{ik}$ is a kinematic
measure whose choice defines the particular rate.  The kinematic increments that drive the
integration -- the spatial velocity gradient increment $\Delta l$ and the vorticity increment
$\Delta w$ -- come from the [strain calculator](Kinematics.md) and depend on the selected
[kinematic approximation](KinematicApproximations.md).  See
[`ComputeLagrangianObjectiveStress`](ComputeLagrangianObjectiveStress.md) for the incremental stress
update and the corresponding algorithmic tangent.

## Available Rates id=rates

Four objective rates are available, selected with the `objective_rate` parameter:

| Rate | `objective_rate` | Notes |
|------|------------------|-------|
| Truesdell (default) | `truesdell` | Avoids the spurious oscillation the Jaumann rate shows at large shear |
| Jaumann | `jaumann` | Matches the implicit ABAQUS integration |
| Green-Naghdi | `green_naghdi` | Rotation taken from the polar decomposition |
| Rashid | `rashid` | Finite rotation by matrix exponential of the incremental vorticity; exact for any step size |

See [`ComputeLagrangianObjectiveStress`](ComputeLagrangianObjectiveStress.md) for the defining
kinematic tensor and algorithmic tangent of each rate.

## Limitations id=limitations

Integrating an objective rate of a small-stress model has well-known limitations
[!cite](simo2006computational, bavzant2014energy):

- +Integrated rotations.+ For the Truesdell, Jaumann, and Green-Naghdi rates the rotational
  kinematics are integrated in time, so large rotations are captured accurately only in the limit of
  small time steps.  The `rashid` rate integrates the rotation in closed form and is exact for any
  step size.
- +Large shears.+ Some rates (notably Jaumann) produce a non-physical, oscillatory stress under very
  large shear.

For problems with very large stretches, a hyperelastic model defined directly in a large-deformation
measure ([`ComputeLagrangianStressPK1`](ComputeLagrangianStressPK1.md) or
[`ComputeLagrangianStressPK2`](ComputeLagrangianStressPK2.md)) is preferable.  See
[`ComputeLagrangianObjectiveStress`](ComputeLagrangianObjectiveStress.md) for a fuller discussion with
illustrative examples.

## Implementation id=implementation

Objective-rate integration is provided by
[`ComputeLagrangianObjectiveStress`](ComputeLagrangianObjectiveStress.md), with
[`ComputeLagrangianObjectiveCustomStress`](ComputeLagrangianObjectiveCustomStress.md) available for
models that supply their own stress form.  Select the rate with the `objective_rate` parameter; the
default is `truesdell`.

!bibtex bibliography
