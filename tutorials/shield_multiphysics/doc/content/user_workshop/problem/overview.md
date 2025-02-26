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

## Governing Equations

Concrete domain

Conservation of Energy:

!equation id=solid_energy_intro
\rho c_p \frac{\partial T}{\partial t} - \nabla\cdot k \nabla T = 0

where $\rho$ is the density, $c_p$ the specific heat capacity, $k$ the thermal conductivity, and $T$ the temperature.

!---

Water domain

Conservation of Mass (incompressible):

!equation id=fluid_mass_intro
\nabla \cdot \vec{u} = 0

Conservation of momentum:

!equation id=fluid_velocity_intro
\frac{\partial \rho  \vec{u}}{\partial t} + \nabla \cdot \left(\rho \vec{u} \otimes \vec{u}\right)
= \nabla \cdot \left(\mu_\text{eff} \left(\nabla\vec{u}_I + \left(\nabla \vec{u}_I\right)^T-\frac{2}{3}\nabla\cdot\vec{u}_I\mathbb{I}\right)\right) -\nabla p + \rho \vec{g}

Conservation of Energy:

!equation id=fluid_energy_intro
\rho c_p\left( \frac{\partial T}{\partial t} + \vec{u}\cdot\nabla T \right) - \nabla\cdot k \nabla T = 0


where $\vec{u}$ is the fluid velocity, $\mu$ is fluid viscosity, $p$ is the pressure, $\rho$ is the density, $\vec{g}$ is the gravity vector, and $T$ is the temperature.

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
