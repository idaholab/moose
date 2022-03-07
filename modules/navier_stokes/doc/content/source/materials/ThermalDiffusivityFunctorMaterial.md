# ThermalDiffusivityFunctorMaterial

!syntax description /Materials/ThermalDiffusivityFunctorMaterial

## Overview

The `ThermalDiffusivityFunctorMaterial` object computes the thermal diffusivity

\begin{equation}
\alpha = \frac{k}{\rho c_p}
\end{equation}

where $k$ is the thermal conductivity, $\rho$ is the density, and $c_p$ is the
constant-pressure specific heat capacity. Thermal diffusivity has units of
length squared over time and is the heat transport analog of the kinematic
viscosity in momentum transport and the diffusion coefficient in species/mass
transport. This object takes $k$, $\rho$, and $c_p$ as input functors and
produces $\alpha$ as an output functor which will be evaluated on-demand.

!syntax parameters /Materials/ThermalDiffusivityFunctorMaterial

!syntax inputs /Materials/ThermalDiffusivityFunctorMaterial

!syntax children /Materials/ThermalDiffusivityFunctorMaterial
