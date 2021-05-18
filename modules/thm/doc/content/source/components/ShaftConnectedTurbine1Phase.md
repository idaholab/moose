# ShaftConnectedTurbine1Phase

The 1-phase shaft-connected turbine component is modeled as a volume junction
with source terms added to the momentum and energy equations due to the
turbine's torque and pressure drop.

The turbine model relies on input parameters, upstream flow conditions, and
rotational speed to calculate the turbine pressure drop, driving torque,
friction torque, power, and moment of inertia. Turbine pressure drop and net
torque (sum of driving and friction torques) are used in source terms to affect
flow conditions coming out of the turbine. Net torque and moment of inertia are
sent to the connected `Shaft` component and affect the rotational speed.

## Usage

A `ShaftConnectedTurbine1Phase` component must be connected to a
[Shaft](Shaft.md) component, which controls the turbine rotational speed. The
user must specify which flow channel boundaries are connected to the turbine
with the [!param](/Components/ShaftConnectedTurbine1Phase/inlet) and
[!param](/Components/ShaftConnectedTurbine1Phase/outlet) parameters.

[!param](/Components/ShaftConnectedTurbine1Phase/A_ref) should generally be
assigned a value in between the [!param](/Components/FlowChannel1Phase/A) values
of the connected `inlet` or `outlet`. `A_ref` can act as a means to scale the
turbine pressure drop. `A_ref` can also be used in combination with
[!param](/Components/ShaftConnectedTurbine1Phase/K) to apply an additional
pressure drop due to form loss. The turbine volume,
[!param](/Components/ShaftConnectedTurbine1Phase/volume), should be a reasonable
estimate of the inlet to outlet fluid volume of the modeled turbine.

