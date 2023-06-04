# LeadBismuthFluidProperties

!syntax description /FluidProperties/LeadBismuthFluidProperties

These properties are based on experiments reported in the Handbook on Lead-bismuth Eutectic Alloy and Lead
Properties, Materials Compatibility, Thermal-hydraulics and Technologies [!citep](Fazio). 
Most properties only depend on temperature; the fluid is considered incompressible.
The fluid properties are summarized in Table [tab:leadbi], which reports the formulas used and their origin. 

!table id=tab:leadbi caption=Table of properties and references to the equations in [!citep](Fazio).
| Properties                     | Equations| Equation/Table # |
| :----------------------------- | :------------- | :------------- |
| Melting point, $T_{mo}$ (K)    | 398 | Equation 2.1  |
| Density, $\rho$ (kg/m^3)       | $11065 - 1.2793T$ | Equation 2.30  |
| Viscosity, $\mu$ (Pa-s)        | $4.94 \times 10^{-4} \times e^{\frac{754.1}{T}}$ | Equation 2.83  |
| Specific enthalpy, $h$ (J)     | $164.8(T - T_{mo}) - 1.97 \times 10^{-2}(T^2 - T_{mo}^2) + 4.167 \times 10^6(T^3 - T_{mo}^3) + 4.56 \times 10^5(\frac{1}{T} -\frac{1}{T_{mo}})$ | Equation 2.55  |
| Thermal Conductivity, $k$ (W/m-K)        | $3.284 + 1.617 \times 10^{-2}T - 2.305 \times 10^{-6}T^2$ | Equation 2.91  |
| Isobaric Specific Heat, $cp$ (J/kg-K)    | $164.8 - 3.94 \times 10^{-2}T + 1.25 \times 10^{-5}T^2 - \frac{4.56 \times 10^5}{T^2}$ |  Table 2.19.3  |
| Isentropic Bulk Modulus, $B_S$ (N/m$^2$) | $(38.02 - 1.296 \times 10^{-2}T + 1.320 \times 10^{-6}T^2)10^9$ | Equation 2.44  |
| Speed of Sound, $c$ (m/s)                | $1855 - 0.212 T$ | Table 2.19.3 |

## Range of validity

The properties defined in `LeadBismuthFluidProperties` from [!citep](Fazio) are valid for:

400 K $\le$ T $\le$ 1100 K

## Uncertainties of Lead Bismuth Eutectic Fluid Properties

The reported uncertainties in [!citep](Fazio) for lead-bismuth eutectic fluid properties are:

| Properties     | Range/ Uncertainties|
| :------------- | :------------- |
| Density        | 0.8% |
| Viscosity      |  8% |
| Thermal Conductivity | 15% |
| Isobaric Specific Heat | 7% |

!syntax parameters /FluidProperties/LeadBismuthFluidProperties

!syntax inputs /FluidProperties/LeadBismuthFluidProperties

!syntax children /FluidProperties/LeadBismuthFluidProperties

!bibtex bibliography
