# EffectiveTurbulentThermalConductivityAux

!syntax description /AuxKernels/EffectiveTurbulentThermalConductivityAux

## Overview

This is the auxiliary kernel used to compute the effective turbulent thermal conductivity.
In wall-bounded turbulent flows, the effective turbulent thermal conductivity requires the use
of a temperature wall function that computes its value at the wall as a boundary condition
[LinearFVTemperatureWallFunctionBC.md].

\begin{equation}
k_eff = \frac{c_p \mu_t}{Pr_t} +k\,,
\end{equation}

where:

- $c_p$ is the specific heat at constant pressure,
- $\mu_t$ is the dynamic turbulent viscosity,
- $Pr_t$ is the turbulent Prandtl number, which usually ranges between 0.3 and 0.9,
- $k$ is the thermal conductivity.

!syntax parameters /AuxKernels/EffectiveTurbulentThermalConductivityAux

!syntax inputs /AuxKernels/EffectiveTurbulentThermalConductivityAux

!syntax children /AuxKernels/EffectiveTurbulentThermalConductivityAux
