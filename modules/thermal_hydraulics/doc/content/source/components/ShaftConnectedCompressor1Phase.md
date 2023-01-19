# ShaftConnectedCompressor1Phase

This component models a compressor that is connected to a [Shaft.md] for the single-phase flow model.
The compressor formulation is based off the model described in [!cite](CompressorR5).
The user supplies performance curves that give the pressure ratio and isentropic
efficiency as functions of shaft speed and mass flow rate. The compressor then
applies a pressure rise and work to the working fluid, according to the current
flow conditions.

## Usage id=usage

!template load file=volume_junction_usage.md.template name=ShaftConnectedCompressor1Phase

!template load file=volume_junction_1phase_usage.md.template name=ShaftConnectedCompressor1Phase

This component must be must be connected to a [Shaft](Shaft.md) component, which
controls the compressor rotational speed. The user must specify which flow
channel boundaries are connected to the compressor with the
[!param](/Components/ShaftConnectedCompressor1Phase/inlet) and
[!param](/Components/ShaftConnectedCompressor1Phase/outlet) parameters.

The user assigns values for several compressor rated conditions:
[!param](/Components/ShaftConnectedCompressor1Phase/omega_rated),
[!param](/Components/ShaftConnectedCompressor1Phase/mdot_rated),
[!param](/Components/ShaftConnectedCompressor1Phase/rho0_rated), and
[!param](/Components/ShaftConnectedCompressor1Phase/c0_rated). These rated
conditions, along with the user-supplied data
[!param](/Components/ShaftConnectedCompressor1Phase/Rp_functions),
[!param](/Components/ShaftConnectedCompressor1Phase/eff_functions),
[!param](/Components/ShaftConnectedCompressor1Phase/speeds), define the
compressor performance model.

