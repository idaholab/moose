# Water97FluidProperties

!syntax description /FluidProperties/Water97FluidProperties

The water implementation in Fluid Properties is the IAPWS Industrial Formulation 1997 for the
Thermodynamic Properties of Water and Steam. This formulation calculates properties of water and
steam using pressure and temperature as inputs. The IAPWS-IF97 formulation is split into five
different regions in the phase diagram.

All five regions are implemented in the Fluid Properties module. To avoid iteration in region 3 of
the IAPWS-IF97 formulation, the backwards equations from [!cite](iapws1997region3) are implemented.

Viscosity is calculated using the IAPWS 2008 formulation [!citep](iapws2008). Note that the critical
enhancement has not been implemented.

Thermal conductivity is calculated using the IAPS 1985 formulation [!citep](iaps1985). Although there
is a newer formulation available [!citep](iapws2011), it is significantly more complicated, so has not
been implemented yet.

Dissolution of a dilute gas into water is calculated using Henry's law [!citep](iapws2004).

## Selected constant properties of water

!table
| Property             | value |
| --- | --- |
| Molar mass           | 0.018015 kg/mol |
| Critical temperature | 647.096 K       |
| Critical pressure    | 22.064 MPa        |
| Critical density     | 322.0 kg/m$^3$ |
| Triple point temperature | 273.16 K |
| Triple point pressure | 611.657 Pa |

## Range of validity

The Water97FluidProperties UserObject is valid for:

- 273.15 K $\le$ T $\le$ 1073.15 K for p $\le$ 100 MPa
- 1073.15 K $\le$ T $\le$ 2273.15 K for p $\le$ 50 MPa

## IAPWS97 regions

The water fluid properties are calculated using different expressions depending on the pressure and temperature.
Most routines are implemented for all five regions. The regions can be seen in
Figure [!ref](regions) from [!cite](iapws97_original).
Some regions are defined explicitly using a temperature/pressure range, others are defined implicitly
using the fluid properties at a given vapor pressure or temperature.

!media large_media/fluid_properties/water97/water97_regions.png
       style=width:60%
       alt=Five regions for expressions defining the water fluid properties in IAPWS97
       id=regions


!syntax parameters /FluidProperties/Water97FluidProperties

!syntax inputs /FluidProperties/Water97FluidProperties

!syntax children /FluidProperties/Water97FluidProperties

!bibtex bibliography
