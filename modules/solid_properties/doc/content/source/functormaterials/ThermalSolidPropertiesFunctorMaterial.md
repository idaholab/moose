# ThermalSolidPropertiesFunctorMaterial

## Description

The `ThermalSolidPropertiesFunctorMaterial` material declares
density, specific heat, and thermal
conductivity as functor material properties. They are evaluated on-the-fly
on every query.

!alert warning title=Conservation on fixed-sized domains
Using a variable density can lead to mass/energy conservation errors if using
a fixed-size domain. If this is a concern, it is recommended to use
a constant density, specified independently using a [GenericFunctorMaterial.md] for example.
For the density parameter of `ThermalSolidPropertiesFunctorMaterial` you may then use a
placeholder dummy name.

!syntax parameters /FunctorMaterials/ThermalSolidPropertiesFunctorMaterial

!syntax inputs /FunctorMaterials/ThermalSolidPropertiesFunctorMaterial

!syntax children /FunctorMaterials/ThermalSolidPropertiesFunctorMaterial
