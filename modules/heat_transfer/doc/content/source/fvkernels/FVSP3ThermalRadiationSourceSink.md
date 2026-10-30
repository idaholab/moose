# FVSP3ThermalRadiationSourceSink

This kernel implements the thermal radiative source and sink term for the SP3 thermal radiation.

## Overview

The source and sink term implemented reads as follows [!citep](larsen2002):
\begin{equation}
{\kappa}({\psi_i}-{\int_{\nu_i}^{\nu_{i+1}}}4{\pi}Bd{\nu})
\end{equation}

where:

- $\kappa$ is the optical thickness of medium (SI unit ($m^{-1}$))
- $\psi_i$ is the radiative heat flux moment of targeting order integrated in frequency band \[$\nu_i$, $\nu_{i+1}$\] (SI unit ($W m^{-2}$))
- $\nu_{i}$ is the lower boundary of radiation frequency band (SI units ($s^{-1}$))
- $\nu_{i+1}$ is the upper boundary of radiation frequency band (SI unit ($s^{-1}$))
- $B$ is the monochromatic black body radiation intensity at certain temperature (SI unit ($W m^{-2} Hz^{-1}$))

More information about black body radiation($B$) can be found on the [FVSP3TemperatureBC](fvbcs/FVSP3TemperatureBC.md).

!syntax parameters /FVKernels/FVSP3ThermalRadiationSourceSink

!syntax inputs /FVKernels/FVSP3ThermalRadiationSourceSink

!syntax children /FVKernels/FVSP3ThermalRadiationSourceSink
