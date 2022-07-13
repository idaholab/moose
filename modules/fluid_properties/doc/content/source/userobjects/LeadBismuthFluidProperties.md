# LeadBismuthFluidProperties

!syntax description /FluidProperties/LeadBismuthFluidProperties

Formulation for Molten Lead that is heated above 398 Kelvin and is below 1300 Kelvin, based off prior experiments reported in the Handbook on Lead-bismuth Eutectic Alloy and Lead Properties, Materials Compatibility, Thermal- hydraulics and Technologies [!citep](Fazio).

Density, enthalpy, internal energy, viscosity, thermal conductivity and isobaric specific heat.
calculated using the formulations provided in [!citep](Fazio).

The formulations used from the handbook are all based off of pressure and temperature dependence,
with only using temperature as a variable in its calculations.

The handbook also assumes that isochoric specific heat is the same as isobaric specific heat,
as the fluid is considered incompressible.

## Uncertainties of Lead Fluid Properties

Table for LeadBismuthFluidProperties Uncertainties:


| Properties | Range/ Uncertainties|
| :------------- | :------------- |
| Density | .8% |
| Viscosity | 8% |
| Thermal Conductivity | 15% |
| Isobaric Specific Heat | 7% |

## Properties Associated Equations and Figure #  

Table of properties, equations and citation of where in [!citep](Fazio).

| Properties | Equations| Figure # |
| :------------- | :------------- | :------------- |
| Melting point $(T_{mo}, K)$ | 398 |  2.1  |
| Boilling Point (K) | 1300 |  6.2.1  |
| Density $(\rho, kg/m^3)$ | $11065 - 1.2793T$ |  2.30  |
| Viscosity $(\mu, Pa \times s)$ | $4.94 \times 10^{-4} \times e^{\frac{754.1}{temperature}}$ |  2.83  |
| Specific Volume (v, m^3) | $\frac{1}{\rho}$ |  NA  |
| Specific enthalpy (h, J) | $164.8(T - T_{mo}) - 1.97 \times 10^{-2}(T^2 - T_{mo}^2) + 4.167 \times 10^6(T^3 - T_{mo}^3) + 4.56 \times 10^5(\frac{1}{T} -\frac{1}{T_{mo}})$ |  2.55  |
| Thermal Conductivity $(k, \frac {W}{m * K})$ | $3.284 + 1.617 \times 10^{-2}T - 2.305 \times 10^{-6}T^2$ |  2.91  |
| Isobaric Specific Heat $(cp, \frac{J}{kg * K})$ | $164.8 - 3.94 * 10^{-2}T + 1.25 \times 10^{-5}T^2 - \frac{4.56 * 10^5}{T^2}$ |  Table 2.19.3  |
| Bulk Modules $(\frac{N}{m^2})$ | $(38.02 - 1.296 \times 10^{-2}T + 1.320 \times 10^{-6}T^2)10^9$ |  2.44  |
| Speed of Sound  (c) | $\sqrt{\frac {Bulk Modules}{\rho}}$ |  NA  |

## Range of validity

The LeadFluidProperties UserObject is valid for:

 698 K $\le$ T $\le$ 1300 K

!syntax parameters /FluidProperties/LeadBismuthFluidProperties

!syntax inputs /FluidProperties/LeadBismuthFluidProperties

!syntax children /FluidProperties/LeadBismuthFluidProperties

!bibtex bibliography
