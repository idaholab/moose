# InletStagnationPressureTemperature1Phase

This is a single-phase [1-D flow boundary component](component_groups/flow_boundary.md)
in which the stagnation pressure and temperature are specified. This boundary is
typically used when fluid is expected to flow from an infinitely large tank where the pressure
and temperature are known.

## Usage

This component must be connected to a [/FlowChannel1Phase.md]. See
[how to connect a flow boundary component](component_groups/flow_boundary.md#usage).

The user specifies the following parameters:

- [!param](/Components/InletStagnationPressureTemperature1Phase/p0): the stagnation pressure, and
- [!param](/Components/InletStagnationPressureTemperature1Phase/T0): the stagnation temperature.

The formulation of this boundary condition assumes flow +entering+ the flow
channel at this boundary.

+Reversible flow+: If +exit+ conditions are encountered,
then the boundary condition is automatically changed to an outlet formulation.
This behavior can be disabled by setting the
[!param](/Components/InletStagnationPressureTemperature1Phase/reversible)
parameter to `false`.

!syntax parameters /Components/InletStagnationPressureTemperature1Phase

!syntax inputs /Components/InletStagnationPressureTemperature1Phase

!syntax children /Components/InletStagnationPressureTemperature1Phase

## Formulation

This boundary condition uses a [ghost cell formulation](component_groups/flow_boundary.md#ghostcell_flux),
where the ghost cell solution $\mathbf{U}_\text{ghost}$ is computed from the following
quantities:

- $p_{0,\text{ext}}$, the provided exterior stagnation pressure,
- $T_{0,\text{ext}}$, the provided exterior stagnation temperature, and
- $u_i$, the interior velocity.

If the boundary is specified to be reversible
([!param](/Components/InletDensityVelocity1Phase/reversible) set to `true`) and
the flow is +exiting+, the ghost cell is instead computed with the following
quantities:

- $p_{\text{ext}}$, the exterior pressure, assumed to be equal to the provided exterior stagnation pressure $p_{0,\text{ext}}$,
- $\rho_i$, the interior density, and
- $u_i$, the interior velocity.
