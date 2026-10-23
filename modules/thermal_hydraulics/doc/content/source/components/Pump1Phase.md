# Pump1Phase

!syntax description /Components/Pump1Phase

This component implements a pump model for 1-phase flow that has a volume,
which is directly supplied by the user. This component is heavily based off of
VolumeJunction1Phase. The main difference is that there is only one momentum
conservation equation (the ones for rhovV and rhowV were removed) since the pump
inlet and outlet are aligned in the rhouV direction. Also there are new pump
head terms added to the momentum and energy equations.

## Formulation

The conservation of mass, momentum, and energy equations for the pump volume are similar to the equations used by [VolumeJunction1Phase](VolumeJunction1Phase.md) but add the pump momentum and energy source terms,

!equation id=momentum_source
S^{\text{momentum}} = \rho \left \| \vec{g} \right \| H A_{\text{ref}} \hat{d},

and

!equation id=energy_source
S^{\text{energy}} = \frac{1}{2} \left \| \vec{g} \right \| H \left( \dot{m}_1 + \dot{m}_2 \right),

where

- $\rho$ is the density of the fluid in the pump volume,
- $\left \| \vec{g} \right \|$ is the magnitude of the gravity vector,
- $H$ is the pump head,
- $A_{\text{ref}}$ is the reference cross-sectional area of the pump,
- $\hat{d}$ is the direction of the flow channel connected to the pump inlet, and
- $\dot{m}_1$ and $\dot{m}_2$ are the mass flow rates at the two connections, along $\hat{d}$.

The energy source is the work of an ideal pump, $\dot{m} \left \| \vec{g} \right \| H$, at steady state. It is computed from the mass flow rates at the connections rather than from the velocity of the pump volume: the pressure rise inside the volume raises the volume velocity above the velocity of the flow through the pump, by approximately $\left \| \vec{g} \right \| H / (2 c)$, where $c$ is the sound speed.

The momentum source produces a steady pressure rise of $\rho \left \| \vec{g} \right \| H$ across the pump when $A_{\text{ref}}$ is equal to the areas of the connected flow channels. When the areas differ, the pressure rise differs from $\rho \left \| \vec{g} \right \| H$.

!syntax parameters /Components/Pump1Phase

!syntax inputs /Components/Pump1Phase

!syntax children /Components/Pump1Phase

!bibtex bibliography
