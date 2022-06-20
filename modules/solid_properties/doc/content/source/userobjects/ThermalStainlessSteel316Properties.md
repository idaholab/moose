# ThermalStainlessSteel316Properties

!syntax description /UserObjects/ThermalStainlessSteel316Properties

## Description

This userobject provides thermal properties for
stainless steel 316 as a function of temperature using curve fits
of data in [!cite](mills).

Thermal conductivity is given as

\begin{equation}
k=-7.301\times 10^{-6}T^2+0.02716T+6.308
\end{equation}

The $R^2$ value of the $k$ curve fit is 0.9960. Uncertainty on the tabulated
data for thermal conductivity is $\pm$ 10%. Isobaric specific heat capacity is given as

\begin{equation}
C_p=0.1816T+428.46
\end{equation}

The $R^2$ value of the $C_p$ curve fit is 0.9926. Uncertainty on the tabulated data
for isobaric specific heat capacity is $\pm$ 5%. Density is given as

\begin{equation}
\rho=-4.454\times 10^{-5}T^2-0.4297T+8089.4
\end{equation}

The $R^2$ value of the $\rho$ curve fit is 0.9995. Uncertainty on the
tabulated data for density is $\pm$ 3%.

## Range of Validity

The properties are valid for 25$\degree$C $\le$ T $\le$ 1300$\degree$C.

## Example Input File Syntax

!listing modules/solid_properties/test/tests/stainless_steel_316/test.i block=UserObjects

!syntax parameters /UserObjects/ThermalStainlessSteel316Properties

!syntax inputs /UserObjects/ThermalStainlessSteel316Properties

!syntax children /UserObjects/ThermalStainlessSteel316Properties

!bibtex bibliography
