# JunctionOneToOne

!syntax description /Components/JunctionOneToOne

This component implements a junction that has no volume. It assumes the following
regarding the connected flow channels:

- There are exactly 2 connected flow channels
- The connected flow channels must have the same cross-sectional area at the junction
- The connected flow channels must be parallel

If any of the above conditions are not met, then one must use either a volumetric junction
instead ([VolumeJunction1Phase](/VolumeJunction1Phase.md) for 1-phase and [VolumeJunction2Phase](/VolumeJunction2Phase.md)
for 2-phase).

Using this junction between 2 flow channels should be numerically equivalent to having
the 2 connected flow channels merged into 1 large flow channel. This junction is useful
for cases where a separation in a flow channel is required. One particular example is
when a [HeatStructure](/HeatStructure.md) is connected to a section of an otherwise
single flow channel.

!syntax parameters /Components/JunctionOneToOne

!syntax inputs /Components/JunctionOneToOne

!syntax children /Components/JunctionOneToOne

!bibtex bibliography
