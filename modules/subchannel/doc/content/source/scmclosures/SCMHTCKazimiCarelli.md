# SCMHTCKazimiCarelli

!syntax description /SCMClosures/SCMHTCKazimiCarelli

## Overview

### General Expression for the Nusselt number

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

### Kazimi-Carelli Correlation for Turbulent Nusselt Number

The Kazimi-Carelli correlation [!cite](kazimi1976) is used for calculating the Nusselt number in fuel-pin bundles, considering the geometry of the bundle.

\begin{equation}
\text{Nu}_{\text{turbulent}} = 4.0 + 0.33 \times \left( \frac{p}{D} \right)^{3.8} \times \left( \frac{Pe}{100} \right)^{0.86} + 0.16 \times \left( \frac{p}{D} \right)^{5},
\end{equation}

where:

- $Nu$: Nusselt number
- $p$: Pitch, the center-to-center distance between neighboring fuel-pins
- $D$: Diameter of the fuel-pin
- $\frac{p}{D}$: Pitch-to-diameter ratio
- $Pe$: Peclet number ($Pe = Re \times Pr$)
- $Re$: Reynolds number
- $Pr$: Prandtl number

!alert note
The Kazimi-Carelli correlation is not currently implemented for computing the duct surface temperature.

### Transition Regime

A linear interpolation weight is defined as follows:

\begin{equation}
w_T = \frac{Re - Re_L}{Re_T - Re_L}.
\end{equation}

Then, the Nusselt number in the transition regime is defined by linearly interpolating the laminar Nusselt number and the turbulent one,
which is defined with the chosen correlation for the pin or duct, as follows:

\begin{equation}
\text{Nu}_{\text{transition}} = w_T \times \text{Nu}_{\text{turbulent}} + (1.0 - w_T) \times \text{Nu}_{\text{laminar}}.
\end{equation}

!syntax parameters /SCMClosures/SCMHTCKazimiCarelli

!syntax inputs /SCMClosures/SCMHTCKazimiCarelli

!syntax children /SCMClosures/SCMHTCKazimiCarelli
