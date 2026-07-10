# SCMFrictionUpdatedChengTodreas

!syntax description /SCMClosures/SCMFrictionUpdatedChengTodreas

## Overview

!! Intentional comment to provide extra spacing

This class is used to model the axial friction factor for a subchannel assembly with wire-wrapped/bare fuel pins in a triangular lattice or bare fuel pins in a quadrilateral lattice. It implements the updated Cheng Todreas correlations [!cite](todreas2021nuclear1), [!cite](chen2018upgraded).

For triangular lattices, the closure flags a solution warning when $P/D$, wire-wrap $H/D$, number of pins, or $Re$ is outside the updated Cheng-Todreas friction correlation data range.

In the intermittent regime between the laminar and turbulent friction factor limits, the original Cheng-Todreas treatment has been simplified in this implementation. The interpolation factor is evaluated as

!equation
\psi = \frac{\ln(Re/Re_L)}{\ln(Re_T/Re_L)}

using the bulk transition limits

!equation
Re_L = 320 \, 10^{P/D - 1}, \qquad Re_T = 10^4 \, 10^{0.7(P/D - 1)} .

These bulk $Re_L$ and $Re_T$ values are used for both triangular and quadrilateral lattice friction factor calculations.
The updated Cheng-Todreas friction correlation uses natural logarithms.

!syntax parameters /SCMClosures/SCMFrictionUpdatedChengTodreas

!syntax inputs /SCMClosures/SCMFrictionUpdatedChengTodreas

!syntax children /SCMClosures/SCMFrictionUpdatedChengTodreas
