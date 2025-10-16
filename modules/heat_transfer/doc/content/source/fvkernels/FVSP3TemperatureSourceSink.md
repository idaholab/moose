# FVSP3TemperatureSourceSink

This kernel implements irradiation-consistent temperature source.

## Overview

The temperature source implemented reads as follows [!citep](larsen2002):
\begin{equation}
{\sum}_{i}{\nabla}{\cdot}{\frac{1}{\kappa}}{\nabla}{(a_1\psi_1+a_2\psi_2)}
\end{equation}

where:

- $\kappa$ is the absorptivity of medium (SI unit ($m^{-1}$))
- $\psi_1$ is the first order radiative heat flux moment integrated in frequency band \[$\nu_i$, $\nu_{i+1}$\] (SI unit ($W m^{-2}$))
- $\psi_2$ is the second order radiative heat flux moment integrated in frequency band \[$\nu_i$, $\nu_{i+1}$\] (SI unit ($W m^{-2}$))
- $a_1$ is the mixing weight for first order moment (SI unit (-))
- $a_2$ is the mixing weight for second order moment (SI unit (-))

!syntax parameters /FVKernels/FVSP3TemperatureSourceSink

!syntax inputs /FVKernels/FVSP3TemperatureSourceSink

!syntax children /FVKernels/FVSP3TemperatureSourceSink
