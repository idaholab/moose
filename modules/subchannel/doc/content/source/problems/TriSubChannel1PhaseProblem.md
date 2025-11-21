# TriSubChannel1PhaseProblem

!syntax description /Problem/TriSubChannel1PhaseProblem

## Overview

!! Intentional comment to provide extra spacing

This class solves for the subchannel flow variables in the case of subchannels/pins arranged in a triangular lattice.
It inherits from the base class : `SubChannel1PhaseProblem`. Information regarding the solver can be found in [subchannel_theory.md].

Pin surface temperature is calculated at the end of the solve using a user-selected correlation.

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
- $D_h$ is the hydraulics diameter of the subchannel neighboring the structure

The modeling of the Nusselt number and consequently of the heat transfer coefficient `h` is selected by the user through a closure. The closure models available to the user that are implemented in SCM are the following:

- [Dittus-Boelter](SCMHTCDittusBoelter.md) (recommended for water coolants)
- [Gnielinski](SCMHTCGnielinski.md) (recommended for liquid metals)
- [Kazimi-Carelli](SCMHTCKazimiCarelli.md) (recommended for liquid metals)

All these models inherit from the base class: [SCMHTCClosureBase](SCMHTCClosureBase.md)

## Example Input File Syntax

!listing /test/tests/problems/Lead-LBE-19pin/test_LEAD-19pin.i block=SubChannel language=moose

!syntax parameters /Problem/TriSubChannel1PhaseProblem

!syntax inputs /Problem/TriSubChannel1PhaseProblem

!syntax children /Problem/TriSubChannel1PhaseProblem
