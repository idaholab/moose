# ShaftConnectedPump1Phase

The 1-phase shaft-connected pump component is modeled as a volume junction with source terms added to the momentum and energy equations due to the pump's head and torque.

The pump model relies on input parameters, flow conditions, and rotational speed to calculate the pump head, hydraulic torque, friction torque, and moment of inertia. Pump head and hydraulic torque are used in source terms to affect flow conditions coming out of the pump. Net torque (sum of hydraulic and friction torques) and moment of inertia are sent to the connected `Shaft` component and affect the rotational speed.

## Usage

A `ShaftConnectedPump1Phase` component must be connected to a [Shaft](Shaft.md) component, which controls the pump rotational speed. The user must specify which flow channel boundaries are connected to the pump with the [!param](/Components/ShaftConnectedPump1Phase/inlet) and [!param](/Components/ShaftConnectedPump1Phase/outlet) parameters.

[!param](/Components/ShaftConnectedPump1Phase/A_ref) should generally be assigned a value in between the [!param](/Components/FlowChannel1Phase/A) values of the connected `inlet` or `outlet`. `A_ref` can act as a means to scale the pump head. `A_ref` can also be used in combination with [!param](/Components/ShaftConnectedPump1Phase/K) to apply a pressure drop due to form loss.  The pump volume, [!param](/Components/ShaftConnectedPump1Phase/volume), should be a reasonable estimate of the inlet to outlet fluid volume of the modeled pump.

