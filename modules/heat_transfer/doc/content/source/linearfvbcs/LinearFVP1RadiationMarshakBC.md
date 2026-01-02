# LinearFVP1RadiationMarshakBC

This boundary condition implements the Marshak boundary conditions for
the radiative heat flux in the LinearFV system.

## Overview

The Marshak boundary condition implemented reads as follows [!citep](modest2021):
\begin{equation}
(-\Gamma \nabla G) \cdot \vec{n_b}   = (G - 4 \sigma T_{b, rad}^4) \frac{\epsilon_b}{2 (2 - \epsilon_b)}
\end{equation}

where:

- $G$ is the radiative heat flux (SI units (W/m$^2$))
- $\sigma$ is the Stefan-Boltzmann constant (SI units (W/m$^2$/K$^4$))
-  $\sigma_a$ is the absorption coefficient (SI units (1/m))
- $T_{b, rad}$ is the radiation temperature at the boundary (SI units (K))
- $\epsilon_b$ is the emissivity of the boundary (SI units (-))
- $\Gamma$ is the P1 radiation diffusion coefficient (SI units (m))


## Example Input File Syntax

!listing test/tests/radiation_participating_media/rad_isothermal_medium_1d.i block=LinearFVBCs

!syntax parameters /LinearFVBCs/LinearFVP1RadiationMarshakBC

!syntax inputs /LinearFVBCs/LinearFVP1RadiationMarshakBC

!syntax children /LinearFVBCs/LinearFVP1RadiationMarshakBC
