# LeadLithiumFluidProperties

!syntax description /FluidProperties/LeadLithiumFluidProperties

These properties are based on experiments and compilations reported in the literature, e.g., [!cite](Schulz1991,Hubberstey1992,Giancarli1996,Zinkle1998). Most properties depend only on temperature; the fluid is considered incompressible over the range of interest. The fluid properties are summarized in Table \ref{tab:leadli}, which reports the formulas used and their origin.

!table id=tab:leadli caption=Table of properties, equations, and references for the Lead–Lithium fluid properties.
| Properties                             | Equations | Reference |
| :------------------------------------- | :-------- | :-------- |
| Melting point, $T_{mo}$ `[K]`            | $508$ | [!cite](Hubberstey1992) |
| Density, $\rho$ `[kg/m^3]`             | $\displaystyle 10520.35 - 1.19051\,T$ | [!cite](MasdeLesValls2008) |
| Viscosity, $\mu$ `[Pa s]`                | $\displaystyle 1.87 \times 10^{-4}\,\exp\!\left(\frac{11640}{8.314\,T}\right)$ | [!cite](Schulz1991) |
| Specific enthalpy, $h$ `[J/kg]`          | $\displaystyle 195\,(T-T_{mo}) - 0.5\times9.116\times10^{-3}\,(T^2-T_{mo}^2)$ | [!cite](Zinkle1998) |
| Thermal Conductivity, $k$ `[W/m-K]`      | $\displaystyle 14.51 + 0.019631\,T$ | [!cite](Mogahed1995) |
| Isobaric Specific Heat, $c_p$ `[J/(kg-K)]`  | $\displaystyle 195 - 9.116\times10^{-3}\,T$ | [!cite](Schulz1991) |
| Isochoric Specific Heat, $c_v$ `[J/(kg-K)]` | $\displaystyle \frac{c_p}{1+\left(\frac{1.19051}{10520.35-1.19051 \, T}\right)^2 \frac{B_S \, T}{\rho \, c_p}}$ | [!cite](Zinkle1998) |
| Isentropic Bulk Modulus, $B_S$ `[Pa]`     | $\displaystyle \left(44.73077 - 0.02634615 \, T + 5.76923 \times 10^{-6} \, T^2 \right) \times 10^9$ | [!cite](Hubberstey1992) |
| Speed of Sound, $c$ `[m/s]`               | $\displaystyle 1959.63 - 0.306\,T$ | [!cite](Schulz1991) |


!alert note
The thermal conductivity differs significantly between [!cite](Mogahed1995) and [!cite](MasdeLesValls2008), by nearly 50%.

## Range of Validity

The properties defined in \texttt{LeadLithiumFluidProperties} are valid for

!equation
508\,\mathrm{K} \le T \le 1800\,\mathrm{K},

and for pressures near atmospheric up to a few MPa, where the assumption of incompressibility holds.

## Uncertainties of Lead–Lithium Fluid Properties

Based on experimental studies [!cite](Schulz1991,Hubberstey1992) and compilations in fusion materials handbooks [!cite](Giancarli1996,Zinkle1998), the reported uncertainties for Lead–Lithium fluid properties are approximately:

!table id=tab:lead-std caption=Uncertainties on properties
| Property                               | Uncertainty (%) |
| :------------------------------------- | :------------- |
| Density                                | $\approx$  1% |
| Viscosity                              | $\approx$ 10% |
| Thermal Conductivity                   | $\approx$ 15% |
| Isobaric Specific Heat                 | $\approx$  7% |

!syntax parameters /FluidProperties/LeadLithiumFluidProperties

!syntax inputs /FluidProperties/LeadLithiumFluidProperties

!syntax children /FluidProperties/LeadLithiumFluidProperties

!bibtex bibliography
