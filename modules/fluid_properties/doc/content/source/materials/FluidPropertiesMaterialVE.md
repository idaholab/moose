# FluidPropertiesMaterialVE

!syntax description /Materials/FluidPropertiesMaterialVE

`FluidPropertiesMaterialVE` samples the functions available in a fluid property user object
and provides them as material properties.

The following material properties created are:

- Density: `rho(specific_volume, specific_energy)`
- Dynamic viscosity: `mu(specific_volume, specific_energy)`
- Isobaric specific heat capacity: `cp(specific_volume, specific_energy)`
- Isochoric specific heat capacity: `cv(specific_volume, specific_energy)`
- Thermal conductivity: `k(specific_volume, specific_energy)`
- Velocity of sound in medium: `c(specific_volume, specific_energy)`


The following variables are also computed through a variable set inversion:

- Pressure
- Temperature


!syntax parameters /Materials/FluidPropertiesMaterialVE

!syntax inputs /Materials/FluidPropertiesMaterialVE

!syntax children /Materials/FluidPropertiesMaterialVE
