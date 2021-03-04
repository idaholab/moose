# JunctionOneToOne1Phase

!syntax description /Components/JunctionOneToOne1Phase

This component implements a junction that has no volume. It assumes the following
regarding the connected flow channels:

- There are exactly 2 connected flow channels
- The connected flow channels must be parallel

If any of the above conditions are not met, then one must use a volumetric junction
instead ([VolumeJunction1Phase](/VolumeJunction1Phase.md)).

Using this junction between 2 flow channels should be numerically equivalent to having
the 2 connected flow channels merged into 1 large flow channel. This junction is useful
for cases where a separation in a flow channel is required. One particular example is
when a heat structure is connected to a section of an otherwise
single flow channel.

!syntax parameters /Components/JunctionOneToOne1Phase

!syntax inputs /Components/JunctionOneToOne1Phase

!syntax children /Components/JunctionOneToOne1Phase

!bibtex bibliography
