# Sodium Fluid Properties
!syntax description /Modules/FluidProperties/SodiumProperties

SodiumProperties inherits from the base class FluidProperties and provides the needed functionality for determining sodium
fluid properties that may be used for coolant channel simulations.

The following methods are provided in this class:

* Thermal Conductivity: `k(temperature)`
* Enthalpy: `h(temperature)`
* Heat capacity: `heatCapacity(temperature)`
* Temperature: `temperature(h)`
* Density: `rho(temperature)`
* Derivative of Density w.r.t temperature: `drho_dT(temperature)`
* Derivative of Density w.r.t enthalpy: `drho_dh(h)`

Properties of liquid sodium are obtained from ANL/RE-95/2 report "Thermodynamic and Transport Properties of
Sodium Liquid and Vapor" from ANL Reactor Engineering Division.

!syntax parameters /Modules/FluidProperties/SodiumProperties

!syntax inputs /Modules/FluidProperties/SodiumProperties

!syntax children /Modules/FluidProperties/SodiumProperties
