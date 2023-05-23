# NaKFluidProperties

!syntax description /FluidProperties/NaKFluidProperties

These properties are based on experiments reported in the Handbook on NaK [!citep](NaKHandbook).
Most properties only depend on temperature; the fluid is considered incompressible.
The fluid properties are summarized in Table [tab:NaK], which reports the formulas used and their origin.

!table id=tab:NaK caption=Table of properties and references to the equations in [!citep](NaKHandbook).
| Properties                     | Equation       | Equation # |
| :----------------------------- | :------------- | :------------- |
| Density liquid Na, $\rho_{Na}$ (kg/m^3)       | $0.9453 - 2.2473e-4 * T_c$ ($T_c$ in C) | Equation 1.5 |
| Density liquid K, $\rho_K$ (kg/m^3)       | $0.8415 - 2.172e-4 * T_c - 2.7e-8 * T_c^2 + 4.77e-12 * T_c^3$ ($T_c$ in C) | Equation 1.8  |
| Density liquid NaK, $\rho$ (kg/m^3)       | $1 / (N_K / \rho_K + N_{Na} / \rho_{Na})$ | Equation 1.9 |
| Viscosity, $\mu$ (Pa-s)        | See handbook | Equation 1.18 - 1.19  |
| Thermal Conductivity, $k$ (W/m-K)        | $0.214 + 2.07e-4 * T_c - 2.2e-7 * T_c^2$ ($T_c$ in C) | Equation 1.53  |
| Isobaric Specific Heat, $cp$ (J/kg-K)    | $0.2320 - 8.82e-5 * T_c + 8.23-8 * T_c^2$ ($T_c$ in C) | Equation 1.59 |


## Range of validity

The reported range of validity is specified for each property:

- Density liquid Na 210 C $\le$ T $\le$ 1100 C
- Density liquid K 63.2 C $\le$ T $\le$ 1250 C
- Viscosity NaK 100 C $\le$ T
- Thermal conductivity 150 C $\le$ T $\le$ 680 C
- Isobaric Specific Heat: unspecified. Measured from 0 to 800 C


## Uncertainties of NaK Fluid Properties

The **reported** uncertainties in [!citep](NaKHandbook) for NaK fluid properties are reported in Table [tab:uncertainty].

!alert note
Many `NaK` properties are computed from a mix of `Na` and `K` properties and the uncertainty is only reported for these individual properties. We report it here, and the user will have to perform uncertainty propagation to obtain the uncertainties for the eutectic.

!table id=tab:uncertainty caption=Uncertainties reported in [!citep](NaKHandbook).
| Properties | Uncertainties |
| :------------- | :------------- |
| Density liquid Na | 0.14-0.18% |
| Density liquid K | 0.25% |
| Viscosity liquid Na | 3.5e-6 - 6.5e-6 Pa.s |
| Viscosity liquid K | 7e-6 - 1.7e-5 Pa.s |
| Thermal Conductivity | 0.8% |
| Isobaric Specific Heat Na | 0.4% |
| Isobaric Specific Heat K | 2-5% |

!syntax parameters /FluidProperties/NaKFluidProperties

!syntax inputs /FluidProperties/NaKFluidProperties

!syntax children /FluidProperties/NaKFluidProperties

!bibtex bibliography
