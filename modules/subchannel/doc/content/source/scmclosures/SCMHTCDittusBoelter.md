# SCMHTCDittusBoelter

!syntax description /SCMClosures/SCMHTCDittusBoelter

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

### Dittus-Boelter Correlation for Turbulent Nusselt Number

The Dittus-Boelter equation [!cite](incropera1990) is implemented as follows:

\begin{equation}
\text{Nu}_{\text{turbulent}} = 0.023 \times Re^{0.8} \times Pr^{0.4},
\end{equation}

where:

- $Nu$: Nusselt number
- $Re$: Reynolds number
- $Pr$: Prandtl number

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

!syntax parameters /SCMClosures/SCMHTCDittusBoelter

!syntax inputs /SCMClosures/SCMHTCDittusBoelter

!syntax children /SCMClosures/SCMHTCDittusBoelter