The user assigns values for several pump rated conditions: [!param](/Components/ShaftConnectedPump1Phase/omega_rated), [!param](/Components/ShaftConnectedPump1Phase/volumetric_rated), [!param](/Components/ShaftConnectedPump1Phase/head_rated), and [!param](/Components/ShaftConnectedPump1Phase/torque_rated). These rated conditions, along with the user-supplied functions [!param](/Components/ShaftConnectedPump1Phase/head) and [!param](/Components/ShaftConnectedPump1Phase/torque_hydraulic), define the pump performance model. The inputs `head` and `torque` should be functions with independent and dependent variables as defined in [#polar]. Pump manufacturers often supply performance data in eight XY sets according to conventional homologous pump variables, as defined in [conventional_curve]. The relationship between the conventional homologous variables ($X$, $Y_{H}$, and $Y_{T}$) and the polar homologous variables ($\theta$, $W_{H}$, and $W_{T}$) is shown in [relationship_head] and [relationship_torque]. Variables used in these equations are defined in [#headntorque].

!equation id=relationship_head
W_{H} = \frac{h}{\alpha^{2} + \nu^{2}} = \frac{Y_{H}}{1 + X^{2}}

!equation id=relationship_torque
W_{T} = \frac{\beta}{\alpha^{2} + \nu^{2}} = \frac{Y_{T}}{1 + X^{2}}

The friction torque determines the pump efficiency. The user inputs values for [!param](/Components/ShaftConnectedPump1Phase/tau_fr_coeff), [!param](/Components/ShaftConnectedPump1Phase/tau_fr_const), and [!param](/Components/ShaftConnectedPump1Phase/speed_cr_fr) that are used to compute friction torque as defined in [#friction].

The pump moment of inertia affects how the pump accelerates. The user inputs values for [!param](/Components/ShaftConnectedPump1Phase/inertia_coeff), [!param](/Components/ShaftConnectedPump1Phase/inertia_const), and [!param](/Components/ShaftConnectedPump1Phase/speed_cr_I) that are used to compute the pump moment of inertia as defined in [#moi].

!syntax parameters /Components/ShaftConnectedPump1Phase

!syntax inputs /Components/ShaftConnectedPump1Phase

!syntax children /Components/ShaftConnectedPump1Phase

## Formulation

The conservation of mass, momentum, and energy equations for the pump volume are similar to the equations used by [VolumeJunction1Phase](VolumeJunction1Phase.md) but add the pump momentum and energy source terms,

!equation id=momentum_source
S^{\text{momentum}} = \rho_{\text{in}} \left \| \vec{g} \right \| H A_{\text{ref}} \cdot \hat{n}_{out},

and

!equation id=energy_source
S^{\text{energy}} = - \tau_{\text{hydraulic}} \omega,


where

- $\rho_{\text{in}}$ is the density of the fluid at the pump inlet,
- $\left \| \vec{g} \right \|$ is the magnitude of the gravity vector,
- $H$ is the pump head,
- $A_{\text{ref}}$ is the cross-sectional area of the pump,
- $\hat{n}_{out}$ is the orientation of the flow channel connected to the pump outlet,
- $\tau_{\text{hydraulic}}$ is the hydraulic torque, and
- $\omega$ is the shaft speed.

### Head and hydraulic torque id=headntorque

$H$ and $\tau_{\text{hydraulic}}$, are defined by means of an empirical homologous pump performance model [!cite](CentrifugalPump). Pump performance data must be generated experimentally. The basic parameters that characterize the pump performance are the rotational speed, $\omega$, the volumetric flow rate, $Q$, the head rise, $H$, and the hydraulic torque, $\tau_{\text{hydraulic}}$. Non-dimensional homologous curves which relate experimental data for these basic pump parameters are used to condense the input. The following rated  parameters are used to normalize the pump operation:

| Quantity | Input file parameter |
| - | - |
| $\omega_{R}$ | [!param](/Components/ShaftConnectedPump1Phase/omega_rated) |
| $Q_{R}$ | [!param](/Components/ShaftConnectedPump1Phase/volumetric_rated) |
| $H_{R}$ | [!param](/Components/ShaftConnectedPump1Phase/head_rated) |
| $\tau_{R}$ | [!param](/Components/ShaftConnectedPump1Phase/torque_rated) |
| $\rho_{R}$ | [!param](/Components/ShaftConnectedPump1Phase/density_rated) |

Parameters for the homologous pump curves are defined as:

- speed ratio, $\alpha = \frac{\omega}{\omega_{R}}$,
- flow ratio, $\nu=\frac{Q}{Q_{R}}$,
- head ratio, $h=\frac{H}{H_{R}}$, and
- torque ratio, $\beta =\frac{\tau_{\text{hydraulic}}}{\tau_{R}}$.

The pump performance is first divided into four quadrants, as seen in [mode].

!table id=mode caption=Pump operating modes.
| Non-dimensional Speed/Capacity | Mode Identifier |
| - | - |
| $\alpha >= 0$ and $\nu >= 0$ | Normal (N) |
| $\alpha > 0$ and $\nu < 0$ | Dissipation (D) |
| $\alpha <= 0$ and $\nu <= 0$ | Turbine (T) |
| $\alpha < 0$ and $\nu > 0$ | Reversal (R) |

Then the four quadrants are further divided into the following two ranges:

- A range, $|\nu / \alpha| <= 1$, and
- V range, $|\alpha / \nu| < 1$.

#### Conventional homologous pump data

Pump performance is typically characterized using conventional homologous pump data pairs. The independent ($X$) and dependent ($Y_{H}$, $Y_{T}$) variables for conventional homologous pump curves are defined in [conventional_curve].  This table spans four pump operating modes and two ranges, for a total of eight separate data pairs each for head ($X$, $Y_{H}$) and hydraulic torque ($X$, $Y_{T}$).

!table id=conventional_curve caption=Conventional homologous pump curve independent and dependent variables.
| | Independent Variable | Head Dependent Variable | Torque Dependent Variable |
| - | - | - | - |
| A range: | $X = \nu / \alpha$ | $Y_{H} = h / \alpha^{2}$ | $Y_{T} = \beta / \alpha^{2}$ |
| V range: | $X = \alpha / \nu$ | $Y_{H} = h / \nu^{2}$ | $Y_{T} = \beta / \nu^{2}$ |

#### Polar homologous pump data id=polar

A polar homologous representation has been used such that the independent variable is always positive and bounded on $[0, 2\pi]$. The variable transformation allows all octants to be ordered in monotonically-increasing fashion with respect to a single independent variable. The independent ($\theta$) and dependent ($W_{H}$, $W_{T}$) variables for polar homologous pump curves are defined in [polar_curve].

!table id=polar_curve caption=Polar homologous pump curve independent and dependent variables.
| | Independent Variable | Head Dependent Variable | Torque Dependent Variable |
| - | - | - | - |
| A and V ranges: | $\theta = C + \arctan(\alpha / \nu)$ | $W_{H} = h / (\alpha^{2} +\nu^{2})$ | $W_{T} = \beta / (\alpha^{2} +\nu^{2})$ |

The constant $C$ assumes different values depending upon the mode of pump operation. The homologous octants, under this definition of $\theta$, are arranged in a predictable way on
$[0,2\pi]$ as summarized in [polar_octants].

!table id=polar_octants caption=Octant arrangement under the polar homologous representation.
| Octant Identifier | $C$ Value | Portion of Domain on $[0,2\pi]$ |
| - | - | - |
| VN | $0$ | $[0,\pi/4]$ |
| AN | $0$ | $[\pi/4,\pi/2]$ |
| AD | $\pi$ | $[\pi/2,3\pi/4]$ |
| VD | $\pi$ | $[3\pi/4,\pi]$ |
| VT | $\pi$ | $[\pi,5\pi/4]$ |
| AT | $\pi$ | $[5\pi/4,3\pi/2]$ |
| AR | $2\pi$ | $[3\pi/2,7\pi/4]$ |
| VR | $2\pi$ | $[7\pi/4,2\pi]$ |

The input parameters [!param](/Components/ShaftConnectedPump1Phase/head) and [!param](/Components/ShaftConnectedPump1Phase/torque_hydraulic) are functions which relate the polar dependent variables ($W_{H}$, $W_{T}$) to the independent variables ($\theta$). Actual head $[m]$ and torque $[N-m]$ values are computed using the two equations below.

!equation
H = (\alpha^{2} + \nu^{2}) * W_{H} * H_{R}

!equation
\tau_{\text{hydraulic}} = (\alpha^{2} + \nu^{2}) * W_{T} * \tau_{R} * \frac{\rho_{\text{in}}}{\rho_{R}}

### Friction torque id=friction

Friction torque always resists the direction of motion, so the sign of the pump friction torque depends on the sign of the connected shaft speed. Positive shaft speed results in negative friction torque while negative shaft speed results in positive friction torque.

The friction torque magnitude, $\tau_{\text{friction}}$, is a function of shaft speed, $\omega$, and four input parameters. If the ratio of shaft speed to [!param](/Components/ShaftConnectedPump1Phase/omega_rated), $\alpha$, is less than [!param](/Components/ShaftConnectedPump1Phase/speed_cr_fr), friction torque magnitude equals [!param](/Components/ShaftConnectedPump1Phase/tau_fr_const),

!equation
\tau_{\text{friction}} = \tau_{fr,const},


otherwise, $\tau_{\text{friction}}$ is a function of shaft speed and friction torque coefficients, [!param](/Components/ShaftConnectedPump1Phase/tau_fr_coeff),

!equation
\tau_{\text{friction}} = \tau_{\text{fr,coeff}}[0] + \tau_{\text{fr,coeff}}[1] \mid \alpha \mid + \tau_{\text{fr,coeff}}[2] \mid\alpha \mid^{2} + \tau_{\text{fr,coeff}}[3] \mid \alpha \mid^{3}.



### Moment of inertia id=moi

The pump moment of inertia, $I_{\text{pump}}$, is a function of shaft speed, $\omega$, and four input parameters. If the ratio of shaft speed to [!param](/Components/ShaftConnectedPump1Phase/omega_rated), $\alpha$, is less than [!param](/Components/ShaftConnectedPump1Phase/speed_cr_I), pump inertia equals [!param](/Components/ShaftConnectedPump1Phase/inertia_const),

!equation
I_{\text{pump}} = I_{\text{const}},


otherwise, $I_{\text{pump}}$ is a function of shaft speed and inertia coefficients, [!param](/Components/ShaftConnectedPump1Phase/inertia_coeff),

!equation
I_{\text{pump}} = I_{\text{coeff}}[0] + I_{\text{coeff}}[1] \mid \alpha \mid + I_{\text{coeff}}[2] \mid\alpha \mid^{2} + I_{\text{coeff}}[3] \mid \alpha \mid^{3}.


!bibtex bibliography
