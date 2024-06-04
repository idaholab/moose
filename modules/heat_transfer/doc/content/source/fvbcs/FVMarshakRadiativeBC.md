# FVMarshakRadiativeBC

This boundary condition implements the Marshak boundary conditions for
the radiative heat flux.

## Overview

The Marshak boundary condition implemented reads as follows [!citep](modest2021):
\begin{equation}
F_G = (G - 4 \sigma T_{b, rad}^4) \frac{\epsilon_b}{2 \Gamma (2 - \epsilon_b)}
\end{equation}

where:

- $F_G$ is the normal space derivative of the radiation heat flux at the boundaries defined by the `boundary` parameter
- $G$ is the ratiative heat flux (SI units (W/m$^2$))
- $\sigma$ is the Stefan-Boltzmann constant (SI units (W/m$^2$/K$^4$))
- $\sigma T_{b, rad}$ is the radiation temperature at the boundary (SI units (K))
- $\epsilon_b$ is the emissivity of the boundary (SI units (-))
- $\Gamma$ is the radiation diffusion coefficient (SI units (m))

!syntax parameters /FVBCs/FVMarshakRadiativeBC

!syntax inputs /FVBCs/FVMarshakRadiativeBC

!syntax children /FVBCs/FVMarshakRadiativeBC
