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
[#step12]\\
[#step13]

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
T = T_{\mathrm{b}} [K] \in \Omega_D

Neumann boundary conditions:

!equation
-k\nabla T\cdot\vec{n} = q'' [W/m^2] \in \Omega_N

or convective boundary conditions:

!equation
-k\nabla T\cdot\vec{n} = h\left(T - T_{\inf}\right) \in \Omega_C

!---

## [#step04]

Instead of passing constant parameters to the heat conduction `Kernel` object, the Material
system can be used to supply the values. This allows for properties that vary in space and time
as well as be coupled to variables in the simulation.

!---

## [#step05]

The heat flux can be compute from the temperature as:

!equation
[q_x, q_y, q_z] = -k \nabla T

This velocity can be computed using the Auxiliary system.

!---

## [#step06]

Solve the transient heat equation using the "heat transfer" module.

!equation
\rho c_p \frac{\partial T}{\partial t} - \nabla \cdot k \nabla T = 0

!---

## [#step07]

Thermal expansion of the concrete can be added to the coupled set of equations
using the "solid mechanics" module, without adding additional code.

!equation
\nabla \cdot (\boldsymbol{\sigma} + \boldsymbol{\sigma}_0) + \boldsymbol{b} = \boldsymbol{0}

!---

## [#step08]

In the transient simulation, a "traveling wave" profile moves through the concrete as it heats up.
Adaptivity lets us resolve the temperature gradient at lower computational cost.

!---

## [#step09]

`Postprocessor` and `VectorPostprocessor` objects can be used to compute aggregate value(s) for a
simulation, such as the average temperature on the boundary or the temperatures along a line
within the solution domain.

!equation
\bar{T} = \frac{\int_{\Omega} T d\Omega}{\int_{\Omega}d\Omega}

!---

## [#step10]

Solve the pressure, velocity and temperature in a coupled system of equations by solving for heat transfer
in the fluid region

Conservation of Mass (incompressible):

!equation
\nabla \cdot \vec{u} = 0

Conservation of momentum:

!equation
\frac{\partial \rho  \vec{u}}{\partial t} + \nabla \cdot \left(\rho \vec{u} \otimes \vec{u}\right)
= \nabla \cdot \left(\mu_\text{eff} \left(\nabla\vec{u}_I + \left(\nabla \vec{u}_I\right)^T-\frac{2}{3}\nabla\cdot\vec{u}_I\mathbb{I}\right)\right) -\nabla p + \rho \vec{g}

Conservation of Energy:

!equation
\rho c_p\left( \frac{\partial T}{\partial t} + \vec{u}\cdot\nabla T \right) - \nabla\cdot k \nabla T = 0

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

!---

## [#step13]

Learn how to recover a MOOSE simulation that ended prematurely.
