# ConstantDensityThermalSolidPropertiesMaterial

## Description

This material is the same as [ThermalSolidPropertiesMaterial.md], except that
it uses a constant density, evaluated at a user-given reference temperature.

This is useful because when using a fixed-size domain with conservation equations
such as transient heat conduction, conservation errors result from using a variable
density, since by changing density without changing the domain volume, the
mass (and thus thermal energy, for example) are changed, which violates the
governing conservation laws.

!syntax parameters /Materials/ConstantDensityThermalSolidPropertiesMaterial

!syntax inputs /Materials/ConstantDensityThermalSolidPropertiesMaterial

!syntax children /Materials/ConstantDensityThermalSolidPropertiesMaterial

!bibtex bibliography
