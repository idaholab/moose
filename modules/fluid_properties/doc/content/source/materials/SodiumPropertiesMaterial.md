# Sodium Properties Material

!syntax description /Materials/SodiumPropertiesMaterial

`SodiumPropertiesMaterial` samples the functions available in [SodiumProperties](SodiumProperties.md)
and provides them as material properties.

The following material properties created for liquid sodium are:

- Thermal Conductivity: `k(temperature)`
- Enthalpy: `h(temperature)`
- Heat capacity: `cp(temperature)`
- Temperature from enthalpy: `T_from_h(h)`
- Density: `rho(temperature)`
- Derivative of Density w.r.t temperature: `drho_dT(temperature)`
- Derivative of Density w.r.t enthalpy: `drho_dh(h)`

Properties of liquid sodium are obtained from [!cite](Fink:1995bf).

!listing modules/fluid_properties/test/tests/sodium/exact.i block=Materials

!syntax parameters /Materials/SodiumPropertiesMaterial

!syntax inputs /Materials/SodiumPropertiesMaterial

!syntax children /Materials/SodiumPropertiesMaterial

!bibtex bibliography
