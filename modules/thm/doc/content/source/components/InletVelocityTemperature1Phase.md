# InletVelocityTemperature1Phase

This is a single-phase [1-D flow boundary component](component_groups/flow_boundary.md)
in which the velocity and temperature are specified.

## Usage

This component must be connected to a [/FlowChannel1Phase.md]. See
[how to connect a flow boundary component](component_groups/flow_boundary.md#usage).

The user specifies the following parameters:

- [!param](/Components/InletVelocityTemperature1Phase/vel): the velocity, and
- [!param](/Components/InletVelocityTemperature1Phase/T): the temperature.

The formulation of this boundary condition assumes flow +entering+ the flow
channel at this boundary.

+Reversible flow+: If +exit+ conditions are encountered,
then the boundary condition is automatically changed to an outlet formulation.
This behavior can be disabled by setting the [!param](/Components/InletVelocityTemperature1Phase/reversible)
parameter to `false`.

!syntax parameters /Components/InletVelocityTemperature1Phase

!syntax inputs /Components/InletVelocityTemperature1Phase

!syntax children /Components/InletVelocityTemperature1Phase

## Formulation

This boundary condition uses a [ghost cell formulation](component_groups/flow_boundary.md#ghostcell_flux),
where the ghost cell solution $\mathbf{U}_\text{ghost}$ is computed from the following
quantities:

- $u_{\text{ext}}$, the provided exterior velocity,
- $T_{\text{ext}}$, the provided exterior temperature, and
- $p_i$, the interior pressure.

If the boundary is specified to be reversible
([!param](/Components/InletDensityVelocity1Phase/reversible) set to `true`) and
the flow is +exiting+, the ghost cell is instead computed with the following
quantities:

- $u_{\text{ext}}$, the provided exterior velocity,
- $\rho_i$, the interior density, and
- $E_i$, the interior specific total energy.
