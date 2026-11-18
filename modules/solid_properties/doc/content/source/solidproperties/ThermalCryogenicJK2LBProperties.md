# ThermalCryogenicJK2LBProperties

!syntax description /SolidProperties/ThermalCryogenicJK2LBProperties

## Description

This userobject provides thermal properties for JK2LB austenitic stainless steel as a function of temperature. JK2LB is a Japanese-grade cryogenic austenitic stainless steel used as the structural jacket in ITER superconducting cable assemblies.

Property correlations were derived by fitting log-polynomial functions to experimental data digitized from [!cite](takahashi2008cryogenic) using nonlinear least squares regression (scipy.optimize.curve_fit). The functional form matches the pattern used by NIST for cryogenic materials properties, including OFHC copper and 304 stainless steel.

!include solid_properties_units.md

### Thermal Conductivity

Thermal conductivity is given by a 5-term log-polynomial:

\begin{equation}
\log_{10}(k) = c_0 + c_1 \log_{10}(T) + c_2 \log_{10}(T)^2 + c_3 \log_{10}(T)^3 + c_4 \log_{10}(T)^4
\end{equation}

where $k$ is in W/(m·K) and $T$ is in K. The fit quality is $R^2 = 0.999832$ with RMSE = 0.042 W/(m·K) and maximum error = 0.08 W/(m·K).

### Specific Heat

Specific heat is given by an 8-term log-polynomial:

\begin{equation}
\log_{10}(c_p) = \sum_{i=0}^{7} c_i \log_{10}(T)^i
\end{equation}

where $c_p$ is in J/(kg·K) and $T$ is in K. This functional form is identical to that used by NIST for cryogenic copper and 304 stainless steel. The fit quality is $R^2 = 0.999898$ with RMSE = 1.59 J/(kg·K) and maximum error = 5.6 J/(kg·K).

The specific internal energy is computed by numerical integration of $c_p(T)$ using the composite trapezoid rule.

### Density

Density is constant at $\rho = 7830$ kg/m$^3$ based on the reference data.

## Range of Validity

The properties are valid for 2 K $\le$ T $\le$ 300 K (-271 °C to 27 °C).

The log-polynomial functional form naturally handles the wide dynamic range of cryogenic properties (thermal conductivity spans 0.28 to 10.7 W/(m·K), specific heat spans 0.4 to 467 J/(kg·K)) and captures the power-law behavior at low temperatures consistent with Debye theory.

## Example Input File Syntax

!listing test/tests/solidproperties/jk2lb/thermal/tests language=python

!syntax parameters /SolidProperties/ThermalCryogenicJK2LBProperties

!syntax inputs /SolidProperties/ThermalCryogenicJK2LBProperties

!syntax children /SolidProperties/ThermalCryogenicJK2LBProperties

!bibtex bibliography
