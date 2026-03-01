# SCMMixingKimAndChung

!syntax description /SCMClosures/SCMMixingKimAndChung

## Overview

!! Intentional comment to provide extra spacing

This closure class is used to model the turbulent mixing coefficient $\beta$ using the Kim and Chung correlations. Specifically this closure model applies to triangular and quadrilateral assemblies with bare pins. The implementation followed:

- A scale analysis of the turbulent mixing rate for various Prandtl number flow fields in rod bundles eq 25,Kim and Chung (2001) [!cite](kim2001scale).
- Modeling of flow blockage in a liquid metal-cooled reactor subassembly with a subchannel analysis code eq 19, Jeong et. al (2005)[!cite](jeong2005modeling).

Information about the use of $\beta$ can be found in [Turbulent crossflow](subchannel_theory.md#turbulent-crossflow).
Additionally, the user may opt to provide the turbulent momentum mixing parameter `CT`. Information about the use of this parameter can be found in [Turbulent momentum transfer](subchannel_theory.md#turbulent-momentum-transfer).

!syntax parameters /SCMClosures/SCMMixingKimAndChung

!syntax inputs /SCMClosures/SCMMixingKimAndChung

!syntax children /SCMClosures/SCMMixingKimAndChung
