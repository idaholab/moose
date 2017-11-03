# MethaneFluidProperties
!syntax description /Modules/FluidProperties/MethaneFluidProperties

Density of methane is calculated assuming an ideal gas, while all other properties are calculated using
the formulations provided in \citet{irvine1984}.

Dissolution of methane into water is calculated using Henry's law \citep{iapws2004}.

##Properties of methane

!table
| Property             | value |
| --- | --- |
| Molar mass           | 0.0160425 kg/mol |
| Critical temperature | 190.564 K       |
| Critical pressure    | 4.5992 MPa        |
| Critical density     | 162.66 kg/m$^3$ |
| Triple point temperature | 90.67 K |
| Triple point pressure | 0.01169 MPa |

##Range of validity
The MethaneFluidProperties UserObject is valid for:

- 280.0 K $\le$ T $\le$ 1080 K

!syntax parameters /Modules/FluidProperties/MethaneFluidProperties

!syntax inputs /Modules/FluidProperties/MethaneFluidProperties

!syntax children /Modules/FluidProperties/MethaneFluidProperties


## References
\bibliographystyle{unsrt}
\bibliography{fluid_properties.bib}
