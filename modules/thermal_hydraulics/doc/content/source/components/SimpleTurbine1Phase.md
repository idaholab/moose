# SimpleTurbine1Phase

The 1-phase simple turbine component is modeled as a volume junction
with source terms added to the momentum and energy equations due to the energy
extraction and pressure drop.


## Usage

A `SimpleTurbine1Phase` component must be connected to two [FlowChannel1Phase](FlowChannel1Phase.md)
boundaries. The first boundary specified in  [!param](/Components/SimpleTurbine1Phase/connections)
is assumed to be the inlet of the turbine and the second one is the outlet. The connected flow channels
must have the same direction.


The user specifies  the power to be extracted by the turbine using the parameter [!param](/Components/SimpleTurbine1Phase/power).
Power will be extracted if the parameter [!param](/Components/SimpleTurbine1Phase/on)  is set to true.
Both parameters are controllable.

!syntax parameters /Components/SimpleTurbine1Phase

!syntax inputs /Components/SimpleTurbine1Phase

!syntax children /Components/SimpleTurbine1Phase

## Formulation

The conservation of mass, momentum, and energy equations for the turbine volume
are similar to the equations used by
[JunctionParallelChannels1Phase](JunctionParallelChannels1Phase.md) but add the turbine momentum and
energy source terms,

!equation id=momentum_source
S^{\text{momentum}} = -\Delta p A ~ \hat{n}_{\text{out}},

and

!equation id=energy_source
S^{\text{energy}} = - \dot{Q} ,

where

- $\Delta p$ is the turbine pressure drop,
- $A$ is the cross-sectional area of the inlet flow channel,
- $\hat{n}_{out}$ is the orientation of the flow channel connected to the turbine outlet, and
- $\dot{Q}$ is the power extracted.

Assuming that the process inside the turbine is isentropic, the pressure drop is

!equation id=pressure_drop
\Delta p = p_{\text{in}} \left( 1 - \left(1 - \frac{\dot{Q}}{\dot{m}h} \right)^{\frac{\gamma}{\gamma-1}}\right) ,

 where

- $p_{\text{in}}$ is the pressure at the inlet of the turbine,
- $\dot{m}$ is the mass flow rate entering the turbine,
- $h$ in the specific enthalpy at the inlet of the turbine, and
- $\gamma$ is the heat capacity ratio.
