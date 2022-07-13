# LeadFluidProperties

!syntax description /FluidProperties/LeadFluidProperties

 Formulation for Molten Lead that is heated above 600 Kelvin and is below 1800 Kelvin, based off prior experiments reported in the Handbook on Lead-bismuth Eutectic Alloy and Lead Properties, Materials Compatibility, Thermal- hydraulics and Technologies [!citep](Fazio).

Density, enthalpy, internal energy, viscosity, thermal conductivity and isobaric specific heat.
calculated using the formulations provided in [!citep](Fazio).

The formulations used from the handbook are all based off of pressure and temperature dependence,
with only using temperature as a variable in its calculations.

The handbook also assumes that isochoric specific heat is the same as isobaric specific heat,
as the fluid is considered incompressible.

## Uncertainties of Lead Fluid Properties

Table for LeadFluidProperties Uncertainties:

| Properties | Uncertainties|
| :------------- | :------------- |
| Density | 1% |
| Viscosity | 5% |
| Thermal Conductivity | 15% |
| Isobaric Specific Heat | 5% |

## Properties Associated Equations and Reference

Table of properties, equations and citation of where in [!citep](Fazio).

| Properties | Equations| Figure # |
| :------------- | :------------- | :------------- |
| Melting point $(T_{mo}, K)$ | 600 |  2.1  |
| Boilling Point (K) | 1750 |  6.2.1  |
| Density $(\rho, kg/m^3)$ | $11441 - 1.2795T$ |  2.28  |
| Viscosity $(\mu, Pa \times s)$ | $4.55 \times 10^{-4} \times e^{\frac{1069}{temperature}}$ |  2.81  |
| Specific Volume (v, m^3) | $\frac{1}{\rho}$ |  NA  |
| Specific enthalpy (h, J) | $176.2(T - T_{mo}) - 2.4615 \times 10^{-2}(T^2 - T_{mo}^2) + 5.147 \times 10^6(T^3 - T_{mo}^3) + 1.524 \times 10^6(\frac{1}{T} -\frac{1}{T_{mo}})$ |  2.53  |
| Thermal Conductivity $(k, \frac {W}{m * K})$ | $9.2 + 0.011T$ |  2.89  |
| Isobaric Specific Heat $(cp, \frac{J}{kg * K})$ | $176.2 - 4.923* 10^{-2} \times T + 1.544 \times 10^-5 \times T^2 - \frac{1.524 * 10^6}{T^2}$ |  Table 2.19.1  |
| Bulk Modules $(\frac{N}{m^2})$ | $(43.50 - 1.552 \times 10^{-2}T + 1.622 \times 10^{-6}T^2)10^9$ |  2.42  |
| Speed of Sound  (c) | $\sqrt{\frac {Bulk Modules}{\rho}}$ |  NA  |

## Range of validity

The properties defined by `LeadFluidProperties` are valid for:

 - 600 K $\le$ T $\le$ 1800 K

!syntax parameters /FluidProperties/LeadFluidProperties

!syntax inputs /FluidProperties/LeadFluidProperties

!syntax children /FluidProperties/LeadFluidProperties

!bibtex bibliography
