# Material Time Step Postprocessor

!syntax description /Postprocessors/MaterialTimeStepPostprocessor

## Description

The `MaterialTimeStepPostprocessor` works in conjunction with material models to
compute the appropriate maximum time step allowed by individual material models.
For creep or plasticity models, this maximum time step size is governed by an
allowable inelastic strain increment, but a variety of methods could be used by
individual material models to compute their acceptable time step.

### Creep Strain Example

The maximum time step size is a numerical tool used to ensure the calculation of
physically reasonable inelastic strains and to improve convergence of the inelastic
material model. As an example, the maximum allowable time step computation for
a creep model has the form
\begin{equation}
  \label{eqn:limiting_ts}
  \Delta t |_{limit} = \Delta t \cdot \frac{\Delta \epsilon^{inel}_{max}}{\Delta \epsilon^{inel}}
\end{equation}
where $\Delta \epsilon^{inel}_{max}$ is the maximum effective inelastic strain
increment (a default value of 1e-4), set by the user, and $\Delta \epsilon^{inel}$
is the current scalar effective inelastic strain increment.

### Mesh-Wide Evaluation

The `MaterialTimeStepPostprocessor` collects the time step limitations from all
of the quadrature points in the simulation mesh and stores the minimum value.
This minimum allowable time step size value is then used by the
[IterationAdaptiveDT](/Executioner/TimeStepper/IterationAdaptiveDT.md)
to restrict the time step size based on the limit calculated in the previous
time step.

Note that the [IterationAdaptiveDT](/Executioner/TimeStepper/IterationAdaptiveDT.md)
will apply the limiting time step size value from the `MaterialTimeStepPostprocessor`
only if that value is less than maximum time step size value calculated by the
internal [IterationAdaptiveDT](/Executioner/TimeStepper/IterationAdaptiveDT.md)
adaptive time step size algorithm.

!alert note title=Time Step Limiter Lags Calculation
The value of the maximum allowable time step as collected by the +`MaterialTimeStepPostprocessor`+
is enforced in the next simulation time step.

### Minimum Time Step Calculated in Material Models

The calculation of the maximum allowable timestep is dependent on each individual
material: both the maximum allowable inelastic strain increment and the method of
calculating the current inelastic strain, [eqn:limiting_ts], are defined separately
for each material.
Given this material dependent nature, the maximum time step value is calculated
separately by the [RadialReturnStressUpdate](/Materials/RadialReturnStressUpdate.md)
materials at each quadrature point.
The `MaterialTimeStepPostprocessor` then determines the minimum time step size
value from all of the quadrature points in the simulation.

Initially the value of the maximum time step size is set to `std::numeric_limits<Real>::max()`.
Once the inelastic material model begins to calculate inelastic strain, the value
of the allowable time step size varies with the inelastic strain computation.

## Example Input File

!listing modules/tensor_mechanics/test/tests/material_limit_time_step/elas_plas/nafems_nl1_lim.i block=Postprocessors/matl_ts_min

The name of the `MaterialTimeStepPostprocessor` is passed to the `IterationAdaptiveDT`
as the argument for the `postprocessor_dtlim` parameter

!listing modules/tensor_mechanics/test/tests/material_limit_time_step/elas_plas/nafems_nl1_lim.i block=Executioner/TimeStepper

and the `max_inelastic_increment` parameter in the inelastic material model(s)
must be set to run the time step limit calculation.

!listing modules/tensor_mechanics/test/tests/material_limit_time_step/elas_plas/nafems_nl1_lim.i block=Materials/isoplas

!syntax parameters /Postprocessors/MaterialTimeStepPostprocessor

!syntax inputs /Postprocessors/MaterialTimeStepPostprocessor

!syntax children /Postprocessors/MaterialTimeStepPostprocessor
