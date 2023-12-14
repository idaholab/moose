# TurbulentConductivityAux

!syntax description /AuxKernels/TurbulentConductivityAux

## Overview

This is the auxiliary kernel used to compute the thermal effective turbulent conductivity

\begin{equation}
k_t = \frac{c_p \mu_t}{Pr_t} \,,
\end{equation}

where:

- $c_p$ is the specific heat at constant pressure,
- $\mu_t$ is the dynamic turbulent viscosity,
- $Pr_t$ is the turbulent Prandtl number, which usually ranges between 0.3 and 0.9.

!syntax parameters /AuxKernels/TurbulentConductivityAux

!syntax inputs /AuxKernels/TurbulentConductivityAux

!syntax children /AuxKernels/TurbulentConductivityAux
