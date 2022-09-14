# ThermalMonolithicSiCProperties

!syntax description /SolidProperties/ThermalMonolithicSiCProperties

## Description

This userobject provides
thermal properties for monolithic silicon carbide as a function of temperature.

!include solid_properties_units.md

Isobaric specific heat is calculated from [!cite](snead) as

\begin{equation}
C_p=925.65 + 0.3772 * T - 7.9259\times10^{-5} * T^2 - 3.1946\times 10^7 * T^{-2};
\end{equation}

The uncertainty is $\pm$ 7% in the range 200 K $\le$ T $\le$ 1000 K and $\pm$ 4% in the range
1000 K $\le$ T $\le$ 2400 K.

Two methods are available for computing thermal conductivity from
[!cite](snead) and [!cite](stone) as

\begin{equation}
k=\begin{cases}
\frac{1.0}{-0.0003 + 1.05\times 10^{-5} * T} & \text{Snead correlation}\\
-3.70\times 10^{-8}T^3+1.54\times 10^{-4}T^2-0.214T+153.1 & \text{Stone correlation}\\
\end{cases}
\end{equation}

The density is assumed constant because the thermal expansion coefficient
of silicon carbide is very small.
A default value if provided as an average
over four different crystal structures at room temperature [!cite](snead) as

\begin{equation}
\rho=3216.0
\end{equation}

## Range of Validity

This userobject is valid for estimating isobaric
specific heat over 200 K $\le$ T $\le$ 2400 K; and for estimating thermal
conductivity over 300 K $\le$ T $\le$ 1800 K with the Snead correlation
[!cite](snead) and over an unspecified range for the Stone correlation
[!cite](stone).

!syntax parameters /SolidProperties/ThermalMonolithicSiCProperties

!syntax inputs /SolidProperties/ThermalMonolithicSiCProperties

!syntax children /SolidProperties/ThermalMonolithicSiCProperties

!bibtex bibliography
