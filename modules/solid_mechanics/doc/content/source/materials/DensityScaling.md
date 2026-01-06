# Density Scaling

!syntax description /Materials/DensityScaling

## Description

This Material computes the inertial density needed to enable stable explicit time-stepping in solid-mechanics problems (see [CriticalTimeStep](/CriticalTimeStep.md)).  It outputs:

- when `additive_contribution = false`: the inertial density needed to enable time-stepping with given `desired_time_step`; 
- when `additive_contribution = true`: the portion of intertial density that should be added to the true density to enable the desired time stepping, that is max(0, inertial density needed minus true inertial density).

When used in the [MassMatrix](MassMatrix.md), time-steps of size `desired_time_step` will be stable.  However, note that the addition of mass will alter the dynamics of the system.  In particular, high-frequency oscillations will largely be eliminated in elements that are small and/or stiff and/or light.  Hence, using mass scaling is particularly recommended when the finite element mesh contains a handful of small/stiff/light elements, or when high-frequency dynamics are unimportant.

## Example

To use `DensityScaling` effectively, two steps are needed.  The following is a worked example.

#### Step 1

To ensure that a user-defined time step ($\Delta t = 4$ in this case) is stable, use a `DensityScaling` Material to compute the required density in each element:

!listing modules/solid_mechanics/test/tests/dynamics/time_integration/mass_scaling.i start=[density_true] end=[Elasticity_tensor]

The important features here are:

- A Material Property called `density_true` is calculated by some Material (in this case, the `density_true` Material) and fed into the `DensityScaling` Material
- The `desired_time_step` is defined in the `DensityScaling` Material
- The `safety_factor` is set.  What happens is that the `DensityScaling` Material computes the density required for stable time-stepping, then increases it using 1/`safety_factor`.  This means that, in this case, $\Delta t = 4 / 0.8 = 5$ is theoretically stable, but for safety, the input file only uses $\Delta t = 4$.
- The `DensityScaling` Material stores the computed density in `density_scaled` (or whatever is specified by the `scaled_density` parameter).

The other specified parameters, such as `implicit = false`, `output_properties`, `outputs`, are unimportant here (eg, the outputs are just so the result is stored in the exodus file).

#### Step 2

Ensure your MassMatrix uses the scaled density.  For instance:

!listing modules/solid_mechanics/test/tests/dynamics/time_integration/mass_scaling.i start=[massmatrix_x] end=[massmatrix_y]


!syntax parameters /Materials/DensityScaling

!syntax inputs /Materials/DensityScaling

!syntax children /Materials/DensityScaling

!bibtex bibliography
