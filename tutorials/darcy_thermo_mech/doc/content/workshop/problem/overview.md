# Problem Statement

!---

Consider a system containing two pressure vessels at differing temperatures. The vessels are
connected via a pipe that contains a filter consisting of close-packed steel spheres. Predict the
velocity and temperature of the fluid inside the filter. The pipe is 0.304 m in length and 0.0514 m
in diameter.

!media darcy_thermo_mech/problem_schematic.png style=width:60%;margin-left:auto;margin-right:auto;display:block;

!style fontsize=large
Pamuk and Ozdemir, [*"Friction factor, permeability, and inertial coefficient of oscillating flow through porous media of packed balls"*](https://www.sciencedirect.com/science/article/pii/S0894177711002640), Experimental Thermal and Fluid Science, v. 38, pp. 134-139, 2012.

!---

## Governing Equations

Conservation of Mass:

!equation id=darcy_mass
\nabla \cdot \vec{u} = 0

Conservation of Energy:

!equation id=darcy_energy
C\left( \frac{\partial T}{\partial t} + \epsilon \vec{u}\cdot\nabla T \right) - \nabla\cdot k \nabla T = 0

Darcy's Law:

!equation id=darcy_velocity
\vec{u} = -\frac{\mathbf{K}}{\mu} (\nabla p - \rho \vec{g})

where $\vec{u}$ is the fluid velocity, $\epsilon$ is porosity, $\mathbf{K}$ is the permeability
tensor, $\mu$ is fluid viscosity, $p$ is the pressure, $\rho$ is the density, $\c_p$ is the specific heat, $\vec{g}$ is the
gravity vector, and $T$ is the temperature.

!---

Assuming that $\vec{g}=0$ and imposing the divergence-free condition of [darcy_mass]
to [darcy_velocity] leads to the following system of two equations in the unknowns
$p$ and $T$:

!equation
-\nabla \cdot \frac{\mathbf{K}}{\mu} \nabla p  = 0

!equation
\rho c_p \left( \frac{\partial T}{\partial t} + \epsilon \vec{u}\cdot\nabla T \right) - \nabla \cdot k \nabla T = 0

!---

The parameters $\rho$, $c_p$, and $k$ are the porosity-dependent density, specific heat capacity, and thermal
conductivity of the combined fluid/solid medium, defined by:

!equation
\rho \equiv \epsilon \rho_f + (1-\epsilon) \rho_s
\\
\rho c_p \equiv \epsilon \rho_f {c_p}_f + (1-\epsilon) \rho_s {c_p}_s
\\
k \equiv \epsilon k_f + (1-\epsilon) k_s

 where $\epsilon$ is the porosity, $c_p$ is the specific heat, and the subscripts $f$ and $s$ refer
 to fluid and solid, respectively.

!---

## Material Properties

| Property | Value | Units |
| :- | :- | :- |
| Viscosity of water, $\mu_f$ | $7.98\times10^{-4}$ |  $\textrm{P}\cdot\textrm{s}$ |
| Density of water, $\rho_f$ | 995.7 | $\textrm{kg}/\textrm{m}^3$ |
| Density of steel, $\rho_s$ | 8000 | $\textrm{kg}/\textrm{m}^3$ |
| Thermal conductivity of water, $k_f$ | 0.6 | $\textrm{W}/\textrm{m}\,\textrm{K}$ |
| Thermal conductivity of steel, $k_s$ | 18 | $\textrm{W}/\textrm{m}\,\textrm{K}$ |
| Specific heat capacity of water, $c_p{_f}$ | 4181.3 | $\textrm{J}/(\textrm{kg}\,\textrm{K})$ |
| Specific heat capacity of steel, $c_p{_s}$ | 466 | $\textrm{J}/(\textrm{kg}\,\textrm{K})$ |