The parameter `speeds` is a list of relative corrected speeds $\alpha$ (see
[speed_rel_corr]) in increasing order. The parameters parameters `Rp_functions`
and `eff_functions` define curves for the pressure ratio $r_p$ (see [Rp]) and
isentropic efficiency $\eta$ (see [eff]), respectively, as
[Functions](Functions/index.md) for each entry in `speeds`, where the time value
in the function corresponds to the relative corrected mass flow rate $\nu$ (see
[flow_rel_corr]). Further discussion on these equations and the input data used
in them is found in [#perfdata].

The user inputs values for
[!param](/Components/ShaftConnectedCompressor1Phase/tau_fr_coeff),
[!param](/Components/ShaftConnectedCompressor1Phase/tau_fr_const), and
[!param](/Components/ShaftConnectedCompressor1Phase/speed_cr_fr) that are used
to compute friction torque as defined in [#friction].

The compressor moment of inertia contributes to the total moment of inertia of
the shaft, which represents the resistance to acceleration of the shaft speed.
The user inputs values for
[!param](/Components/ShaftConnectedCompressor1Phase/inertia_coeff),
[!param](/Components/ShaftConnectedCompressor1Phase/inertia_const), and
[!param](/Components/ShaftConnectedCompressor1Phase/speed_cr_I) that are used to
compute the compressor moment of inertia as defined in [#moi].

The parameter [!param](/Components/ShaftConnectedCompressor1Phase/A_ref)
corresponds to the reference area $A_\text{ref}$ (see [momentum]). It
should generally be assigned a value between the inlet area and outlet area.
`A_ref` can act as a means to scale the compressor head and can also be used in
combination with [!param](/Components/ShaftConnectedCompressor1Phase/K) to apply
a pressure drop due to form loss.

!syntax parameters /Components/ShaftConnectedCompressor1Phase

!syntax inputs /Components/ShaftConnectedCompressor1Phase

!syntax children /Components/ShaftConnectedCompressor1Phase

## Formulation

The compressor is modeled as a 0-D volume based on the [VolumeJunction1Phase.md]
component, which has conservation of mass, momentum, and energy equations in the
volume, but with the addition of source terms to the momentum and energy equations:

!equation
\ddt{\rho V} = \dot{m}_\text{in} - \dot{m}_\text{out} \eqc

!equation id=momentum
\ddt{\rho u V}
  = \pr{(\rho u^2 + p) A}_\text{in}
  - \pr{(\rho u^2 + p) A}_\text{out}
  + \Delta p_0 A_\text{ref} \eqc

!equation
\ddt{\rho E V} = \pr{\dot{m} H}_\text{in} - \pr{\dot{m} H}_\text{out} + \dot{W} \eqc

where

- $\Delta p_0$ is the change in stagnation pressure,
- $A_{\text{ref}}$ is the reference cross-sectional area, and
- $\dot{W}$ is the work rate.

The pressure ratio $r_p$, which is given by user-defined performance data (see
[#perfdata]), is defined as the ratio of the outlet stagnation pressure to the
inlet stagnation pressure:

!equation id=Rp
r_p \equiv \frac{p_{\text{0, out}}} {p_{\text{0, in}}} \eqp

The stagnation pressure change can then be computed as

!equation id=delta_p
\Delta p_0 = p_{\text{0, in}} (r_p - 1) \eqp

The isentropic efficiency $\eta$, which is given by user-defined performance
data (see [#perfdata]), is defined as the ratio of the isentropic work rate
$\dot{W}_s$ to the actual work rate $\dot{W}$:

!equation id=eff
\eta \equiv \frac{\dot{W}_s}{\dot{W}} \eqc

!equation
\dot{W}_s = \dot{m} (H_{\text{out},s} - H_\text{in}) \eqc

!equation
\dot{W} = \dot{m} (H_\text{out} - H_\text{in}) = \frac{\dot{W}_s}{\eta} \eqp

The connected [Shaft.md] receives a corresponding torque $\tau_\text{compressor}$,
computed using the work rate and shaft speed $\omega$,
and also a friction torque $\tau_\text{friction}$ (discussed in [#friction]):

!equation
\tau_\text{shaft,compressor} = \tau_\text{compressor} + \tau_\text{friction} \eqc

!equation
\tau_\text{compressor} = -\frac{\dot{W}}{\omega} \eqp

### Compressor performance data id=perfdata

Users specify performance curves that give the pressure ratio $r_p$ and isentropic
efficiency $\eta$ as functions of the mass flow rate and shaft speed. Specifically,
the mass flow rate used is a quantity normalized by rated conditions, often referred to as the
"relative corrected mass flow rate", given the symbol $\nu$:

!equation id=flow_rel_corr
\nu \equiv \frac{ (\frac{\dot{m}}{\rho_{\text{0, in}} c_{\text{0, in}}}) }
  { (\frac{\dot{m}}{\rho_{\text{0, in}} c_{\text{0, in}}})_{\text{rated}} } \eqc

where $\rho_0$ and $c_0$ are stagnation density and sound speed, respectively.
Similarly, the shaft speed used is also normalized by rated conditions, often referred to as
the "relative corrected shaft speed", given the symbol $\alpha$:

!equation id=speed_rel_corr
\alpha \equiv \frac{ (\frac{\omega}{c_{\text{0, in}}}) }
  { (\frac{\omega}{c_{\text{0, in}}})_{\text{rated}} } \eqp

The user provides an ordered list of relative corrected shaft speeds, which
correspond to ordered lists of pressure ratio and isentropic efficiency functions:

!equation
\left\{\alpha_1, \alpha_2, \ldots, \alpha_n\right\} \eqc

!equation
\left\{r_{p,1}(\nu), r_{p,2}(\nu), \ldots, r_{p,n}(\nu)\right\} \eqc

!equation
\left\{\eta_1(\nu), \eta_2(\nu), \ldots, \eta_n(\nu)\right\} \eqc

Linear interpolation and extrapolation is used to get the dependence on $\alpha$
between and outside of the provided $\alpha_i$ values:

!equation
r_p(\nu, \alpha) = r_{p,i}(\nu) + \frac{r_{p,i+1}(\nu) - r_{p,i}(\nu)}{\alpha_{i+1} - \alpha_i}(\alpha - \alpha_i) \eqc

!equation
\eta(\nu, \alpha) = \eta_i(\nu) + \frac{\eta_{i+1}(\nu) - \eta_i(\nu)}{\alpha_{i+1} - \alpha_i}(\alpha - \alpha_i) \eqp

Lastly, bounds are applied to the pressure ratio, since extrapolation outside of
limited performance data may yield unphysical values.

!equation
r_p^- \leq r_p \leq r_p^+ \eqp

### Friction torque id=friction

Friction torque always resists the direction of motion, so the sign of the
compressor friction torque depends on the sign of the connected shaft speed.
Positive shaft speed results in negative friction torque while negative shaft
speed results in positive friction torque.

The friction torque magnitude, $\tau_{\text{friction}}$, is a function of shaft
speed, $\omega$, and four input parameters. If the ratio of shaft speed to
[!param](/Components/ShaftConnectedCompressor1Phase/omega_rated), $\alpha$, is
less than [!param](/Components/ShaftConnectedCompressor1Phase/speed_cr_fr),
friction torque magnitude equals
[!param](/Components/ShaftConnectedCompressor1Phase/tau_fr_const),

!equation
\tau_{\text{friction}} = \tau_{\text{fr,const}},


otherwise, $\tau_{\text{friction}}$ is a function of shaft speed and friction
torque coefficients,
[!param](/Components/ShaftConnectedCompressor1Phase/tau_fr_coeff),

!equation
\tau_{\text{friction}} = \tau_{\text{fr,coeff}}[0] + \tau_{\text{fr,coeff}}[1] \mid \alpha \mid
  + \tau_{\text{fr,coeff}}[2] \mid\alpha \mid^{2} + \tau_{\text{fr,coeff}}[3] \mid \alpha \mid^{3}.

### Moment of inertia id=moi

The compressor moment of inertia, $I_{\text{compressor}}$, is a function of
shaft speed, $\omega$, and four input parameters. If the ratio of shaft speed to
[!param](/Components/ShaftConnectedCompressor1Phase/omega_rated), $\alpha$, is
less than [!param](/Components/ShaftConnectedCompressor1Phase/speed_cr_I),
compressor inertia equals
[!param](/Components/ShaftConnectedCompressor1Phase/inertia_const),

!equation
I_{\text{compressor}} = I_{\text{const}},


otherwise, $I_{\text{compressor}}$ is a function of shaft speed and inertia coefficients, [!param](/Components/ShaftConnectedCompressor1Phase/inertia_coeff),

!equation
I_{\text{compressor}} = I_{\text{coeff}}[0] + I_{\text{coeff}}[1] \mid \alpha \mid + I_{\text{coeff}}[2] \mid\alpha \mid^{2} + I_{\text{coeff}}[3] \mid \alpha \mid^{3}.

### Form Loss

!template load file=volume_junction_1phase_formulation_formloss.md.template name=ShaftConnectedCompressor1Phase

## Output

In addition to the junction variables, this component creates the following post-processors and auxillary scalar variables:

| Post-processor/variable name | Description |Notation |Unit|
| - | :- | - | -|
| `comp_name:delta_p` | Pressure difference across the compressor component  | $\Delta p_0$ | \[Pa\] |
| `comp_name:dissipation_torque` | Dissipation torque  | - | \[Nm\]
| `comp_name:isentropic_torque` | Isentropic torque  | - | \[Nm\]
| `comp_name:friction_torque` | Friction torque | $\tau_{\text{friction}}$ | \[Nm\]
| `comp_name:moment_of_inertia` | Moment of inertia |  $I_{\text{compressor}}$ | \[kg-m^2\]
| `comp_name:efficiency` | Efficiency  | $\eta$ | \[-\]
| `comp_name:pressure_ratio` | Pressure ratio | $r_p$  | \[-\]
| `comp_name:rel_corrected_flow` | Relative corrected mass flow rate | $\alpha$ | \[-\]
| `comp_name:rel_corrected_speed` | Relative corrected shaft speed | $\nu$ | \[-\]

!bibtex bibliography
