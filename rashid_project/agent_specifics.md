# Updating solid_mechanics to unify material behavior

## Overview

The `solid_mechanics` module has "new" kernels (called the Lagrangian kernels, will all live in subdirectories) and "old" kernels, i.e. those based on `StressDeivergenceTensors`.  The two sets of kernels have their own corresponding material system.  The goal of this project is to make the materials originally written for the "old" kernels, based on `ComputeStressBase` and `ComputeStrainBase`, fully usable in the Lagrangian kernel system.

There is already partial interoperability using the `ComputeLagrangianWrappedStress` class.  This works perfectly for small deformation kinematics simulations.  However, there is a gap when using large deformation kinematics.

That gap is that the original system assumes the material update is mediated through the Rashid objective rate (see `additional_math.pdf` in this directory).  The Lagrangian kernels have a variety of objective stress rates provided, but not the specific Rashid formulation.  The goal of the projec then is to be able to emulate the Rashid behavior in the Lagrangian kernel system in general, and specifically to enable one-to-one compatibility of large deformation simulations using the same (old style) material models in both the old and new kernels.

## Plan

I put together `plan_outline.pdf` to describe how to go about doing this.  Basically we have four tasks, each of which is a complete feature:
0. Read the section below and find all the locations where we tacitly use the derivative of the increment in the spatial velocity gradient with respect to the deformation gradient.  Pre-calculate the derivative of the spatial velocity gradient increment with respect to the deformation gradient and the deformation gradient with respect to the displacement gradient (will be the identity initially) in the strain calculator and then use this quantity in the kernels and the homogenization constraint system, rather the "assumed" derivative we're working with now.
1. Calculate the deformation gradient in `ComputeLagrangianStrain` with the generalized alpha rule, rather than backward Euler.  We don't need this for Rashid, but it helps us match Abaqus Implicit.
2. Provide the suite of deformation/vorticity increment calculators as described in Section 2 of `plan_outline.pdf`.
3. Update the Lagrangian objective rate system to add the Rashid model and take advantage of what we did in the previous step, specifically:
    1. Alter the calculation of the Green-Naghdi rate to use the new kinematic quantities we setup in Step 2.
    2. Add a new subclass to actually calculate the Rashid rate, as we can't cast it into a linear form like we can with the existing options.
4. Update the physics action for solid mechanics to provide default sets of these new material options to make it easy to setup different material options:
    1. Default Lagrangian behavior with objective rates (alpha = 1.0, linear kinematic approximation, Truesdell rate)
    2. Old kernel style Rashid model (alpha = 1.0, "rashid" kinematics, Rashid objective rate).
    3. Old kernel style Rashid model with "eigen" decomposition (alpha = 1.0, "exact" kinematics, Rashid objective rate).
    4. Abaqus/Implicit (alpha = 0.5, linear kinematics, Jaumann rate).

We'll work on this one step at a time.

## A problem

There is an annoyance in the current implementation of the Lagrangian kernel system in that the derivative of the increment in the kinematic quantities with respect to the displacement gradient (which you can view as the chain rule of the derivative of the increment with respect to the deformation gradient and then the deformation gradient with respect to the displacement gradient) is assumed to be equal to the derivative of the approximation we are currently using (commit eddcae55b483a746e08d5efd18432744bdc97de9) of the increment in the spatial velocity gradient.  This is what we're calling the "linear" approximation.  We will need to modularize these derivatives (I would suggest storing both the derivative of the kinematic increment with respect to the deformation gradient and the deformation gradient with respect to the displacement gradient) by storing them in the strain calculator.

Then we'll need to find all the locations we use the "assumed" derivative and replace them with the chain rule using the stored quantities.  I know this happens in the kernels themselves as well as in the homogenization constraint system.  I'm afraid it may  happen elsewhere as well; you'll need to look.

## Final tests

After all this is done and tested we will want to add some tests demonstrating agreement between the "new" and "old" kernels for the Rashid model, using "old" style plasticity material models, and including with the "rashid" fourth order approximate kinematics and the "exact" kinematics (which the old kernels call "eigen").

## Notes

You, the LLM, can use this section for storing notes after completing each task to make restarting things easy between sessions.  We will check in this file on the feature branch, but remove prior to the final merge.  Please write some changelog type notes here after successfully committing the change completing a task in the general plan above.

### Step 0 (2026-05-20) — modularize the kinematic derivatives

Branch `materials_cross_compatibility`.

`ComputeLagrangianStrainBase` now declares and populates two new `RankFourTensor` material properties:
- `d_spatial_velocity_increment_d_deformation_gradient` — `d(dL)/dF_{n+1}` (= `f^{-1}_{km} F^{-1}_{nl}` for the linear approximation, identity-rank-4 in the small-kinematics branch).
- `d_deformation_gradient_d_grad_displacement` — `dF_{n+1}/d(grad u_{n+1})` (= identity-rank-4 for backward Euler; Step 1 will scale by alpha).

`_spatial_velocity_increment` semantics changed from "vorticity + mechanical strain" to the **full kinematic dL** (no eigenstrain subtraction). The objective-rate advection in `ComputeLagrangianObjectiveStress::objectiveUpdateTruesdell` now consumes this directly. `_strain_increment` and `_mechanical_strain` are still post-eigenstrain (unchanged).

