# SCMMixingChengTodreas

!syntax description /SCMClosures/SCMMixingChengTodreas

## Overview

!! Intentional comment to provide extra spacing

This closure class is used to model the turbulent mixing coefficient $\beta$ using the Cheng and Todreas correlations. Specifically this closure model applies to triangular assemblies with wire-wrapped pins. The implementation was based on:

- Hydrodynamic models and correlations for bare and wire-wrapped hexagonal rod bundles—bundle friction factors, subchannel friction factors and mixing parameters, Cheng and Todreas [!cite](cheng1986hydrodynamic).

The implementation separates the two Cheng-Todreas uses of $\beta$:

- interior gaps use the base turbulent mixing parameter in the global turbulent crossflow relation $w'_{ij} = \beta S_{ij} \bar{G}$;
- edge/corner gaps return a sweep-flow parameter only when the subchannel enthalpy equation asks for `sweep_flow = true`; this sweep-flow term is applied separately in the triangular-assembly energy equation and is not added to the base turbulent crossflow parameter.

The closure flags a solution warning when $P/D$, $H/D$, number of pins, or $Re$ is outside the data range used for the Cheng-Todreas wire-wrapped mixing correlation.

In the intermittent regime, the original Cheng-Todreas treatment has been simplified in this implementation. The interpolation factor is evaluated as

!equation
\psi = \frac{\ln(Re) - \ln(Re_L)}{\ln(Re_T) - \ln(Re_L)}

using the bulk transition limits

!equation
Re_L = 320 \, 10^{P/D - 1}, \qquad Re_T = 10^4 \, 10^{0.7(P/D - 1)} .

These bulk $Re_L$ and $Re_T$ values are used for both the base turbulent mixing parameter and the peripheral sweep-flow parameter.

Information about the use of $\beta$ can be found in [Turbulent crossflow](subchannel_theory.md#turbulent-crossflow).
Additionally, the user may opt to provide the turbulent momentum mixing parameter `CT`. Information about the use of this parameter can be found in [Turbulent momentum transfer](subchannel_theory.md#turbulent-momentum-transfer).

!syntax parameters /SCMClosures/SCMMixingChengTodreas

!syntax inputs /SCMClosures/SCMMixingChengTodreas

!syntax children /SCMClosures/SCMMixingChengTodreas
