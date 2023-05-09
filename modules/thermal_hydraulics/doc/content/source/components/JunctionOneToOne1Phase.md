# JunctionOneToOne1Phase

This is a [flow junction](component_groups/flow_junction.md) that connects 2
[FlowChannel1Phase.md] components and assumes that there is no loss of momentum,
even if the connected flow channels are not parallel.

Using this junction between 2 flow channels should be numerically equivalent to having
the 2 connected flow channels merged into 1 large flow channel. This junction is useful
for cases where a separation in a flow channel is required. One particular example is
when a heat structure is connected to a section of an otherwise
single flow channel.

## Formulation

This junction is treated just like any interface between two elements in the mesh.

## Usage

!template load file=flow_junction_usage.md.template name=JunctionOneToOne1Phase

!syntax parameters /Components/JunctionOneToOne1Phase

!syntax inputs /Components/JunctionOneToOne1Phase

!syntax children /Components/JunctionOneToOne1Phase
