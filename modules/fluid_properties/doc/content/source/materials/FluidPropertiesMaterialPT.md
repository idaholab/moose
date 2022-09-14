# FluidPropertiesMaterialPT

!syntax description /Materials/FluidPropertiesMaterialPT

`FluidPropertiesMaterialPT` samples the functions available in a fluid property user object
and provides them as material properties.

The following material properties created are:

- Density: `rho(temperature, pressure)`
- Dynamic viscosity: `mu(temperature, pressure)`
- Isobaric specific heat capacity: `cp(temperature, pressure)`
- Isochoric specific heat capacity: `cv(temperature, pressure)`
- Thermal conductivity: `k(temperature, pressure)`
- Specific enthalpy: `h(temperature, pressure)`
- Specific energy: `e(temperature, pressure)`
- Specific entropy: `s(temperature, pressure)`
- Velocity of sound in medium: `c(temperature, pressure)`

!syntax parameters /Materials/FluidPropertiesMaterialPT

!syntax inputs /Materials/FluidPropertiesMaterialPT

!syntax children /Materials/FluidPropertiesMaterialPT
