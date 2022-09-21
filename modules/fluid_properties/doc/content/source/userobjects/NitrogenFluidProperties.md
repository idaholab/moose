# NitrogenFluidProperties

!syntax description /FluidProperties/NitrogenFluidProperties

Fluid properties for nitrogen are mainly calculated using the Span et al. equation of state
[!citep](span2000). This formulation uses density and temperature as the primary variables with
which to calculate properties such as density, enthalpy and internal energy.

When used with the pressure and temperature interface, which is the case in the Porous Flow module, nitrogen properties are typically calculated by first calculating density iteratively for a given pressure and temperature. This density is then used to calculate the other properties, such as internal energy, directly.

Viscosity and thermal conductivity are calculated using the formulation presented in [!cite](lemmon2004).

Dissolution of nitrogen into water is calculated using Henry's law [!citep](iapws2004).

## Properties of nitrogen

!table
| Property             | value |
| --- | --- |
| Molar mass           | 0.02801348 kg/mol |
| Critical temperature | 126.192 K       |
| Critical pressure    | 3.3958 MPa        |
| Critical density     | 313.3 kg/m$^3$ |
| Triple point temperature | 63.151 K |
| Triple point pressure | 12.523 kPa |

## Range of validity

The NitrogenFluidProperties UserObject is valid for:

- 63.151 K $\le$ T $\le$ 1000 K for p $\le$ 2200 MPa

!syntax parameters /FluidProperties/NitrogenFluidProperties

!syntax inputs /FluidProperties/NitrogenFluidProperties

!syntax children /FluidProperties/NitrogenFluidProperties

!bibtex bibliography
