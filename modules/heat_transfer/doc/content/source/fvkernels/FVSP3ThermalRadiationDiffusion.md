# FVSP3ThermalRadiationDiffusion

This kernel implements the thermal radiative diffusion term for the SP3 thermal radiation.

## Overview

The diffusion term implemented reads as follows [!citep](larsen2002):
\begin{equation}
-{\nabla}{\cdot}{\frac{{\varepsilon}^2{\mu_i}^2}{{\kappa}}}{\nabla}{\psi_i}
\end{equation}

where:

- $\varepsilon$ is the optical thickness of medium (SI unit (-))
- $\mu_i$ is the dimensionless eigenvalue parameter of targeting order (SI unit (-))
- $\kappa$ is the absorptivity of medium (SI unit ($m^{-1}$))
- $\psi_i$ is the radiative heat flux moment of targeting order integrated in frequency band \[$\nu_i$, $\nu_{i+1}$\] (SI unit ($W m^{-2}$))

!syntax parameters /FVKernels/FVSP3ThermalRadiationDiffusion

!syntax inputs /FVKernels/FVSP3ThermalRadiationDiffusion

!syntax children /FVKernels/FVSP3ThermalRadiationDiffusion
