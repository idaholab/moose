# ShaftConnectedCompressor1Phase

The 1-phase shaft-connected compressor component is modeled as a volume junction with source terms added to the momentum and energy equations due to the compressor's torque and pressure head. The compressor model is based off the model described in [!cite](CompressorR5).

The compressor model relies on input parameters, upstream flow conditions, and rotational speed to calculate the compressor pressure head, isentropic torque, dissipation torque, friction torque, and moment of inertia. Compressor head and net torque (sum of isentropic, dissipation, and friction torques) are used in source terms to affect flow conditions coming out of the compressor. Net torque and moment of inertia are sent to the connected `Shaft` component and affect the rotational speed.

## Usage

A `ShaftConnectedCompressor1Phase` component must be connected to a [Shaft](Shaft.md) component, which controls the compressor rotational speed. The user must specify which flow channel boundaries are connected to the compressor with the [!param](/Components/ShaftConnectedCompressor1Phase/inlet) and [!param](/Components/ShaftConnectedCompressor1Phase/outlet) parameters.

[!param](/Components/ShaftConnectedCompressor1Phase/A_ref) should generally be assigned a value in between the [!param](/Components/FlowChannel1Phase/A) values of the connected `inlet` or `outlet`. `A_ref` can act as a means to scale the compressor head. `A_ref` can also be used in combination with [!param](/Components/ShaftConnectedCompressor1Phase/K) to apply a pressure drop due to form loss. The compressor volume, [!param](/Components/ShaftConnectedCompressor1Phase/volume), should be a reasonable estimate of the inlet to outlet fluid volume of the modeled compressor.

The user assigns values for several compressor rated conditions: [!param](/Components/ShaftConnectedCompressor1Phase/omega_rated), [!param](/Components/ShaftConnectedCompressor1Phase/mdot_rated), [!param](/Components/ShaftConnectedCompressor1Phase/rho0_rated), and [!param](/Components/ShaftConnectedCompressor1Phase/c0_rated). These rated conditions, along with the user-supplied data [!param](/Components/ShaftConnectedCompressor1Phase/Rp_functions), [!param](/Components/ShaftConnectedCompressor1Phase/eff_functions), [!param](/Components/ShaftConnectedCompressor1Phase/speeds), define the compressor performance model.

