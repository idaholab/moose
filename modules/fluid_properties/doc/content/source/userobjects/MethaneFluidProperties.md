# MethaneFluidProperties

!syntax description /FluidProperties/MethaneFluidProperties

Fluid properties for methane are mainly calculated using the Setzmann and Wagner equation of state
[!citep](setzmann1991). This formulation uses density and temperature as the primary variables with
which to calculate properties such as density, enthalpy and internal energy.

When used with the pressure and temperature interface, which is the case in the Porous Flow module, methane properties are typically calculated by first calculating density iteratively for a given pressure and temperature. This density is then used to calculate the other properties, such as internal energy, directly. The computational expense associated with the iterative calculation can be mitigated using [TabulatedFluidProperties](/TabulatedFluidProperties.md).

For low pressures (typically less than 10 MPa), the properties of methane can be approximated using an [ideal gas](/IdealGasFluidProperties.md), which are much faster to calculate. However, at higher pressures, this approximation can lead to large differences, see [methane_density].

!media media/fluid_properties/methane_density.png
       id=methane_density
       style=width:60%;margin-left:10px;
       caption=Methane density at 350K for various pressures.

Transport properties such as viscosity and thermal conductivity are calculated using the formulations provided in [!cite](irvine1984).

Dissolution of methane into water is calculated using Henry's law [!citep](iapws2004).

## Properties of methane

!table
| Property             | value |
| --- | --- |
| Molar mass           | 0.0160425 kg/mol |
| Critical temperature | 190.564 K       |
| Critical pressure    | 4.5992 MPa        |
| Critical density     | 162.66 kg/m$^3$ |
| Triple point temperature | 90.6941 K |
| Triple point pressure | 0.01169 MPa |

## Range of validity

The MethaneFluidProperties UserObject is valid for:

- 90.69 K $\le$ T $\le$ 625 K

and pressures up to 100 MPa.

!syntax parameters /FluidProperties/MethaneFluidProperties

!syntax inputs /FluidProperties/MethaneFluidProperties

!syntax children /FluidProperties/MethaneFluidProperties

!bibtex bibliography
