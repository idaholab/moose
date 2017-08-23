#NaClFluidProperties
!syntax description /Modules/FluidProperties/NaClFluidProperties

NaCl fluid properties as a function of pressure (Pa) and temperature (K).
!!! note
    Only solid state (halite) properties are currently implemented to use in brine formulation

Properties for halite given by \citet{Driesner2007b}, apart from thermal conductivity,
which is calculated using the data of \citet{urqhart2015}. Critical values are taken
from \citet{Anderko1992}.

##Properties of NaCl

!table
| Property             | value |
| --- | --- |
| Molar mass           | 0.058443 kg/mol |
| Critical temperature | 3841.15 K       |
| Critical pressure    | 18.2 MPa        |
| Critical density     | 108.43 kg/m$^3$ |
| Triple point temperature | 1073.85 K |
| Triple point pressure | 50 Pa |

##Range of validity
The NaClFluidProperties UserObject is valid for the solid phase region only



!syntax parameters /Modules/FluidProperties/NaClFluidProperties

!syntax inputs /Modules/FluidProperties/NaClFluidProperties

!syntax children /Modules/FluidProperties/NaClFluidProperties

## References
\bibliographystyle{unsrt}
\bibliography{fluid_properties.bib}
