# ThermalGraphiteProperties

!syntax description /SolidProperties/ThermalGraphiteProperties

## Description

This userobject provides thermal properties for graphite
as a function of temperature. Because there are many different
grades of graphite, this userobject computes properties individually
for each grade. Because many grades are encapsulated in this
userobject, the applicability ranges of the correlations are unique to
each grade.

Many of the graphite grades encapsulated in this userobject are coke-based.
Because many coke-based graphite grades show approximately the same specific heat,
it is a reasonable approximation to use the same $C_p$ correlation from
[!cite](butland) for many different grades [!cite](baker).

!include solid_properties_units.md

### H-451

H-451 graphite is a near-isotropic, artificial graphite based on
petroleum coke. H-451 graphite is commonly used for reflectors in nuclear
applications.

Isobaric specific heat is calculated from [!cite](butland) as

\begin{equation}
C_p=4184\left\lbrack 0.54212-2.42667e-6T-90.2725 T^{-1}-43449.3 T^{-3}+1.59309\times 10^7 T^{-3}-1.43688\times 10^9T^{-4}\right\rbrack
\end{equation}

with a validity range of 200 K $\le T \le$ 3500.

The thermal conductivity is calculated from [!cite](nea2018) as

\begin{equation}
k=3.28248\times 10^{-5}T^2-1.24890\times 10^{-1}T+1.69214\times 10^2
\end{equation}

with a validity range of 500 K $\le T \le$ 1800 K.

Density is taken as a constant value; a default value is provided based on
[!cite](nea2018) as

\begin{equation}
\rho=1850.0
\end{equation}

!syntax parameters /SolidProperties/ThermalGraphiteProperties

!syntax inputs /SolidProperties/ThermalGraphiteProperties

!syntax children /SolidProperties/ThermalGraphiteProperties

!bibtex bibliography
