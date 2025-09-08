# FVSP3ThermalRadiationBC

This boundary condition implements the Robin boundary conditions for the radiative heat flux moments for the SP3 method.

## Overview

The Robin boundary condition implemented reads as follows [!citep](larsen2002):
\begin{equation}
\frac{\kappa}{\varepsilon}(-\alpha_i\psi_i -\beta_j\psi_j + \int_{\nu_i}^{\nu_{i+1}}\eta_i{B_b}d\nu)
\end{equation}

where:

- $\kappa$ is the absorpsivity of medium (SI unit ($m^{-1}$))
- $\varepsilon$ is the optical thickness of medium (SI unit (-))
- $\alpha_i$ is the first term coefficient with identical order of flux moments (SI unit (-))
- $\beta_j$ is the second term coefficient with coupled order of flux moments (SI unit (-))
- $\eta_i$ is the third term coefficient with identical order of flux moments (SI unit (-))
- $\nu_{i}$ is the lower boundary of radiation frequency band (SI units ($s^{-1}$))
- $\nu_{i+1}$ is the upper boundary of radiation frequency band (SI unit ($s^{-1}$))
- $\psi_i$ is the radiative heat flux moment of targeting order integrated in frequency band \[$\nu_i$, $\nu_{i+1}$\] (SI unit ($W m^{-2}$))
- $\psi_j$ is the radiative heat flux moment of coupled order integrated in frequency band \[$\nu_i$, $\nu_{i+1}$\] (SI unit ($W m^{-2}$))
- $B_b$ is the monochromatic black body radiation intensity at ambient temperature ({B}({T_b},{\nu})) (SI unit ($W m^{-2} Hz^{-1}$))

More information about black body radiation($B$) can be found on the [FVSP3TemperatureBC](fvbcs/FVSP3TemperatureBC.md).

!syntax parameters /FVBCs/FVSP3ThermalRadiationBC

!syntax inputs /FVBCs/FVSP3ThermalRadiationBC

!syntax children /FVBCs/FVSP3ThermalRadiationBC
