# CO2FluidProperties

!syntax description /FluidProperties/CO2FluidProperties

Fluid properties for CO$_2$ are mainly calculated using the Span and Wagner equation of state
[!citep](spanwagner1996). This formulation uses density and temperature as the primary variables with
which to calculate properties such as density, enthalpy and internal energy. However, the Fluid
Properties module uses pressure and temperature in its interface, which is suitable for use in the
Porous Flow module. As a result, CO$_2$ properties are typically calculated by first calculating
density iteratively for a given pressure and temperature. This density is then used to calculate the
other properties, such as internal energy, directly.

Viscosity is calculated using the formulation presented in [!cite](fenghour1998), while
thermal conductivity is taken from [!cite](scalabrin2006).

Dissolution of CO$_2$ into water is calculated using Henry's law [!citep](iapws2004).

## Properties of CO$_2$

!table
| Property             | value |
| --- | --- |
| Molar mass           | 0.0440098 kg/mol |
| Critical temperature | 304.1282 K       |
| Critical pressure    | 7.3773 MPa        |
| Critical density     | 467.6 kg/m$^3$ |
| Triple point temperature | 216.592 K |
| Triple point pressure | 0.51795 MPa |

## Range of validity

The CO2FluidProperties UserObject is valid for:

- 216.592 K $\le$ T $\le$ 1100 K for p $\le$ 800 MPa

!syntax parameters /FluidProperties/CO2FluidProperties

!syntax inputs /FluidProperties/CO2FluidProperties

!syntax children /FluidProperties/CO2FluidProperties

!bibtex bibliography
