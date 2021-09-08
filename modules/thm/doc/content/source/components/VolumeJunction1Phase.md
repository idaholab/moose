# VolumeJunction1Phase

!syntax description /Components/VolumeJunction1Phase

This component implements a junction model for 1-phase flow that has a volume.
A form loss coefficient
[!param](/Components/VolumeJunction1Phase/K) can be supplied by the user. The source term on the momentum equation is:

!equation id=momentum_source
S^{\text{momentum}} = - K (p_0 - p) A  \hat{n}_1,


where

- $p_0$ is the stagnation pressure of the first [flow channel](FlowChannel1Phase.md) connected to the junction in [!param](/Components/VolumeJunction1Phase/connections),
- $p$ is the pressure of the first [flow channel](FlowChannel1Phase.md) connected to the junction,
- A is the flow area of the first [flow channel](FlowChannel1Phase.md) connected to the junction or [!param](/Components/VolumeJunction1Phase/A_ref) if it was supplied by the user, and
- $\hat{n}_1$ is the direction of the flow in the first [flow channel](FlowChannel1Phase.md) connected to the junction.

The source term on the energy equation is

!equation id=momentum_energy
S^{\text{momentum}} = - K (p_0 - p) A |u|,

where $u$ is the velocity in the first connected flow channel.

!syntax parameters /Components/VolumeJunction1Phase

!syntax inputs /Components/VolumeJunction1Phase

!syntax children /Components/VolumeJunction1Phase

!bibtex bibliography
