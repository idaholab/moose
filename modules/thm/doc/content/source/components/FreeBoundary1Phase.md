# FreeBoundary1Phase

This is a single-phase [1-D flow boundary component](component_groups/flow_boundary.md)
in which no boundary data is supplied by the user. This component should be used
only if the boundary is a supersonic exit, since this is the only situation in
which no external characteristics enter the domain.

## Usage

This component must be connected to a [/FlowChannel1Phase.md]. See
[how to connect a flow boundary component](component_groups/flow_boundary.md#usage).

!syntax parameters /Components/FreeBoundary1Phase

!syntax inputs /Components/FreeBoundary1Phase

!syntax children /Components/FreeBoundary1Phase

## Formulation

In this formulation, no boundary data is supplied; thus, the boundary flux is evaluated
entirely using the interior solution:

\begin{equation}
  \mathbf{F}_\text{b} = \mathcal{f}(\mathbf{U}_i) \eqp
\end{equation}
