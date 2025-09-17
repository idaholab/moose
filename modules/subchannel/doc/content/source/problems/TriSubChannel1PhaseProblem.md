# TriSubChannel1PhaseProblem

!syntax description /Problem/TriSubChannel1PhaseProblem

## Overview

!! Intentional comment to provide extra spacing

This class solves for the subchannel flow variables in the case of subchannels/pins arranged in a triangular lattice.
It inherits from the base class : `SubChannel1PhaseProblem`. Information regarding the solver can be found in [subchannel_theory.md].

Pin surface temperature is calculated at the end of the solve, if there is a PinMesh using Dittus Boelter correlation.

## Channel-to-Pin and Channel-to-Duct Heat Transfer Modeling

The pin surface temperature are computed via the heat transfer coefficient as follows:

\being{equation}
T_{s,\text{pin}}(z) = \frac{1}{N} \sum_{sc=1}^N T_{bulk,sc}(z) + \frac{q'_{\text{pin}}(z)}{\pi D_{\text{pin}}(z) h_{sc}(z)},
\end{equation}

where:
- $T_{s,\text{pin}}(z)$ is the surface temperature for the pin at a height $z$
- $N$ is the number of subchannel neighboring the pin
- $T_{bulk,sc}(z)$ is the bulk temperature for a subchannel $sc$ neighboring the pin at a height $z$
- $q'_{\text{pin}}(z)$ is the linear heat generation rate for the pin at a height $z$
- $D_{\text{pin}}(z)$ is the pin diameter at a height $z$
- $h_{sc}(z)$ is the heat exchange coefficient for a subchannel $sc$ neighboring the pin at a height $z$

For the duct, the duct surface temperature is defined as follows:

\being{equation}
T_{s,d}(z) = T_{bulk,d}(z) + \frac{q''_d(z)}{h_d(z)},
\end{equation}

where:
- $T_{s,d}(z)$ is the surface temperature for the duct at a height $z$
- $T_{bulk,d}(z)$ is the bulk temperature the subchannel next to the duct node $d$
- $q''_d(z)$ is the heat flux at the duct at a height $z$
- $h_d(z)$ is the heat exchange coefficient for for the subchannel next to the duct node at a height $z$

In both cases, the heat exchange coefficients are computed using the the Nusselt number (Nu) as follows:

\being{equation}
h = \frac{\text{Nu} \times k}{D_h}
\end{equation}

where:
- $k$ is the thermal conductivity of the subchannel neighboring the structure
- $D_h$ is the hydraulics diamter of the subchannel neighboring the structure

The following correlations have been implemented for the Nusselt number
which can be selected via the [!param](/Problem/TriSubChannel1PhaseProblem/pin_htc_correlation) parameter
for the pin and the [!param](/Problem/TriSubChannel1PhaseProblem/duct_htc_correlation) for the duct, respectively.

### Dittus-Boelter Equation
The Dittus-Boelter equation~\cite{incropera1990} is implemented as follows:

\being{equation}
Nu = 0.023 \times Re^{0.8} \times Pr^{0.4},
\end{equation}

where:
- $Nu$: Nusselt number
- $Re$: Reynolds number
- $Pr$: Prandtl number

### Gnielinski Correlation
The modified Gnielinski correlation for low Prandtl numbers~\cite{angelucci2018} is used for calculating the Nusselt number for transitional and turbulent flows as follows:

\being{equation}
Nu = \frac{f/8.0 \times (Re - 1000) \times (Pr + 0.01)}{1 + 12.7 \times \sqrt{f/8.0} \times \left( (Pr + 0.01)^{2/3} - 1 \right)},
\end{equation}

where:
- $Nu$: Nusselt number
- $Re$: Reynolds number
- $Pr$: Prandtl number
- $f: Darcy friction factor

### Kazimi-Carelli Correlation
The Kazimi-Carelli correlation~\cite{kazimi1976} is used for calculating the Nusselt number in rod bundles, considering the geometry of the bundle.

\being{equation}
Nu = 4.0 + 0.33 \times \left( \frac{p}{D} \right)^{3.8} \times \left( \frac{Pe}{100} \right)^{0.86} + 0.16 \times \left( \frac{p}{D} \right)^{5},
\end{equation}

where:
- $Nu$: Nusselt number
- $p$: Pitch, the center-to-center distance between adjacent rods
- $D$: Diameter of the rod
- $\frac{p}{D}$: Pitch-to-diameter ratio
- $Pe$: Peclet number ($Pe = Re \times Pr$)
- $Re$: Reynolds number
- $Pr$: Prandtl number

!alert note
The Kazimi-Carelli correlation is not currently implemented for computing the duct surface temperature.
The code will error out if 'Kazimi-Carelli' is used in [!param](/Problem/TriSubChannel1PhaseProblem/duct_htc_correlation).

## Example Input File Syntax

!listing /test/tests/problems/Lead-LBE-19pin/test_LEAD-19pin.i block=Problem language=moose

!syntax parameters /Problem/TriSubChannel1PhaseProblem

!syntax inputs /Problem/TriSubChannel1PhaseProblem

!syntax children /Problem/TriSubChannel1PhaseProblem
