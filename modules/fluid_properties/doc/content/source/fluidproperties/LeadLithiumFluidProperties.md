# LeadLithiumFluidProperties

!syntax description /FluidProperties/LeadLithiumFluidProperties

These properties are based on experiments and compilations reported in the literature, e.g., [!cite](Schulz1991,Zinkle1998,MasdeLesValls2008). Most properties depend only on temperature; the fluid is considered incompressible over the range of interest. The fluid properties are summarized in Table [tab:leadli], which reports the formulas used and their origin.

!table id=tab:leadli caption=Table of properties, equations, and references for the Lead–Lithium fluid properties.
| Properties                             | Equations | Reference |
| :------------------------------------- | :-------- | :-------- |
| Melting point, $T_{mo}$ `[K]`          | $508$ | [!cite](Martelli2009,MasdeLesValls2008) |
| Density, $\rho$ `[kg/m^3]`             | $\displaystyle 10520.35 - 1.19051\,T$ | [!cite](MasdeLesValls2008) |
| Viscosity, $\mu$ `[Pa s]`              | $\displaystyle 1.87 \times 10^{-4}\,\exp\!\left(\frac{11640}{8.314\,T}\right)$ | [!cite](Schulz1991) |
| Specific enthalpy, $h$ `[J/kg]`        | $\displaystyle 195\,(T-T_{mo}) - 0.5\times9.116\times10^{-3}\,(T^2-T_{mo}^2)$ | [!cite](Zinkle1998) |
| Thermal Conductivity, $k$ `[W/(m-K)]`       | $\displaystyle 14.51 + 0.019631\,T$ | [!cite](Mogahed1995) |
| Isobaric Specific Heat, $c_p$ `[J/(kg-K)]`  | $\displaystyle 195 - 9.116\times10^{-3}\,T$ | [!cite](Schulz1991) |
| Isochoric Specific Heat, $c_v$ `[J/(kg-K)]` | $\displaystyle \frac{c_p}{1+\left(\frac{1.19051}{10520.35-1.19051 \, T}\right)^2 \frac{B_S \, T}{\rho \, c_p}}$ | [!cite](Zinkle1998) |
| Isentropic Bulk Modulus, $B_S$ `[Pa]`       | $\dfrac{c^2}{\rho} | [!cite](Martelli2009)|
| Speed of Sound, $c$ `[m/s]`                 | $\displaystyle 1876 - 0.306\,T$ | [!cite](Ueki2009) |


!alert note
The thermal conductivity differs significantly between [!cite](Mogahed1995) and [!cite](MasdeLesValls2008,Zinkle1998,Schulz1991), by nearly 50%.

## Range of Validity

The properties defined in `LeadLithiumFluidProperties` are valid in the following temperature ranges:

!equation
508\,\mathrm{K} \le T \le 880\,\mathrm{K}, \text{for density}

!equation
508\,\mathrm{K} \le T \le 873\,\mathrm{K}, \text{for thermal conductivity}

!equation
508\,\mathrm{K} \le T \le 625\,\mathrm{K}, \text{for specific heat}

!equation
508\,\mathrm{K} \le T \le 625\,\mathrm{K}, \text{for viscosity}


and for pressures near atmospheric up to a few MPa, where the assumption of incompressibility holds.

## Uncertainties of Lead–Lithium Fluid Properties

Based on experimental studies [!cite](Schulz1991) and compilations in fusion materials handbooks [!cite](Zinkle1998), and thermophysical property reviews [!cite](MasdeLesValls2008), the reported uncertainties for Lead–Lithium fluid properties are approximately:

!table id=tab:lead-std caption=Uncertainties on properties
| Property                               | Uncertainty / Scattering (%) |
| :------------------------------------- | :------------- |
| Density                                | $\approx$  4%  [!cite](Martelli2009) |
| Viscosity                              | $\approx$ 12%  [!cite](Martelli2009) |
| Thermal Conductivity                   | $\approx$ 37%  [!cite](Martelli2009) |
| Isobaric Specific Heat                 | $\approx$  32% [!cite](Martelli2009) |

!syntax parameters /FluidProperties/LeadLithiumFluidProperties

!syntax inputs /FluidProperties/LeadLithiumFluidProperties

!syntax children /FluidProperties/LeadLithiumFluidProperties

!bibtex bibliography
