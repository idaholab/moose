# QuadSubChannel1PhaseProblem

!syntax description /Problem/QuadSubChannel1PhaseProblem

## Overview

!! Intentional comment to provide extra spacing

This class solves for the subchannel flow variables in the case of subchannels/pins arranged in a square lattice.
It inherits from the base class : `SubChannel1PhaseProblem`. Information regarding the solver can be found in [subchannel_theory.md].

Pin surface temperature is calculated at the end of the solve using a user-selected correlation.

## Channel-to-Pin Heat Transfer Modeling

!! Intentional comment to provide extra spacing

The pin surface temperature are computed via the convective heat transfer coefficient as follows:

\begin{equation}
T_{s,\text{pin}}(z) = \frac{1}{N} \sum_{sc=1}^N T_{bulk,sc}(z) + \frac{q'_{\text{pin}}(z)}{\pi D_{\text{pin}}(z) h_{sc}(z)},
\end{equation}

where:

- $T_{s,\text{pin}}(z)$ is the surface temperature for the pin at a height $z$
- $N$ is the number of subchannels neighboring the pin
- $T_{bulk,sc}(z)$ is the bulk temperature for a subchannel $sc$ neighboring the pin at a height $z$
- $q'_{\text{pin}}(z)$ is the linear heat generation rate for the pin at a height $z$
- $D_{\text{pin}}(z)$ is the pin diameter at a height $z$
- $h_d(z)$ is the convective heat transfer coefficient for the subchannel next to the duct node at a height $z$

The convective heat transfer coefficientis computed using the Nusselt number (Nu) as follows:

\begin{equation}
h = \frac{\text{Nu} \times k}{D_h}
\end{equation}

where:

- $k$ is the thermal conductivity of the subchannel neighboring the structure
- $D_h$ is the hydraulics diameter of the subchannel neighboring the structure

The modeling of the Nusselt number and consequently of the convective heat transfer coefficient `h` is selected by the user through a closure. The closure models available to the user that are implemented in SCM are the following:

- [Dittus-Boelter](SCMHTCDittusBoelter.md) (recommended for water coolant)
- [Gnielinski](SCMHTCGnielinski.md) (recommended for all types of coolants)
- [Kazimi-Carelli](SCMHTCKazimiCarelli.md) (recommended for liquid metals)
- [Schad-Modified](SCMHTCSchadModified.md) (recommended for liquid metals)
- [Graber-Rieger](SCMHTCGraberRieger.md) (recommended for liquid metals)
- [Borishanskii](SCMHTCBorishanskii.md) (recommended for liquid metals)

All these models inherit from the base class: [SCMHTCClosureBase](SCMHTCClosureBase.md). A synopsis of the closure models availabe in SCM with the range of validity, is presented in Table [HTC models](SCMHTCClosureBase.md#HTC-models).

## Example Input File Syntax

!listing /test/tests/problems/psbt/psbt_implicit.i block=SubChannel language=moose

!syntax parameters /Problem/QuadSubChannel1PhaseProblem

!syntax inputs /Problem/QuadSubChannel1PhaseProblem

!syntax children /Problem/QuadSubChannel1PhaseProblem
