# SCMHTCClosureBase

## Overview

!! Intentional comment to provide extra spacing

This is the base class from which all the heat transfer coefficient closure models inherit. It provides the toolset for the calculation of the Nusselt number and finally the heat transfer coefficient.

## Channel-to-Pin and Channel-to-Duct Heat Transfer Modeling

The pin/duct surface temperature are computed via the heat transfer coefficient as follows:

\begin{equation}
T_{s,\text{pin}}(z) = \frac{1}{N} \sum_{sc=1}^N T_{bulk,sc}(z) + \frac{q'_{\text{pin}}(z)}{\pi D_{\text{pin}}(z) h_{sc}(z)},
\end{equation}

where:

- $T_{s,\text{pin}}(z)$ is the surface temperature for the pin at a height $z$
- $N$ is the number of subchannels neighboring the pin
- $T_{bulk,sc}(z)$ is the bulk temperature for a subchannel $sc$ neighboring the pin at a height $z$
- $q'_{\text{pin}}(z)$ is the linear heat generation rate for the pin at a height $z$
- $D_{\text{pin}}(z)$ is the pin diameter at a height $z$
- $h_{sc}(z)$ is the heat exchange coefficient for a subchannel $sc$ neighboring the pin at a height $z$.

For the duct, the duct surface temperature is defined as follows:

\begin{equation}
T_{s,d}(z) = T_{bulk,d}(z) + \frac{q''_d(z)}{h_d(z)},
\end{equation}

where:
- $T_{s,d}(z)$ is the duct surface temperature at a height $z$
- $T_{bulk,d}(z)$ is the bulk temperature of the subchannel next to the duct node $d$
- $q''_d(z)$ is the heat flux at the duct at a height $z$
- $h_d(z)$ is the heat exchange coefficient for the subchannel next to the duct node at a height $z$

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

The bounding laminar and turbulent Reynolds numbers for the turbulent transition are defined as follows:

\begin{equation}
Re_L = 300 \times 10^{1.7 \times (P/D_{\text{pin}} - 1.0)}
\end{equation}

\begin{equation}
Re_T = 10^4 \times 10^{1.7 \times (P/D_{\text{pin}} - 1.0)}
\end{equation}

where:

- P is the pitch
- D$_{\text{pin}}$ is the pin diameter

The flow is laminar if $Re \leq Re_L$, turbulent if $Re \geq Re_T$, and in the transition regime if it lies in between.
The modeling of each regime is explained below.

#### Laminar Nusselt Number

The following relation is used depending on the subchannel type [!cite](Todreas):

\begin{equation}
\text{Nu}_{\text{laminar}} =
\begin{cases}
4.0 & \text{if subchannel is CENTER} \\
3.7 & \text{if subchannel is EDGE} \\
3.3 & \text{if subchannel is CORNER}
\end{cases}
\end{equation}

### Correlation for Turbulent Nusselt Number

The modeling of the Turbulent Nusselt number and consequently of the heat transfer coefficient `h` is defined by the user. The closure models available to the user that are implemented in SCM are the following:

- [Dittus-Boelter](SCMHTCDittusBoelter.md) (recommended for water coolant)
- [Gnielinski](SCMHTCGnielinski.md) (recommended for liquid metals)
- [Kazimi-Carelli](SCMHTCKazimiCarelli.md) (recommended for liquid metals)

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
