# ThermalUCProperties

!syntax description /SolidProperties/ThermalUCProperties

## Description

This `SolidProperties` object provides thermal properties for Uranium monocarbide as a function of temperature.

!include solid_properties_units.md

Isobaric specific heat is calculated from [!cite](iaea) as

\begin{equation}
C_p= 239.7 - 5.068\times 10^{-3} * T + 1.7604\times10^{-5} * T^2 - 3488100 * T^{-2}
\end{equation}


This is valid for estimating isobaric specific heat over 298 K $\le$ T $\le$ 2838 K

Thermal conductivity is calculated from [!cite](Vasudevamurthy2022) as:

For 323 K $\le$ T $\le$ 923 K
\begin{equation}
k=21.7-3.04\times 10^{-3} * T+3.61\times 10^{-6} * T^2
\end{equation}

And for 924 K $\le$ T $\le$ 2573 K, the thermal conductivity is:
\begin{equation}
k=20.2+1.48\times 10^{-3} T
\end{equation}

The density is assumed constant.
A default value is provided [!cite](Vasudevamurthy2022) as

\begin{equation}
\rho= 13824.7
\end{equation}


!syntax parameters /SolidProperties/ThermalUCProperties

!syntax inputs /SolidProperties/ThermalUCProperties

!syntax children /SolidProperties/ThermalUCProperties

!bibtex bibliography
