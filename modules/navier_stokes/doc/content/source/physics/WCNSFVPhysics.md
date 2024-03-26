# WCNSFVPhysics

## Overview

The `WCNSFVPhysics` are dedicated to decrease the effort required by the user to
prepare simulations which need to solve the Navier Stokes equations. The action
is capable of setting up:

- +Incompressible+ and +weakly-compressible+ simulations for
- +Clean fluids+ and flows in +porous media+ with
- a fully coupled single-nonlinear system approach

The action handles boundary conditions, necessary materials for fluid properties,
variables, variable initialization and various objects for turbulence modeling.

The weakly-compressible finite volume discretization of the Navier Stokes flow equations is
described in further details on this [page](modules/navier_stokes/wcnsfv.md).

!alert note
This input syntax is replacing the `Modules/NavierStokesFV` syntax for flow equations with `Physics/NavierStokes/WCNSFV------/<a name>`.
The additional innermost level of nesting enables the definition of multiple sets of flow equations in a single simulation.

## Automatically defined variables

The `WCNSFVPhysics`-derived classes automatically sets up the variables which are
necessary for the solution of a given problem. These variables can then be used
to couple fluid flow simulations with other physics. The list of variable names
commonly used in the action syntax is presented below:

- Velocities for non-porous-medium simulations:

  !listing include/base/NS.h start=std::string velocity_x end=std::string velocity_z include-end=true

- Velocities for porous medium simulations:

  !listing include/base/NS.h start=std::string superficial_velocity_x end=std::string superficial_velocity_z include-end=true

- Pressure and temperature:

  !listing include/base/NS.h line=pressure

  !listing include/base/NS.h line=std::string T_fluid

For the default names of other variables used in this action, visit [this site](include/base/NS.h).

## Examples

### Incompressible fluid flow in a lid-driven cavity

In the following examples we present how the `WCNSFVPhysics` can be
utilized to simplify input files. We start with the simple lid-driven cavity problem
with an [Incompressible Navier Stokes](modules/navier_stokes/insfv.md) formulation.
The original description of the problem is available under the following
[link](modules/navier_stokes/insfv.md). First, we present the input file where
the simulation is set up manually, by defining every kernel, boundary condition and
material explicitly.

!listing modules/navier_stokes/test/tests/finite_volume/ins/lid-driven/lid-driven-with-energy.i

!alert! note

Careful! The utilization of central difference (`average`) advected interpolation
may lead to oscillatory behavior in certain scenarios. Even though it is not the case
for this example, if this phenomenon arises,
we recommend using first order `upwind` or second order TVD schemes.

!alert-end!

The same simulation can be set up using the `Physics` syntax which improves
input file readability:

!listing modules/navier_stokes/test/tests/finite_volume/ins/lid-driven/lid-driven-with-energy-physics.i

It is visible that in this case we defined the [!param](/Physics/NavierStokes/WCNSFVFlow/compressibility)
parameter to be `incompressible`.
Furthermore, the energy (enthalpy) equation is solved as well, with the addition of a [WCNSFVHeatAdvectionPhysics.md].
The boundary types are grouped into +wall+, +inlet+ and +outlet+ types.
For more information on the available boundary types, see the
list of parameters at the bottom of the page.

### Incompressible fluid flow in porous medium

The following input file sets up a simulation of an incompressible fluid flow
within a channel which contains a homogenized structure that is treated as a
porous medium. The model accounts for the heat exchange between the fluid and the
homogenized structure as well. For more description on the used model, visit
the [Porous medium Incompressible Navier Stokes](modules/navier_stokes/pinsfv.md) page.
First, the input file with the manually defined kernels and boundary conditions
is presented:

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/heated/2d-rc-heated.i

!alert! note

Careful! The utilization of central difference (`average`) advected interpolation
may lead to oscillatory behavior in certain scenarios. Even though it is not the case
for this example, if this phenomenon arises,
we recommend using first order `upwind` or second order TVD schemes.

!alert-end!

The same simulation can also be set up using the `WCNSFVFlowPhysics` syntax:

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/heated/2d-rc-heated-physics.i

Compared to the previous example, we see that in this case the porous medium
treatment is enabled by setting [!param](/Physics/NavierStokes/WCNSFVFlow/porous_medium_treatment)
to `true`. The corresponding porosity can be supplied through the
[!param](/Physics/NavierStokes/WCNSFVFlow/porosity) parameter.

The energy (enthalpy) equation is solved as well, with the addition of a [WCNSFVHeatAdvectionPhysics.md].
Furthermore, the heat exchange
between the fluid and the homogenized structure is enabled using the
[!param](/Physics/NavierStokes/WCNSFVFlow/ambient_temperature) and
[!param](/Physics/NavierStokes/WCNSFVFlow/ambient_convection_alpha) parameters.

### Weakly-compressible fluid flow

The last example is dedicated to demonstrating a transient flow in a channel
using a weakly-compressible approximation. The following examples shows how
this simulation is set up by manually defining the kernels and boundary conditions.
For more information on the weakly-compressible treatment, visit
the [Weakly-compressible Navier Stokes](modules/navier_stokes/wcnsfv.md) page.

!listing modules/navier_stokes/test/tests/finite_volume/wcns/channel-flow/2d-transient.i

!alert! note

Careful! The utilization of central difference (`average`) advected interpolation
may lead to oscillatory behavior in certain scenarios. Even though it is not the case
for this example, if this phenomenon arises,
we recommend using first order `upwind` or second order TVD schemes.

!alert-end!

The same simulation can be set up using the `NavierStokes/WCNSFVPhysics` syntax as follows:

!listing modules/navier_stokes/test/tests/finite_volume/wcns/channel-flow/2d-transient-physics.i

We note that the weakly-compressible handling can be enabled by setting
[!param](/Physics/NavierStokes/WCNSFVFlow/compressibility) to `weakly-compressible`.

The energy (enthalpy) equation is solved as well, with the addition of a [WCNSFVHeatAdvectionPhysics.md].
As shown in the example, an arbitrary
energy source function can also be supplied to the incorporated
energy equation using the [!param](/Physics/NavierStokes/WCNSFVFlow/external_heat_source) parameter.