The inputs `Rp_functions` and `eff_functions` should be vectors of function names; each function should be defined for a constant relative corrected speed, $\alpha$, [speed_rel_corr]. The length and order of `speeds` need to correspond to the length and order of `Rp_functions` and `eff_functions`. These functions describe $Rp$, [Rp], and $\eta_{\text{ad}}$, [eff], versus $\nu$, [flow_rel_corr]. Further discussion on these equations and the input data used in them is found in [#data].

The user inputs values for [!param](/Components/ShaftConnectedCompressor1Phase/tau_fr_coeff), [!param](/Components/ShaftConnectedCompressor1Phase/tau_fr_const), and [!param](/Components/ShaftConnectedCompressor1Phase/speed_cr_fr) that are used to compute friction torque as defined in [#friction].

The compressor moment of inertia affects how the compressor accelerates. The user inputs values for [!param](/Components/ShaftConnectedCompressor1Phase/inertia_coeff), [!param](/Components/ShaftConnectedCompressor1Phase/inertia_const), and [!param](/Components/ShaftConnectedCompressor1Phase/speed_cr_I) that are used to compute the compressor moment of inertia as defined in [#moi].

!syntax parameters /Components/ShaftConnectedCompressor1Phase

!syntax inputs /Components/ShaftConnectedCompressor1Phase

!syntax children /Components/ShaftConnectedCompressor1Phase

## Formulation

The conservation of mass, momentum, and energy equations for the compressor volume are similar to the equations used by [VolumeJunction1Phase](VolumeJunction1Phase.md) but add the compressor momentum and energy source terms,

!equation id=momentum_source
S^{\text{momentum}} = \Delta p A_{\text{ref}} \cdot \hat{n}_{\text{out}},

and

!equation id=energy_source
S^{\text{energy}} = -(\tau_{\text{isentropic}} + \tau_{\text{dissipation}}) \omega,

where

- $\Delta p$ is the change in pressure due to compressor head,
- $A_{\text{ref}}$ is the cross-sectional area of the compressor,
- $\hat{n}_{out}$ is the orientation of the flow channel connected to the compressor outlet,
- $\tau_{\text{isentropic}}$ is the isentropic torque,
- $\tau_{\text{dissipation}}$ is the dissipation torque, and
- $\omega$ is the shaft speed.

The connected [Shaft.md] receives not only the isentropic and dissipation torque,
but also a friction torque $\tau_\text{friction}$:

!equation
\tau_\text{shaft,compressor} = \tau_\text{isentropic} + \tau_\text{dissipation} + \tau_\text{friction} \eqp

#### Compressor performance data id=data

Values are obtained by means of an empirical compressor performance map. The independent variables are based on incoming mass flow rate and shaft angular velocity. The dependent variables are pressure ratio, $Rp$, and adiabatic efficiency, $\eta_{\text{ad}}$. The mass flow rate and angular velocity tables should be entered as relative corrected values. The relative corrected mass flow rate, $\nu$, is computed as,

!equation id=flow_rel_corr
\nu = \frac{ (\frac{\dot{m}}{\rho_{\text{0, in}} a_{\text{0, in}}}) } { (\frac{\dot{m}}{\rho_{\text{0, in}} a_{\text{0, in}}})_{\text{rated}} }.

The relative corrected speed, $\alpha$, is computed as,

!equation id=speed_rel_corr
\alpha = \frac{ (\frac{\omega}{a_{\text{0, in}}}) } { (\frac{\omega}{a_{\text{0, in}}})_{\text{rated}} }.

Pressure ratio, $Rp$, is defined as the ratio of outlet stagnation pressure over inlet stagnation pressure; it is a function of $\nu$ and $\alpha$.

!equation id=Rp
Rp = Rp(\nu, \alpha) = \frac{p_{\text{0, out}}} {p_{\text{0, in}}}

Adiabatic efficiency, $\eta_{\text{ad}}$, is defined as the ratio of isentropic work over actual work; it is a function of $\nu$ and $\alpha$.

!equation id=eff
\eta_{\text{ad}} = \eta_{\text{ad}}(\nu, \alpha) = \frac{\text{isentropic work}}{\text{actual work}} = \frac{h_{\text{0, out}}^{*} - h_{\text{0, in}}} {h_{\text{0, out}} - h_{\text{0, in}}}

The user should enter functions of $Rp$ and $\eta_{\text{ad}}$ versus $\nu$ for a range of constant speeds, $\alpha = \text{const}$. These functions are typically entered as data tables. The model linearly interpolates using the constant speed data tables to compute $Rp$ and $\eta_{\text{ad}}$ for the actual current compressor shaft speed.

Pressure head, isentropic torque, dissipation torque are calculated using the input compressor performance data, fluid properties, and the three equations below.

!equation id=delta_p
\Delta p = p_{\text{0, in}} * (Rp - 1.0)

!equation id=tau_isen
\tau_{\text{isentropic}} = \frac{\dot{m}}{\omega} (h_{\text{0, out}}^{*} - h_{\text{0, in}})

!equation id=tau_diss
\tau_{\text{dissipation}} = \frac{\dot{m}}{\omega} (h_{\text{0, out}} - h_{\text{0, out}}^{*})

### Friction torque id=friction

Friction torque always resists the direction of motion, so the sign of the compressor friction torque depends on the sign of the connected shaft speed. Positive shaft speed results in negative friction torque while negative shaft speed results in positive friction torque.

The friction torque magnitude, $\tau_{\text{friction}}$, is a function of shaft speed, $\omega$, and four input parameters. If the ratio of shaft speed to [!param](/Components/ShaftConnectedCompressor1Phase/omega_rated), $\alpha$, is less than [!param](/Components/ShaftConnectedCompressor1Phase/speed_cr_fr), friction torque magnitude equals [!param](/Components/ShaftConnectedCompressor1Phase/tau_fr_const),

!equation
\tau_{\text{friction}} = \tau_{\text{fr,const}},


otherwise, $\tau_{\text{friction}}$ is a function of shaft speed and friction torque coefficients, [!param](/Components/ShaftConnectedCompressor1Phase/tau_fr_coeff),

!equation
\tau_{\text{friction}} = \tau_{\text{fr,coeff}}[0] + \tau_{\text{fr,coeff}}[1] \mid \alpha \mid + \tau_{\text{fr,coeff}}[2] \mid\alpha \mid^{2} + \tau_{\text{fr,coeff}}[3] \mid \alpha \mid^{3}.


### Moment of inertia id=moi

The compressor moment of inertia, $I_{\text{compressor}}$, is a function of shaft speed, $\omega$, and four input parameters. If the ratio of shaft speed to [!param](/Components/ShaftConnectedCompressor1Phase/omega_rated), $\alpha$, is less than [!param](/Components/ShaftConnectedCompressor1Phase/speed_cr_I), compressor inertia equals [!param](/Components/ShaftConnectedCompressor1Phase/inertia_const),

!equation
I_{\text{compressor}} = I_{\text{const}},


otherwise, $I_{\text{compressor}}$ is a function of shaft speed and inertia coefficients, [!param](/Components/ShaftConnectedCompressor1Phase/inertia_coeff),

!equation
I_{\text{compressor}} = I_{\text{coeff}}[0] + I_{\text{coeff}}[1] \mid \alpha \mid + I_{\text{coeff}}[2] \mid\alpha \mid^{2} + I_{\text{coeff}}[3] \mid \alpha \mid^{3}.


!bibtex bibliography
