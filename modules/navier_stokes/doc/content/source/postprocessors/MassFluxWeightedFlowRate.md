# MassFluxWeightedFlowRate

!syntax description /Postprocessors/MassFluxWeightedFlowRate

## Explanation

This postprocessor computes:

!equation
\frac{\int_{\partial \Omega} \vec{v} \cdot \vec{n} \rho f ds }
{\int_{\partial \Omega} \vec{v} \cdot \vec{n} \rho ds},

where $f$ is the advected quantity, $\partial \Omega$ is the boundary
we integrate over, $\vec{v}$ is the fluid velocity, $\rho$ is density,
and $\vec{n}$ is the face normal.

## Example input syntax

In this example, the mass-flow rate weighted average of temperature
is computed over the right outflow and one interior face. This temperature
should be $500$ K above the inlet temperature.

The `mfr-weighted-T-out` and `mfr-weighted-T-interior` satisfy this condition
when the problem is run to steady-state. However `outlet-temp` does not, because
it is a simple side average and does not take into account differences of
$\rho \vec{v}$ over the face.

!listing test/tests/postprocessors/flow_rates/mass_flux_weighted_pp.i block=Postprocessors

!syntax parameters /Postprocessors/MassFluxWeightedFlowRate

!syntax inputs /Postprocessors/MassFluxWeightedFlowRate

!syntax children /Postprocessors/MassFluxWeightedFlowRate
