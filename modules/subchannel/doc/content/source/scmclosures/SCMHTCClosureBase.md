# SCMHTCClosureBase

## Overview

!! Intentional comment to provide extra spacing

This is the base class from which all the convective heat transfer coefficient closure models inherit. It provides the toolset for the calculation of the Nusselt number and finally the heat transfer coefficient.

## Channel-to-Pin and Channel-to-Duct Heat Transfer Modeling

!! Intentional comment to provide extra spacing

The pin/duct surface temperature are computed via the convective heat transfer coefficient as follows:

\begin{equation}
T_{s,\text{pin}}(z) = \frac{1}{N} \sum_{sc=1}^N T_{bulk,sc}(z) + \frac{q'_{\text{pin}}(z)}{\pi D_{\text{pin}}(z) h_{sc}(z)},
\end{equation}

where:

- $T_{s,\text{pin}}(z)$ is the surface temperature for the pin at a height $z$
- $N$ is the number of subchannels neighboring the pin
- $T_{bulk,sc}(z)$ is the bulk temperature for a subchannel $sc$ neighboring the pin at a height $z$
- $q'_{\text{pin}}(z)$ is the linear heat generation rate for the pin at a height $z$
- $D_{\text{pin}}(z)$ is the pin diameter at a height $z$
- $h_{sc}(z)$ is the convective heat transfer coefficient for a subchannel $sc$ neighboring the pin at a height $z$.

For the duct, the duct surface temperature is defined as follows:

\begin{equation}
T_{s,d}(z) = T_{bulk,d}(z) + \frac{q''_d(z)}{h_d(z)},
\end{equation}

where:

- $T_{s,d}(z)$ is the duct surface temperature at a height $z$
- $T_{bulk,d}(z)$ is the bulk temperature of the subchannel next to the duct node $d$
- $q''_d(z)$ is the heat flux at the duct at a height $z$
- $h_d(z)$ is the convective heat transfer coefficient for the subchannel next to the duct node at a height $z$

In both cases, the heat exchange coefficients are computed using the Nusselt number (Nu) as follows:

\begin{equation}
h = \frac{\text{Nu} \times k}{D_h}
\end{equation}

where:

- $k$ is the thermal conductivity of the subchannel neighboring the structure
- $D_h$ is the hydraulic diameter of the subchannel neighboring the structure

!alert note
Currently there is no subchannel-to-duct heat transfer model implemented for square assemblies (square ducts). It only exists for hexagonal assemblies (hexagonal ducts). The subchannel-to-pin model is available for both hexagonal and square assemblies.

### General Expression for the Nusselt number

The laminar, turbulent and transition regimes use different coefficients for the Nusselt number.

The bounding laminar and turbulent Reynolds numbers for the turbulent transition are defined as follows [!cite](chen2018upgraded):

\begin{equation}
Re_L = 320 \times 10^{(P/D_{\text{pin}} - 1.0)}
\end{equation}

\begin{equation}
Re_T = 10^4 \times 10^{0.7 \times (P/D_{\text{pin}} - 1.0)}
\end{equation}

where:

- P is the pitch
- D$_{\text{pin}}$ is the pin diameter

The flow is laminar if $Re \leq Re_L$, turbulent if $Re \geq Re_T$, and in the transition regime if it lies in between.
The modeling of each regime is explained below.

#### Laminar Nusselt Number

The following relation is used depending on the subchannel:

\begin{equation}
\text{Nu}_{\text{laminar}} =
\begin{cases}
3.73 & \text{if subchannel is CENTER} \\
3.59 & \text{if subchannel is EDGE} \\
3.52 & \text{if subchannel is CORNER}
\end{cases}
\end{equation}

The values defined here are chosen based on engineering judgement and the values for laminar flow in a circular/square tube with constant surface temperature (3.66/3.091) and laminar flow in a circular/square tube subjected to constant surface heat flux (4.36/3.54). The heat transfer boundary condition for the interface area between subchannels is considered to be that of constant temperature. The heat transfer boundary condition for the fuel-pin surface and duct surface is considered to be that of constant heat-flux.

For a center subchannel that has approximately half contact with circular fuel pins and half with flat interface, the laminar Nusselt number is chosen to be the average of the two: $\text{Nu}_{\text{laminar}} = (4.36 + 3.091)/2 = 3.73$. Similar logic has been applied for the edge and corner subchannels. For the edge: $\text{Nu}_{\text{laminar}} = (3.54 + 2*4.36 + 3 * 3.091)/6 = 3.59$. For the corner: $\text{Nu}_{\text{laminar}} = (2*3.54 + 4.36 + 2*3.091)/5 = 3.52$

### Correlation for Turbulent Nusselt Number

The modeling of the Turbulent Nusselt number and consequently of the convective heat transfer coefficient `h` is defined by the user. The closure models available to the user that are implemented in SCM are the following:

- [Dittus-Boelter](SCMHTCDittusBoelter.md) (recommended for water coolant)
- [Modified Gnielinski](SCMHTCGnielinski.md) (recommended for duct surface)
- [Kazimi-Carelli](SCMHTCKazimiCarelli.md) (applicable to liquid metals)
- [Schad-Modified](SCMHTCSchadModified.md) (applicable to liquid metals)
- [Graber-Rieger](SCMHTCGraberRieger.md) (applicable to liquid metals)
- [Borishanskii](SCMHTCBorishanskii.md) (applicable to liquid metals)

According to [!cite](todreas2021nuclear1), the correlations of Borishanskii and Schad-modified yield the best agreement over the entire range of P/D values. The Graber and Rieger correlation appears to significantly overpredict the heat transfer coefficient if extended beyond the published range of applicability P/D ≤ 1.15. The Kazimi and Carelli correlation underestimates Nu at high values of P/D. A summary of the closure models available in SCM with the range of validity, is presented in Table [HTC-models] below:

!table id=HTC-models caption=Convective heat transfer coefficient, closure models availabel in `SCM`.
| Name | Geometry  | Fluid Type  | Range of Validity | Rod Spacing | Entrance Effects | References |
|------|--------------------------------|-----------------------------|--------------------------------|--------------|------------------|-------------|
| Dittus-Boelter | Circular tubes | Ordinary non-metallic fluids | $0.7 < Pr < 100, Re>10000$ | Bare (no wire grid or spacers) | None | [!cite](dittus1930heat), [!cite](mcadams1954heat) |
| Dittus-Boelter/Presser | Rod bundles triangular and square | Ordinary non-metallic fluids | triangular: $1.05 \leq P/D \leq 2.2$, square: $1.05 \leq P/D \leq 1.9$ | Bare (no wire grid or spacers) | None | [!cite](presser1967waermeuebergang) |
| Dittus-Boelter/Weisman | Rod bundles triangular and square | Water | triangular: $1.1 \leq P/D \leq 1.5$, square: $1.1 \leq P/D \leq 1.3$ | Bare (no wire grid or spacers) | None | [!cite](weisman1959heat) |
| Modified Gnielinski | Concentric annular ducts / Circular tubes | Extended to most fluids | Extended from original to: $1e-5 \leq Pr \leq 2000$ | Bare (no wire grid or spacers) | None | [!cite](gnielinski1975neue) |
| Kazimi-Carelli | Triangular Rod Bundle | Metallic Fluids | $1.1 \leq P/D \leq 1.4$, $10 \leq Pe \leq 5000$ | Wire-wrapped (no grid or spacers) | None | [!cite](kazimi1976) |
| Schad-Modified | Triangular Rod Bundle | Metallic Fluids | $(1.1 \le P/D \le 1.5)$ and $(0.0 \le Pe \le 1000)$, | Bare (no grid or spacers) | None | [!cite](kazimi1976) |
| Graber-Rieger | Triangular Rod Bundle | Metallic Fluids | $(1.25 \le P/D \le 1.95)$ and $(110 \le Pe \le 4300)$, | Bare (no grid or spacers) | None | [!cite](graber) |
| Borishanskii | Triangular Rod Bundle | Metallic Fluids | $(1.1 \le P/D \le 1.5)$ and $(0.0 \le \mathrm{Pe} \le 2200)$, | Bare (no grid or spacers) | None | [!cite](borishanskii1969heat) |

!alert note
The following features are not currently implemented: spacer grids in quad lattices, entrance region effects. Though it is important to point out that, the value of the turbulent Nu is insensitive to the boundary conditions for Pr > 0.7 [!cite](todreas2021nuclear1).

### Transition Regime

In the transition region we use a linear interpolation. The linear interpolation weight is defined as follows:

\begin{equation}
w_T = \frac{Re - Re_L}{Re_T - Re_L}.
\end{equation}

Then, the Nusselt number in the transition regime is defined by linearly interpolating the laminar Nusselt number and the turbulent one,
which is defined with the chosen correlation for the pin or duct, as follows:

\begin{equation}
\text{Nu}_{\text{transition}} = w_T \times \text{Nu}_{\text{turbulent}} + (1.0 - w_T) \times \text{Nu}_{\text{laminar}}.
\end{equation}
