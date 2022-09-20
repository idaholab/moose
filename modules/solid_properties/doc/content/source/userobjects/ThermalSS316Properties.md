# ThermalSS316Properties

!syntax description /SolidProperties/ThermalSS316Properties

## Description

This userobject provides thermal properties for
stainless steel 316 as a function of temperature using curve fits
of data in [!cite](mills).

!include solid_properties_units.md

Thermal conductivity is given as

\begin{equation}
k=-7.301\times 10^{-6}T^2+0.02716T+6.308
\end{equation}

Uncertainty on the original tabulated
data for thermal conductivity is $\pm$ 10%, and the $R^2$ value of the curve fit is 0.9960.

Isobaric specific heat capacity is given as

\begin{equation}
C_p=0.1816T+428.46
\end{equation}

Uncertainty on the original tabulated data
for isobaric specific heat capacity is $\pm$ 5%, and the $R^2$ value of the curve fit is 0.9926.

Density is given as

\begin{equation}
\rho=-4.454\times 10^{-5}T^2-0.4297T+8089.4
\end{equation}

Uncertainty on the original
tabulated data for density is $\pm$ 3%, and the $R^2$ value of the curve
fit is 0.9995.

## Range of Validity

The properties are valid for 25$\degree$C $\le$ T $\le$ 1300$\degree$C.

!syntax parameters /SolidProperties/ThermalSS316Properties

!syntax inputs /SolidProperties/ThermalSS316Properties

!syntax children /SolidProperties/ThermalSS316Properties

!bibtex bibliography
