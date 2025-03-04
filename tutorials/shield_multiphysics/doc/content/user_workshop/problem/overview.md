# Problem Statement

!---

We will study the cooling of the concrete shielding around a future micro-reactor in the DOME Test Bed at the National Reactor
Innovation Center in Idaho.

!media shield_multiphysics/both.png style=width:80%;margin-left:auto;margin-right:auto;display:block; caption=Exterior (left), Interior (right)

This example was created independently from studies at NRIC and INL. The dimensions have been modified
and numerous systems and complexities are omitted.

!---

Cooling system schematic (from NRIC overview Nov. 21)

!media shield_multiphysics/schematic.png style=width:70%;margin-left:auto;margin-right:auto;display:block;

!---

Simplified geometry:

!media shield_multiphysics/shield.png style=width:60%;margin-left:auto;margin-right:auto;display:block;

6.5m x 9.7m x 5.25m concrete box with 4m x 7.6m x 3.6m room

!---

## Physics of Interest

- Concrete Domain: +Thermal Mechanics+

  - Heat conduction of the thermal radiation from the reactor to the boundary of shield.
  - Mechanical displacement and stress from the thermal expansion.

- Water Domain: +Thermal Fluids+

  - Heat transfer through the fluid and back-and-forth from the concrete.
  - Natural convection of the fluid due to the temperature gradients.

!---

## Material Properties

| Property | Units | Magnetite Concrete | Ordinary Concrete | Aluminum | Water |
| :- | :- | -: | -: | -: | -: |
| Thermal conductivity, $k$ | W/(mK) | 5.0 | 2.25 | 175 | 0.6 |
| Density, $\rho$ | kg/m$^3$ | 3,524 | 2,403 | 2,270 | 955.7 |
| Heat capacity, $c_p$ | J/(kgK) | 1,050 | 1,050 | 875 | 4,181 |
| Viscosity, $\mu$ | mPa$\cdot$s | --- | --- | --- | 0.798 |
| Water heat transfer coefficient | W/m$^2$ K | 600 | 600 | 600 | --- |
| Air heat transfer coefficient | W/m$^2$ K | 10 | --- | --- | --- |
| Young's modulus | GPa | 2.75 | 30 | 68 | --- |
| Poisson's ratio | --- | 0.15 | 0.2 | 0.36 |
| Thermal expansion coefficient | 10$^\text{-5}$/K | 1.0 | 1.0 | 2.4 | --- |
