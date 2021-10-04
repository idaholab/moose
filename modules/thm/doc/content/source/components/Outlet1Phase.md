# Outlet1Phase

This is a single-phase [1-D flow boundary component](component_groups/flow_boundary.md)
in which the pressure is specified. This boundary is typically used when the
boundary is anticipated to be an outlet.

## Usage

This component must be connected to a [/FlowChannel1Phase.md]. See
[how to connect a flow boundary component](component_groups/flow_boundary.md#usage).

The user specifies the following parameters:

- [!param](/Components/Outlet1Phase/p): the pressure.

The formulation of this boundary condition assumes flow +exiting+ the flow
channel at this boundary.

!syntax parameters /Components/Outlet1Phase

!syntax inputs /Components/Outlet1Phase

!syntax children /Components/Outlet1Phase

## Formulation

This boundary condition uses a [ghost cell formulation](component_groups/flow_boundary.md#ghostcell_flux),
where the ghost cell solution $\mathbf{U}_\text{ghost}$ is computed from the following
quantities:

- $p_{\text{ext}}$, the provided exterior pressure,
- $\rho_i$, the interior density, and
- $u_i$, the interior velocity.