The user assigns values for the rated turbine speed,
[!param](/Components/ShaftConnectedTurbine1Phase/omega_rated), and the turbine
wheel diameter, [!param](/Components/ShaftConnectedTurbine1Phase/D_wheel).
`D_wheel` can be any physical dimension of the turbine as a measure of the
turbine's size, usually the rotor diameter. These rated values, along with the
user-supplied functions
[!param](/Components/ShaftConnectedTurbine1Phase/head_coefficient) and
[!param](/Components/ShaftConnectedTurbine1Phase/power_coefficient), define the
turbine performance model. Further discussion on these parameters, their
defining equations, and the input data used in them is found in [#data].

The user inputs values for
[!param](/Components/ShaftConnectedTurbine1Phase/tau_fr_coeff),
[!param](/Components/ShaftConnectedTurbine1Phase/tau_fr_const), and
[!param](/Components/ShaftConnectedTurbine1Phase/speed_cr_fr) that are used to
compute friction torque as defined in [#friction].

The turbine moment of inertia affects how the turbine accelerates. The user
inputs values for
[!param](/Components/ShaftConnectedTurbine1Phase/inertia_coeff),
[!param](/Components/ShaftConnectedTurbine1Phase/inertia_const), and
[!param](/Components/ShaftConnectedTurbine1Phase/speed_cr_I) that are used to
compute the turbine moment of inertia as defined in [#moi].

!syntax parameters /Components/ShaftConnectedTurbine1Phase

!syntax inputs /Components/ShaftConnectedTurbine1Phase

!syntax children /Components/ShaftConnectedTurbine1Phase

## Formulation

The conservation of mass, momentum, and energy equations for the turbine volume
are similar to the equations used by
[VolumeJunction1Phase](VolumeJunction1Phase.md) but add the turbine momentum and
energy source terms,

!equation id=momentum_source
S^{\text{momentum}} = -\Delta p A_{\text{ref}} \cdot \hat{n}_{\text{out}},

and

!equation id=energy_source
S^{\text{energy}} = -(\tau_{\text{driving}} + \tau_{\text{friction}}) \omega,

where

- $\Delta p$ is the pressure drop from turbine inlet to outlet,
- $A_{\text{ref}}$ is the cross-sectional area of the turbine,
- $\hat{n}_{out}$ is the orientation of the flow channel connected to the turbine outlet,
- $\tau_{\text{driving}}$ is the driving torque,
- $\tau_{\text{friction}}$ is the friction torque, and
- $\omega$ is the shaft speed.

#### Turbine performance data id=data

The turbine model is based on dimensional analysis using Buckingham's Pi Theorem
[!cite](TurbineLecture). "The Affinity Laws were derived using the dimensional
analysis methods developed by Buckingham to reduce the complex performance map
into distinct dimensionless curves which are more useful output parameters. For
a single-phase gas turbine, the turbine's performance parameters are
non-dimensionalized to evaluate the effects of flow rate (represented by the
flow coefficient, $\Phi$), fluid density ($\rho$), rotational speed ($\omega$)
and turbine wheel diameter ($D_{\text{wheel}}$) on the power input represented
by the head coefficient ($\Psi$) and on useful turbine work represented by the
power output coefficient ($\Pi$) [!cite](TurbineCharacterization)."

Values are obtained by means of an empirical turbine performance map. The
independent variables are flow coefficient ($\Phi$) and shaft speed ($\omega$).
The dependent variables are the head coefficient ($\Psi$) and power coefficient
($\Pi$).

The flow coefficient is computed as

!equation id=flow_coeff
\Phi = \frac{ Q_{\text{in}} } { \omega D_{\text{wheel}}^{3} },

where $Q_{\text{in}}$ is volumetric flow rate at the turbine inlet.

The head coefficient is computed as

!equation id=head_coeff
\Psi = \frac{ g H } { D_{\text{wheel}}^{2} \omega^{2} }.

The power coefficient is computed as

!equation id=power_coeff
\Pi = \frac{ \tau_{\text{driving}} } { \rho_{\text{turbine}} \omega^{2}
D_{\text{wheel}}^{5} },

where $\rho_{\text{turbine}}$ is the fluid density within the turbine volume.

Pressure drop ($\Delta p$) and driving torque ($\tau_{\text{driving}}$) are
calculated using the input turbine performance data, dimensional coefficients
defined above, and the two equations below.

!equation id=delta_p
\Delta p = \rho_{\text{turbine}} g H

!equation id=tau_driving
\tau_{\text{driving}} = \Pi \rho_{\text{turbine}} \omega^{2} D_{\text{wheel}}^{5}


### Friction torque id=friction

Friction torque always resists the direction of motion, so the sign of the
turbine friction torque depends on the sign of the connected shaft speed.
Positive shaft speed results in negative friction torque while negative shaft
speed results in positive friction torque.

The friction torque magnitude, $\tau_{\text{friction}}$, is a function of shaft
speed, $\omega$, and four input parameters. If the ratio of shaft speed to
[!param](/Components/ShaftConnectedTurbine1Phase/omega_rated), $\alpha$, is less
than [!param](/Components/ShaftConnectedTurbine1Phase/speed_cr_fr), friction
torque magnitude equals
[!param](/Components/ShaftConnectedTurbine1Phase/tau_fr_const),

!equation
\tau_{\text{friction}} = \tau_{\text{fr,const}},


otherwise, $\tau_{\text{friction}}$ is a function of shaft speed and friction
torque coefficients,
[!param](/Components/ShaftConnectedTurbine1Phase/tau_fr_coeff),

!equation
\tau_{\text{friction}} = \tau_{\text{fr,coeff}}[0] + \tau_{\text{fr,coeff}}[1]
\mid \alpha \mid + \tau_{\text{fr,coeff}}[2] \mid\alpha \mid^{2} +
\tau_{\text{fr,coeff}}[3] \mid \alpha \mid^{3}.


### Moment of inertia id=moi

The turbine moment of inertia, $I_{\text{turbine}}$, is a function of shaft
speed, $\omega$, and four input parameters. If the ratio of shaft speed to
[!param](/Components/ShaftConnectedTurbine1Phase/omega_rated), $\alpha$, is less
than [!param](/Components/ShaftConnectedTurbine1Phase/speed_cr_I), turbine
inertia equals [!param](/Components/ShaftConnectedTurbine1Phase/inertia_const),

!equation
I_{\text{turbine}} = I_{\text{const}},

otherwise, $I_{\text{turbine}}$ is a function of shaft speed and inertia
coefficients, [!param](/Components/ShaftConnectedTurbine1Phase/inertia_coeff),

!equation
I_{\text{turbine}} = I_{\text{coeff}}[0] + I_{\text{coeff}}[1] \mid \alpha \mid
+ I_{\text{coeff}}[2] \mid\alpha \mid^{2} + I_{\text{coeff}}[3] \mid \alpha
\mid^{3}.


!bibtex bibliography
