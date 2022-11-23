# ThermalCompositeSiCProperties

!syntax description /SolidProperties/ThermalCompositeSiCProperties

## Description

This userobject provides
thermal properties for composite silicon carbide as a function of temperature.

!include solid_properties_units.md

Isobaric specific heat is calculated from [!cite](snead) as

\begin{equation}
C_p=925.65 + 0.3772 * T - 7.9259\times10^{-5} * T^2 - 3.1946\times 10^7 * T^{-2};
\end{equation}

The uncertainty is $\pm$ 7% in the range 200 K $\le$ T $\le$ 1000 K and $\pm$ 4% in the range
1000 K $\le$ T $\le$ 2400 K.

Thermal conductivity is calculated from [!cite](stone) as

\begin{equation}
k=-1.71\times 10^{-11} T^4+7.35\times 10^{-8}T^3 - 1.10\times 10^{-4}T^2+0.061T+7.97
\end{equation}

The density is assumed constant because the thermal expansion coefficient
of silicon carbide is very small.
A default value is provided as an average
over four different crystal structures at room temperature [!cite](snead) as

\begin{equation}
\rho=3216.0
\end{equation}

## Range of Validity

This userobject is valid for estimating isobaric
specific heat over 200 K $\le$ T $\le$ 2400 K, and for estimating thermal conductivity
over an unspecified range [!cite](stone).

!syntax parameters /SolidProperties/ThermalCompositeSiCProperties

!syntax inputs /SolidProperties/ThermalCompositeSiCProperties

!syntax children /SolidProperties/ThermalCompositeSiCProperties

!bibtex bibliography
