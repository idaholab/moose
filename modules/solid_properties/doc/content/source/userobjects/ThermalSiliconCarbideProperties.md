# ThermalSiliconCarbideProperties

!syntax description /UserObjects/ThermalSiliconCarbideProperties

## Description

This userobject provides
thermal properties for monolithic silicon carbide as a function of temperature.
These properties may be used for modeling the silicon carbide layer in
[!ac](TRISO) fuels, although there is still much variation in reported properties
for monolithic silicon carbide. Manufacturer-specific correlations are always
preferred.

Isobaric specific heat is calculated from [!cite](snead) as

\begin{equation}
C_p=925.65 + 0.3772 * T - 7.9259\times10^{-5} * T^2 - 3.1946\times 10^7 * T^{-2};
\end{equation}

The uncertainty is $\pm$ 7% in the range 200 K $\le$ T $\le$ 1000 K and $\pm$ 4% in the range
1000 K $\le$ T $\le$ 2400 K. This correlation
agrees reasonably well with the constant value of 1300 J/kg$\dot$K used
by \cite xin_wang_thesis, but is about double the value of 620 J/kg$\cdot$K used by
Hales et. al \cite hales.

Two methods are available for computing thermal conductivity because there is
a very wide variation in thermal conductivity values reported for silicon
carbide. The thermal conductivity of silicon carbide depends strongly on
the grain size, the nature of the grain boundaries,
and the presence of additives such as those included in sintering. For
monolithic silicon carbide, it is recommended to use single crystal or
[!ac](CVD) properties (though even within [!ac](CVD) grades
there can be significant variation) \cite snead.
The first correlation is from [!cite](snead), and generally
applies for [!ac](CVD) silicon carbide. The second correlation
is from [!cite](parfume) and has been used for modeling the same types of
materials as the first correlation, but generally predicts thermal conductivities
about two times smaller than the Snead correlation but in better agreement
with typical values used for modeling [!ac](TRISO) fuels \cite stainsby.

\begin{equation}
k=\begin{cases}
\frac{1.0}{-0.0003 + 1.05\times 10^{-5} * T} & \text{Snead correlation}\\
\frac{17885.0}{T} + 2.0 & \text{PARFUME correlation}\\
\end{cases}
\end{equation}

The density is assumed constant because the thermal expansion coefficient
of silicon carbide is very small.
A default value if provided as an average
over four different crystal structures at room temperature [!cite](snead).

\begin{equation}
\rho=3216.0
\end{equation}

## Range of Validity

This userobject is valid for estimating isobaric
specific heat over 200 K $\le$ T $\le$ 2400 K; for estimating thermal
conductivity over 300 K $\le$ T $\le$ 1800 K with the Snead correlation
[!cite](snead) and over an unspecified range for the second correlation
[!cite](parfume).

## Example Input File Syntax

!listing modules/solid_properties/test/tests/silicon_carbide/test.i block=UserObjects

!syntax parameters /UserObjects/ThermalSiliconCarbideProperties

!syntax inputs /UserObjects/ThermalSiliconCarbideProperties

!syntax children /UserObjects/ThermalSiliconCarbideProperties

!bibtex bibliography
