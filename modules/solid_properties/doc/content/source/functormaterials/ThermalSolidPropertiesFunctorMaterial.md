# ThermalSolidPropertiesFunctorMaterial

## Description

`ThermalSolidPropertiesFunctorMaterial` declares functor material properties for
density, specific heat, and thermal conductivity, and specific internal energy.

The parameter [!param](/FunctorMaterials/ThermalSolidPropertiesFunctorMaterial/use_constant_density)
can be used to specify that the density should be constant, evaluated at the temperature
[!param](/FunctorMaterials/ThermalSolidPropertiesFunctorMaterial/T_ref). This is
useful because for fixed-sized domains, mass/energy conservation errors result
from using a variable density.

!syntax parameters /FunctorMaterials/ThermalSolidPropertiesFunctorMaterial

!syntax inputs /FunctorMaterials/ThermalSolidPropertiesFunctorMaterial

!syntax children /FunctorMaterials/ThermalSolidPropertiesFunctorMaterial
