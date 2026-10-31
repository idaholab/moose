# LithiumFluidProperties

!syntax description /FluidProperties/LithiumFluidProperties

These properties are the empirical correlations for saturated liquid lithium compiled by NASA
Lewis Research Center for high-temperature space nuclear power systems [!citep](Davison1968).
All properties only depend on temperature; the fluid is considered incompressible.
The fluid properties are summarized in [tab:lithium], which reports the formulas used and their
equation numbers in [!citep](Davison1968). The temperature $T$ is in K.

!table id=tab:lithium caption=Table of properties and references to the equations in [!citep](Davison1968).
| Properties | Equation | Equation # |
| :--------- | :------- | :--------- |
| Density, $\rho$ (kg/m^3) | $562 - 0.100\,T$ | Equation 1 |
| Specific enthalpy relative to solid lithium at 273 K, $h$ (J/kg) | $-7.519 \times 10^{5} + 4169\,T$ | Equation 3 |
| Isobaric specific heat, $c_p$ (J/kg-K) | $4169$ | Equation 5 |
| Thermal conductivity, $k$ (W/m-K) | $21.874 + 0.056255\,T - 1.8325 \times 10^{-5}\,T^2$ | Polynomial fit of Equation 8, page 11 |
| Dynamic viscosity, $\mu$ (Pa-s) | $10^{\,-3.080 + 57.63/T - 5.172 \times 10^{-4}\,T}$ | Equation 10 |

The isochoric specific heat is taken equal to the isobaric specific heat, as is conventional
for an incompressible liquid.

## Range of validity

The correlations were fitted to data between the melting point and about 1000 K, and
extrapolated to about 1600 K, close to the normal boiling point (1608 K) [!citep](Davison1968).
An invalid solution is flagged outside of

!equation
453.7\,\mathrm{K} \le T \le 1600\,\mathrm{K}

## Uncertainties of lithium fluid properties

The deviations between the correlations and the experimental data reported in
[!citep](Davison1968) are summarized in [tab:uncertainty].

!table id=tab:uncertainty caption=Deviations reported in [!citep](Davison1968).
| Property | Standard deviation | Maximum deviation |
| :------- | :----------------- | :---------------- |
| Density | 0.7% | 2.4% |
| Enthalpy | 1.6% | 5.8% |
| Thermal conductivity (Equation 8) | 5.6% | 8.3% |
| Dynamic viscosity | 19% | 29% |

!alert note
The polynomial used for the thermal conductivity is within 3% of Equation 8 of
[!citep](Davison1968). The heat capacity is the derivative of the linear enthalpy correlation;
the measured heat capacities show considerable scatter and quadratic or cubic enthalpy fits
give heat capacities up to 11% different at 1600 K.

!syntax parameters /FluidProperties/LithiumFluidProperties

!syntax inputs /FluidProperties/LithiumFluidProperties

!syntax children /FluidProperties/LithiumFluidProperties

!bibtex bibliography
