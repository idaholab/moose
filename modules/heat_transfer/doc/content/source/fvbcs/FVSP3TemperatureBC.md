# FVSP3TemperatureBC

This boundary condition implements the Robin boundary conditions for the Temperature in Semi-Transparent boundary for the SP3 method.

## Overview

The Robin boundary condition implemented reads as follows [!citep](larsen2002):
\begin{equation}
\frac{1}{\varepsilon k}
\left[
h\,(T_b - T) +
\alpha \pi \left( \frac{n_2}{n_1} \right)^2
\int_{0}^{\nu_1} \left( B(\nu, T_b) - B(\nu, T) \right)\, d\nu
\right]
\end{equation}

where:

- $\varepsilon$ is the optical thickness of medium (SI unit (-))
- $k$ is the thermal conductivity of medium (SI unit ($W m^{-1} K^{-1}$))
- $h$ is the convective heat transfer coefficient of medium (SI unit ($W m^{-2} K^{-1}$))
- $T_b$ is the ambient (boundary) temperature (SI unit (K))
- $n_1$ is the refraction coefficient of incident medium (cavity) (SI unit (-))
- $n_2$ is the refraction coefficient of transmitting medium (boundary) (SI unit (-))
- $\alpha$ is the hemispheric emissivity of the medium (SI unit (-))
- $\nu_{1}$ is the maximum opaque frequency of the medium (SI units ($s^{-1}$))
- $B$ is the monochromatic black body radiation intensity from under Planck function (SI unit ($W sr^{-1} m^{-2}$))

Plank function:
\begin{equation}
{B}({T},{\nu}) = {\frac{n_1^2}{c_0^2}}{\frac{2{h_p}{\nu^3}}{e^{{{h_p}{\nu}}/{({k_B}{T})}}-1}}
\end{equation}

with:

- $h_p := 6.62608\times{10^{-34}} J\cdot{s}$ for the Planck constant
- $k_B := 1.38066\times{10^{-23}} J K^{-1}$ for the Boltzmann constant
- $c_0 := 2.99279\times{10^{8}} m s^{-1}$ for the speed of light

!syntax parameters /FVBCs/FVSP3TemperatureBC

!syntax inputs /FVBCs/FVSP3TemperatureBC

!syntax children /FVBCs/FVSP3TemperatureBC
