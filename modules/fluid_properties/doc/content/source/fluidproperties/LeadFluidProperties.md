# LeadFluidProperties

!syntax description /FluidProperties/LeadFluidProperties

These properties are based on experiments reported in the Handbook on Lead-bismuth Eutectic Alloy and Lead
Properties, Materials Compatibility, Thermal-hydraulics and Technologies [!citep](Fazio).
Most properties only depend on temperature; the fluid is considered incompressible.
The fluid properties are summarized in Table [tab:lead], which reports the formulas used and their origin. 

!table id=tab:lead caption=Table of properties and references to the equations in [!citep](Fazio).
| Properties                     | Equation       | Equation/Table # |
| :----------------------------- | :------------- | :------------- |
| Melting point, $T_{mo}$ (K)    | 600            | Equation 2.1  |
| Density, $\rho$ (kg/m^3)       | $11441 - 1.2795T$ | Equation 2.28  |
| Viscosity, $\mu$ (Pa-s)        | $4.55 \times 10^{-4} \times e^{\frac{1069}{T}}$ | Equation 2.81  |
| Specific enthalpy, $h$ (J)     | $176.2(T - T_{mo}) - 2.4615 \times 10^{-2}(T^2 - T_{mo}^2) + 5.147 \times 10^6(T^3 - T_{mo}^3) + 1.524 \times 10^6(\frac{1}{T} -\frac{1}{T_{mo}})$ | Equation 2.53  |
| Thermal Conductivity, $k$ (W/m-K)        | $9.2 + 0.011T$ | Equation 2.89  |
| Isobaric Specific Heat, $cp$ (J/kg-K)    | $176.2 - 4.923\times 10^{-2} \times T + 1.544 \times 10^{-5} \times T^2 - \frac{1.524 \times 10^6}{T^2}$ |  Table 2.19.1  |
| Isentropic Bulk Modulus, $B_S$ (N/m$^2$) | $(43.50 - 1.552 \times 10^{-2}T + 1.622 \times 10^{-6}T^2)10^9$ | Equation 2.42  |
| Speed of Sound, $c$ (m/s)                | $1953 - 0.246 T$ | Table 2.19.1 |

## Range of validity

The properties defined by `LeadFluidProperties` are valid for:

 - 600 K $\le$ T $\le$ 1800 K

## Uncertainties of Lead Fluid Properties

The reported uncertainties in [!citep](Fazio) for lead fluid properties are:

| Properties | Uncertainties |
| :------------- | :------------- |
| Density | 1% |
| Viscosity | 5% |
| Thermal Conductivity | 15% |
| Isobaric Specific Heat | 5% |

!syntax parameters /FluidProperties/LeadFluidProperties

!syntax inputs /FluidProperties/LeadFluidProperties

!syntax children /FluidProperties/LeadFluidProperties

!bibtex bibliography
