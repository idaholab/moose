# ElectromagneticCryogenicJK2LBProperties

!syntax description /SolidProperties/ElectromagneticCryogenicJK2LBProperties

## Description

This userobject provides electromagnetic properties for JK2LB austenitic stainless steel (Japanese designation for cryogenic structural steel) as a function of temperature. JK2LB is used as the structural jacket in ITER superconducting cable assemblies.

!include solid_properties_units.md

### Electrical Resistivity

Electrical resistivity is given by a quintic polynomial fitted to experimental data from Lu et al. (2009) [!cite](LU2009133):

\begin{equation}
\rho(T) = c_0 + c_1 T + c_2 T^2 + c_3 T^3 + c_4 T^4 + c_5 T^5
\end{equation}

where the coefficients are determined via nonlinear least squares regression (scipy.optimize.curve_fit). The $R^2$ value of the fit is 0.9987, RMSE = $8.03 \times 10^{-10}$ Ω·m, and maximum error = $2.25 \times 10^{-9}$ Ω·m.

The quintic polynomial captures the antiferromagnetic phase transition signature at the Néel temperature (240 K), where Lu et al. report: "The antiferromagnetic phase transition is also evident in the resistivity vs. T curve." The fit shows a characteristic slope change in dρ/dT at this temperature.

### Electrical Conductivity

Electrical conductivity is computed as the reciprocal of resistivity:

\begin{equation}
\sigma(T) = \frac{1}{\rho(T)}
\end{equation}

The temperature derivative is obtained via the chain rule:

\begin{equation}
\frac{d\sigma}{dT} = -\frac{1}{\rho^2} \frac{d\rho}{dT}
\end{equation}

### Magnetic Permeability

Magnetic permeability is constant:

\begin{equation}
\mu = \mu_0 = 4\pi \times 10^{-7} \text{ H/m}
\end{equation}

JK2LB is an austenitic stainless steel that is non-magnetic at room temperature and antiferromagnetic below 240 K. Neither phase exhibits significant magnetic permeability beyond the vacuum value $\mu_0$.

## Range of Validity

The properties are valid for 2 K $\le$ T $\le$ 300 K (-271 C to 27 C).

## Example Input File Syntax

!listing test/tests/solidproperties/jk2lb/electromagnetic/electromagnetic_solid_properties.i block=SolidProperties

!syntax parameters /SolidProperties/ElectromagneticCryogenicJK2LBProperties

!syntax inputs /SolidProperties/ElectromagneticCryogenicJK2LBProperties

!syntax children /SolidProperties/ElectromagneticCryogenicJK2LBProperties

!bibtex bibliography