Consumers updated to use the stored derivatives instead of inline `f`/`f^{-1}` factors:
- `UpdatedLagrangianStressDivergence::computeQpJacobianDisplacement` — chain through `_d_F_d_grad_u : _d_spatial_velocity_increment_d_F` instead of `_f_inv * grad_trial`. In UL with large kinematics, `grad_trial` is pulled back through `F` first.
- `ComputeLagrangianObjectiveStress::objectiveUpdateJaumann` — consumes `_vorticity_increment` directly.
- `ComputeLagrangianObjectiveStress::objectiveUpdateGreenNaghdi` — uses `_d_spatial_velocity_increment_d_F.inverse()` instead of inline `f.times<...>(F)` for `dF/d(dL)`.
- `ComputeLagrangianStressCauchy::computeQpPK1Stress` — replaces the inner `_inv_df.transpose()` factor of `tripleProductJkl` with an explicit chain through `_d_spatial_velocity_increment_d_F`.
- `ComputeLagrangianStressPK1::computeQpCauchyStress` — rebuilds `dsigma/dF` from the three contributions of `sigma = (1/J) P F^T`, then chains through `_d_spatial_velocity_increment_d_F.inverse()`.
- `ComputeHypoelasticStVenantKirchhoffStress::computeQpSmallStress` — `dFddL` comes from `_d_spatial_velocity_increment_d_F.inverse()` rather than `f.times<...>(F)`.

Full `solid_mechanics` test suite green (1697/1697). All 219 lagrangian tests pass.

Gotcha encountered: the original `_spatial_velocity_increment` was eigenstrain-subtracted (mechanical), which is the wrong quantity for objective-rate advection. The Cartesian thermal-expansion jacobian tests hid this because their `jactest.i` has no temperature BCs and the eigenstrain is zero at the test state. Only the axisymmetric thermal-expansion jacobian tests, which set `T_left=0, T_right=1`, exposed it.

### Step 1 (2026-05-20) — generalized midpoint deformation gradient

`ComputeLagrangianStrainBase` now accepts a range-checked `alpha` parameter (default 1.0, range [0.5, 1.0]) and computes the deformation gradient via the generalized midpoint rule:
  `F^alpha_{n+1} = I + alpha * (grad u_{n+1}, u_{n+1}) + (1 - alpha) * (grad u_n, u_n)`
The two contributions are accumulated with two `addGradOp` calls, so the existing coordinate-system polymorphism (Cartesian / axisymmetric / centrosymmetric) is preserved. With a Steady executioner the old contribution is treated as identically zero (F_n = I) so the alpha rule works in both transient and steady mode.

`_d_F_d_grad_u` is now populated as `alpha * IdentityFour` instead of `IdentityFour`. The strain calculator also declares a new property `actual_deformation_gradient` = `I + grad u_{n+1}` (no alpha weighting, no F-bar), since the UL kernel's spatial-to-reference pull-back needs the literal F at n+1 when `alpha != 1`.

Kernel changes:
- `LagrangianStressDivergenceBase` gets its own `alpha` parameter (defaults to 1.0). The user is expected to set `alpha` via `GlobalParams` so both the kernel and the strain material see the same value.
- `UpdatedLagrangianStressDivergenceBase::computeQpJacobianDisplacement` picks `_F_actual` for the spatial pull-back when `_kinematic_alpha != 1.0` and stays on `_F` (the alpha-weighted, F-bar-stabilized F) when `_kinematic_alpha == 1.0`. The alpha=1 branch preserves the pre-Step-0 "self-consistent f^{-1} * grad_trial" pattern that F-bar tests rely on; the alpha != 1 branch uses the literal F at n+1 so the chain rule is mathematically correct.
- `TotalLagrangianStressDivergenceBase::computeQpJacobianDisplacement` multiplies the existing `gradTest : (_dpk1 * gradTrial)` by `_kinematic_alpha`, since `_dpk1 = dP/dF_alpha` and the chain to grad u_{n+1} carries an alpha factor.

Test additions (15 total):
- `lagrangian/axisymmetric_cylindrical/total/jacobian` — 4 alpha=0.5 variants (dirichlet/neumann × stab/no-stab).
- `lagrangian/centrosymmetric_spherical/total/jacobian` — 4 alpha=0.5 variants.
- `lagrangian/cartesian/total/rates` — 3 alpha=0.5 variants (truesdell, jaumann, green_naghdi).
- `lagrangian/cartesian/total/thermal_expansion` — 2 alpha=0.5 variants (with and without stab).
- `lagrangian/cartesian/total/generalized_alpha` — new directory with two correctness CSVDiff tests (alpha=0.5 and alpha=1.0). A single-element 3D uniaxial extension with linearly-ramped Dirichlet BC checks that the postprocessor-read `F^alpha_{11}` matches `1 + alpha*(0.01*t_n) + (1-alpha)*(0.01*t_{n-1})` exactly and that `F^actual_{11}` matches `1 + 0.01*t_n`.

Known limitation flagged in `lagrangian/axisymmetric_cylindrical/total/thermal_expansion/tests`: the temperature off-diagonal Jacobian in `TotalLagrangianStressDivergenceBase::computeQpJacobianTemperature` uses a linear-chain `U = sym(f_alpha ⊗ F_alpha)` that matches FD for `alpha=1` in any coordinate system and for `alpha=0.5` in Cartesian, but does NOT match FD for `alpha=0.5` in axisymmetric/centrosymmetric coordinates (the gradTest there picks up a hoop term that the U formula does not account for). A proper fix needs a more careful re-derivation of the eigenstrain-to-PK1 chain for alpha != 1 in non-Cartesian coordinates; deferred.

Full `solid_mechanics` suite green: 1712 / 1712 (1697 baseline + 15 new alpha=0.5 / correctness tests).

Tip for future steps: `_kinematic_alpha != 1` requires the user to set `alpha` on BOTH the strain material and the kernel. Recommend setting via `GlobalParams/alpha=0.5` so both pick it up automatically — this is the pattern the new test cli_args follow.
