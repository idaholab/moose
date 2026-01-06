# Density Scaling

!syntax description /Materials/DensityScaling

## Description

This material computes the density required to achieve stable explicit time stepping with a user-defined time step (see [CriticalTimeStep](/CriticalTimeStep.md) for the stability condition).  Depending on the advanced `additive_contribution` parameter, this Material computes:

- when `additive_contribution = false`: the density needed to enable time-stepping with given `desired_time_step`;
- when `additive_contribution = true`: the portion of the density that should be added to the true density to enable the desired time stepping, that is max(0, density needed minus true density).

Note that the density computed acts as your model's *inertial* density (see example below) and you should not use it as your *gravitational* density.

When used in the [MassMatrix](MassMatrix.md), time-steps of size `desired_time_step` will be stable.  However, note that the addition of mass will alter the dynamics of the system.  In particular, high-frequency oscillations will largely be eliminated in elements that are small and/or stiff and/or light.  Hence, using mass scaling is particularly recommended when the finite element mesh contains a handful of small/stiff/light elements, or when high-frequency dynamics are unimportant.  Mass scaling has a smaller, yet noticeable, impact on low-frequency dynamics.

## Example

To use `DensityScaling` effectively, two steps are needed.  The following is a worked example.

#### Step 1

To ensure that a user-defined time step ($\Delta t = 4$ in this case) is stable, use a `DensityScaling` Material to compute the required density in each element:

!listing modules/solid_mechanics/test/tests/dynamics/time_integration/mass_scaling.i start=[density_true] end=[Elasticity_tensor]

The important features here are:

- A Material Property called `density_true` is calculated by some Material (in this case, the `density_true` Material) and fed into the `DensityScaling` Material
- The `desired_time_step` is defined in the `DensityScaling` Material
- The `DensityScaling` Material computes the density required for stable time-stepping, then increases it by multiplying it by 1/`safety_factor`.  For example, in this case, $\Delta t = 4 / 0.8 = 5$ is theoretically stable, but for safety, the input file only uses $\Delta t = 4$.
- The `DensityScaling` Material stores the computed density in `density_scaled` (or whatever is specified by the `scaled_density` parameter).

The other specified parameters, such as `implicit = false`, `output_properties`, `outputs`, are unimportant here (eg, the outputs are just so the result is stored in the exodus file).

#### Step 2

Ensure your MassMatrix uses the scaled density.  For instance:

!listing modules/solid_mechanics/test/tests/dynamics/time_integration/mass_scaling.i start=[massmatrix_x] end=[massmatrix_y]


!syntax parameters /Materials/DensityScaling

!syntax inputs /Materials/DensityScaling

!syntax children /Materials/DensityScaling

!bibtex bibliography
