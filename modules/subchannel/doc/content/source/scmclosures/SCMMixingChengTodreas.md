# SCMMixingChengTodreas

!syntax description /SCMClosures/SCMMixingChengTodreas

## Overview

!! Intentional comment to provide extra spacing

This closure class is used to model the turbulent mixing coefficient $\beta$ using the Cheng and Todreas correlations. Specifically this closure model applies to triangular assemblies with wire-wrapped pins. The implementation was based on:

- Hydrodynamic models and correlations for bare and wire-wrapped hexagonal rod bundles—bundle friction factors, subchannel friction factors and mixing parameters, Cheng and Todreas [!cite](cheng1986hydrodynamic).

Information about the use of $\beta$ can be found in [Turbulent crossflow](subchannel_theory.md#turbulent-crossflow).
Additionally, the user may opt to provide the turbulent momentum mixing parameter `CT`. Information about the use of this parameter can be found in [Turbulent momentum transfer](subchannel_theory.md#turbulent-momentum-transfer).

!syntax parameters /SCMClosures/SCMMixingChengTodreas

!syntax inputs /SCMClosures/SCMMixingChengTodreas

!syntax children /SCMClosures/SCMMixingChengTodreas
