# LithiumFluidProperties

!syntax description /FluidProperties/LithiumFluidProperties

These properties are the recommended correlations for liquid lithium of the IUPAC critical evaluation of alkali metal properties [!citep](Ohse1985), as tabulated by [!citet](Zinkle1998), the same compilation used by [LeadLithiumFluidProperties.md].
All properties only depend on temperature; the fluid is considered incompressible.
The fluid properties are summarized in [tab:lithium]. The temperature $T$ is in K.

!table id=tab:lithium caption=Liquid lithium properties from [!citep](Ohse1985), as tabulated by [!citet](Zinkle1998).
| Properties | Equation |
| :--------- | :------- |
| Density, $\rho$ (kg/m^3) | $278.5 - 0.04657\,T + 274.6\,(1 - T/3500)^{0.467}$ |
| Isobaric specific heat, $c_p$ (J/kg-K) | $4754 - 0.925\,T + 2.91 \times 10^{-4}\,T^2$ |
| Thermal conductivity, $k$ (W/m-K) | $22.28 + 0.0500\,T - 1.243 \times 10^{-5}\,T^2$ |
| Dynamic viscosity, $\mu$ (Pa-s) | $\exp\left(-4.164 - 0.6374 \ln T + 292.1/T\right)$ |

The specific enthalpy is the integral of $c_p$ from the melting temperature, 454 K, so that $h = 0$ for the liquid at the melting point.
The isochoric specific heat is taken equal to the isobaric specific heat, as is conventional for an incompressible liquid.
The temperature is recovered from the density or the enthalpy with a Newton solve.

## Range of validity

The correlations are given for 455 K to 1500 K [!citep](Zinkle1998). An invalid solution is flagged outside of this range.

## Comparison with other compilations

The NASA compilation of [!citet](Davison1968) and the evaluation of experimental data published from 1950 to 2020 by [!citet](Chusov2022) were compared with these correlations between 500 K and 1000 K.
Density, specific heat and thermal conductivity agree within about 5% among the three.
For the dynamic viscosity, [!citet](Chusov2022) agrees with the correlation used here within 4% from 500 K to 1500 K, while [!citet](Davison1968) is 13% to 20% higher between 500 K and 1000 K; its fit includes a data set it reports as about 25% above the others.

!syntax parameters /FluidProperties/LithiumFluidProperties

!syntax inputs /FluidProperties/LithiumFluidProperties

!syntax children /FluidProperties/LithiumFluidProperties

!bibtex bibliography
