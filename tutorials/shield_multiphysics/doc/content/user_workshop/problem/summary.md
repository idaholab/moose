# Tutorial Steps

!style halign=center
[#step01]\\
[#step02]\\
[#step03]\\
[#step04]\\
[#step05]\\
[#step06]\\
[#step07]\\
[#step08]\\
[#step09]\\
[#step10]\\
[#step11]\\
[#step12]

!---

## [#step01]

The first step is to solve a simple "Diffusion" problem, which requires no code. This step
will introduce the basic system of MOOSE.

!equation
-\nabla\cdot\nabla T = 0

!---

## [#step02]

In order to implement the heat conduction equation, a `Kernel` object is needed to represent:

!equation
-\nabla\cdot (k\nabla T) = 0

!---

## [#step03]

We represent the boundary conditions using `BC` objects. For example, Dirichlet boundary conditions:

!equation
T = 300 K

or Neumann boundary conditions

!equation
q = 30 W/m^2

!---

## [#step04]

Instead of passing constant parameters to the heat conduction `Kernel` object, the Material
system can be used to supply the values. This allows for properties that vary in space and time
as well as be coupled to variables in the simulation.

!---

## [#step05]

The heat flux can be compute from the temperature as:

!equation
q = -k \nabla T

This velocity can be computed using the Auxiliary system.

!---

## [#step06]

Solve the transient heat equation using the "heat transfer" module.

!equation
\rho c \frac{\partial T}{\partial t} - \nabla \cdot k \nabla T = 0

!---

## [#step07]

Solve the pressure, velocity and temperature in a coupled system of equations by solving for conjugate heat transfer
with the fluid region

!equation
\rho_c c_c \frac{\partial T_c}{\partial t} - \nabla \cdot k_c \nabla T_c = 0
\\
\nabla \cdot \vec{u} = 0
\\
\frac{\partial \rho_w  \vec{u}}{\partial t} + \nabla \cdot \left(\rho_w \vec{u} \otimes \vec{u}\right)
= \nabla \cdot \left(\mu_{w,\text{eff}} \left(\nabla\vec{u}_I + \left(\nabla \vec{u}_I\right)^T-\frac{2}{3}\nabla\cdot\vec{u}_I\mathbb{I}\right)\right) -\nabla p + \rho_w \vec{g}
\\
\rho_w c_w \left( \frac{\partial T_w}{\partial t} + \vec{u}\cdot\nabla T_w \right) - \nabla\cdot k_w \nabla T_w = 0

!---

## [#step08]

In the transient simulation, a "traveling wave" profile moves through the concrete as it heats up.
Adaptivity lets us resolve the temperature gradient at lower computational cost.

!---

## [#step09]

`Postprocessor` and `VectorPostprocessor` objects can be used to compute aggregate value(s) for a
simulation, such as the average temperature on the boundary or the temperatures along a line
within the solution domain.

!---

## [#step10]

Thermal expansion of the concrete can be added to the coupled set of equations
using the "solid mechanics" module, without adding additional code.

!---

## [#step11]

MOOSE is capable of running multiple applications together and transfer data between the various
applications.

We introduce distributed thermal calculations for neutron detectors, placing the detectors
in various locations in the concrete.

!---

## [#step12]

MOOSE includes a system to create custom input syntax for common tasks, in this step the syntax
for the sets of equations are simplified for end-users.
