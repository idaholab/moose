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

Water region in blue, concrete in grey. 10m x 13m x 8m concrete box

!---

## Governing Equations

Concrete domain

Conservation of Energy:

!equation id=solid_energy_intro
\rho_c c_c \frac{\partial T}{\partial t} - \nabla\cdot k_c \nabla T = 0

where $\rho_c$ is the density, $c_c$ the specific heat, $k_c$ the thermal diffusivity and $T$ the temperature.

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
C\left( \frac{\partial T}{\partial t} + \vec{u}\cdot\nabla T \right) - \nabla\cdot k \nabla T = 0


where $\vec{u}$ is the fluid velocity, $\mu$ is fluid viscosity, $p$ is the pressure, $\rho$ is the density, $\vec{g}$ is the gravity vector, and $T$ is the temperature.

!---

## Material Properties

Source: Google suggested answer

| Property | Value | Units |
| :- | :- | :- |
| Density of concrete, $\rho_c$ | 2400 | $\textrm{kg}/\textrm{m}^3$ |
| Thermal conductivity of concrete, $k_c$ | 2.25 | $\textrm{W}/\textrm{m}\,\textrm{K}$ |
| Specific heat capacity of concrete, $c_p{_c}$ | 1170 | $\textrm{J}/(\textrm{kg}\,\textrm{K})$ |
| Viscosity of water, $\mu_f$ | $7.98\times10^{-4}$ |  $\textrm{P}\cdot\textrm{s}$ |
| Density of water, $\rho_f$ | 995.7 | $\textrm{kg}/\textrm{m}^3$ |
| Thermal conductivity of water, $k_f$ | 0.6 | $\textrm{W}/\textrm{m}\,\textrm{K}$ |
| Specific heat capacity of water, $c_p{_f}$ | 4181.3 | $\textrm{J}/(\textrm{kg}\,\textrm{K})$ |
