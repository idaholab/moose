# MechanicalCryogenicJK2LBProperties

!syntax description /SolidProperties/MechanicalCryogenicJK2LBProperties

## Description

This userobject provides mechanical properties for JK2LB austenitic stainless steel (Japanese designation for cryogenic structural steel) as a function of temperature. JK2LB is used as the structural jacket in ITER superconducting cable assemblies.

!include solid_properties_units.md

### Young's Modulus

Young's modulus is given by a linear correlation from Princeton Plasma Physics Laboratory report PPPL-4687 [!cite](pppl4687):

\begin{equation}
E(T) = E_0 + E_1 \cdot T
\end{equation}

where $E_0 = 2.001 \times 10^{11}$ Pa and $E_1 = -2.768 \times 10^{7}$ Pa/K. The correlation is fitted to experimental data: $E(4~\text{K}) = 200$ GPa and $E(293~\text{K}) = 192$ GPa.

### Poisson's Ratio

Poisson's ratio is constant:

\begin{equation}
\nu = 0.268
\end{equation}

This value is from Lu et al. (2009) [!cite](LU2009133) at room temperature and is assumed constant over the cryogenic range.

### Thermal Expansion Coefficient

The thermal expansion coefficient is computed as the temperature derivative of the thermal strain. The thermal strain $(L - L_{293})/L_{293}$ is fitted to experimental data from Lu et al. (2009) [!cite](LU2009133) using a quintic polynomial:

\begin{equation}
\varepsilon(T) = c_0 + c_1 T + c_2 T^2 + c_3 T^3 + c_4 T^4 + c_5 T^5
\end{equation}

where the coefficients are determined via nonlinear least squares regression (scipy.optimize.curve_fit). The $R^2$ value of the fit is 0.9998, RMSE = $9.41 \times 10^{-4}$, and maximum error = $2.25 \times 10^{-3}$.

The thermal expansion coefficient is then:

\begin{equation}
\alpha(T) = \frac{d\varepsilon}{dT} = c_1 + 2c_2 T + 3c_3 T^2 + 4c_4 T^3 + 5c_5 T^4
\end{equation}

Note that $\alpha(T)$ represents the instantaneous thermal expansion coefficient at temperature $T$, which is the derivative of the integrated thermal strain. At low temperatures (T < 50 K), $\alpha(T)$ is negative, indicating thermal contraction relative to the reference state at 293 K. At higher temperatures, $\alpha(T)$ becomes positive and increases with temperature.

## Range of Validity

- Young's modulus and Poisson's ratio: 4 K $\le$ T $\le$ 293 K (-269 C to 20 C)
- Thermal expansion coefficient: 10 K $\le$ T $\le$ 300 K (-263 C to 27 C)

## Example Input File Syntax

!listing test/tests/solidproperties/jk2lb/mechanical/mechanical_solid_properties.i block=SolidProperties

!syntax parameters /SolidProperties/MechanicalCryogenicJK2LBProperties

!syntax inputs /SolidProperties/MechanicalCryogenicJK2LBProperties

!syntax children /SolidProperties/MechanicalCryogenicJK2LBProperties

!bibtex bibliography
