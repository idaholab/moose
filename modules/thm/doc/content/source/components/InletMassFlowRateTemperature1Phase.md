# InletMassFlowRateTemperature1Phase

This is a single-phase [1-D flow boundary component](component_groups/flow_boundary.md)
in which the mass flow rate and temperature are specified. This boundary is
typically used when fluid is expected to flow from an infinitely large tank where the pressure
and temperature are known.

## Usage

This component must be connected to a [/FlowChannel1Phase.md]. See
[how to connect a flow boundary component](component_groups/flow_boundary.md#usage).

The user specifies the following parameters:

- [!param](/Components/InletMassFlowRateTemperature1Phase/m_dot): the mass flow rate, and
- [!param](/Components/InletMassFlowRateTemperature1Phase/T): the temperature.

The formulation of this boundary condition assumes flow +entering+ the flow
channel at this boundary.

+Reversible flow+: If +exit+ conditions are encountered,
then the boundary condition is automatically changed to an outlet formulation.
This behavior can be disabled by setting the
[!param](/Components/InletMassFlowRateTemperature1Phase/reversible)
parameter to `false`.

!syntax parameters /Components/InletMassFlowRateTemperature1Phase

!syntax inputs /Components/InletMassFlowRateTemperature1Phase

!syntax children /Components/InletMassFlowRateTemperature1Phase

## Formulation

This boundary condition uses a [ghost cell formulation](component_groups/flow_boundary.md#ghostcell_flux),
where the ghost cell solution $\mathbf{U}_\text{ghost}$ is computed from the following
quantities:

- $\dot{m}_\text{ext}$, the provided exterior mass flow rate,
- $T_\text{ext}$, the provided exterior temperature, and
- $p_i$, the interior pressure.

If the boundary is specified to be reversible
([!param](/Components/InletDensityVelocity1Phase/reversible) set to `true`) and
the flow is +exiting+, the ghost cell is instead computed with the following
quantities:

- $\dot{m}_\text{ext}$, the provided exterior mass flow rate,
- $\rho_i$, the interior density, and
- $E_i$, the interior specific total energy.
