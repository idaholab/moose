# NaClFluidProperties

!syntax description /FluidProperties/NaClFluidProperties

NaCl fluid properties as a function of pressure (Pa) and temperature (K).

!alert note
Only solid state (halite) properties are currently implemented to use in brine formulation

Properties for halite given by [!cite](Driesner2007b), apart from thermal conductivity, which is
calculated using the data of [!cite](urqhart2015). Critical values are taken from [!cite](Anderko1992).

## Properties of NaCl

!table
| Property             | value |
| --- | --- |
| Molar mass           | 0.058443 kg/mol |
| Critical temperature | 3841.15 K       |
| Critical pressure    | 18.2 MPa        |
| Critical density     | 108.43 kg/m$^3$ |
| Triple point temperature | 1073.85 K |
| Triple point pressure | 50 Pa |

## Range of validity

The NaClFluidProperties UserObject is valid for the solid phase region only

!syntax parameters /FluidProperties/NaClFluidProperties

!syntax inputs /FluidProperties/NaClFluidProperties

!syntax children /FluidProperties/NaClFluidProperties

!bibtex bibliography
