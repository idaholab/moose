# HydrogenFluidProperties

!syntax description /FluidProperties/HydrogenFluidProperties

Fluid properties for hydrogen are mainly calculated using the Leachman et al. equation of state
[!citep](leachman2009). This formulation uses density and temperature as the primary variables with
which to calculate properties such as density, enthalpy and internal energy.

When used with the pressure and temperature interface, which is the case in the Porous Flow module, hydrogen properties are typically calculated by first calculating density iteratively for a given pressure and temperature. This density is then used to calculate the other properties, such as internal energy, directly.

Viscosity is calculated using the formulation presented in [!cite](muzny2013). Thermal conductivity is calculated using the relationship presented in [!cite](assael2011)

Dissolution of hydrogen into water is calculated using Henry's law [!citep](iapws2004).

## Properties of hydrogen

!table
| Property             | value |
| --- | --- |
| Molar mass           | 0.00201588e kg/mol |
| Critical temperature | 33.19 K       |
| Critical pressure    | 1.315 MPa        |
| Critical density     | 31.262kg/m$^3$ |
| Triple point temperature | 13.952 K |
| Triple point pressure | 7.7 kPa |

## Range of validity

The HydrogenFluidProperties UserObject is valid for:

- 13.957 K $\le$ T $\le$ 1000 K for p $\le$ 2000 MPa

!syntax parameters /FluidProperties/HydrogenFluidProperties

!syntax inputs /FluidProperties/HydrogenFluidProperties

!syntax children /FluidProperties/HydrogenFluidProperties

!bibtex bibliography
