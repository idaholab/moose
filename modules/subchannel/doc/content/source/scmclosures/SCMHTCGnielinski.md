# SCMHTCGnielinski

!syntax description /SCMClosures/SCMHTCGnielinski

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

### Gnielinski Correlation for Turbulent Nusselt Number

A modified Gnielinski correlation for low Prandtl numbers is used for calculating the Nusselt number for transitional and turbulent flows. The baseline correlation is taken from Angelucci's work [!cite](angelucci2018). The modified correlation reads as follows:

\begin{equation}
\text{Nu}_{\text{turbulent}} = \frac{f/8.0 \times (Re - 1000) \times (Pr + 0.01)}{1 + 12.7 \times \sqrt{f/8.0} \times \left( (Pr + 0.01)^{2/3} - 1 \right)},
\end{equation}

where:

- $Nu$: Nusselt number
- $Re$: Reynolds number
- $Pr$: Prandtl number
- $f$: Darcy friction factor

The key modification in the correlation is the addition of $0.01$ to the Prandtl number.
This modification retains predictions within experimental uncertainty at high $Pr$ numbers but enables the correlation to be used at low $Pr$ numbers.
With this modification, at low $Pr$ numbers (approximately for $Pr < 0.01$), one can expect behavior similar to that of the Lubarsky and Kaufman correlation from this modified Gnielinski correlation.

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

!syntax parameters /SCMClosures/SCMHTCGnielinski

!syntax inputs /SCMClosures/SCMHTCGnielinski

!syntax children /SCMClosures/SCMHTCGnielinski
