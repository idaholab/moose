# SolidWall1Phase

This is a single-phase [1-D flow boundary component](component_groups/flow_boundary.md)
corresponding to a solid wall. This component should be used where a flow channel
ends with no inlet or outlet.

## Usage

This component must be connected to a [/FlowChannel1Phase.md]. See
[how to connect a flow boundary component](component_groups/flow_boundary.md#usage).

!syntax parameters /Components/SolidWall1Phase

!syntax inputs /Components/SolidWall1Phase

!syntax children /Components/SolidWall1Phase

## Formulation

This boundary condition uses a [ghost cell formulation](component_groups/flow_boundary.md#ghostcell_flux),
where the ghost cell solution $\mathbf{U}_\text{ghost}$ is the same as the interior
solution but with opposite velocity:

\begin{equation}
  u_\text{ghost} = -u_i \,.
\end{equation}

This produces a zero velocity in the boundary flux computation.
