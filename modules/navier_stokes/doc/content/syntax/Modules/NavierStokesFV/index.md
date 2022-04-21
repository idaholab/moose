# NavierStokesFV System

## Overview

The NavierStokesFV system is dedicated to decrease the effort required by the user to
prepare simulations which need to solve the Navier Stokes equations. The action
is capable of setting up:

- +Incompressible+ and +weakly-compressible+ simulations for
- +Clean fluids+ and flows in +porous media+

The action handles boundary conditions, necessary materials for fluid properties,
variables, variable initialization and various objects for turbulence modeling.

!alert! note

This action only supports Rhie-Chow interpolation for the determination
of face velocities in the advection terms. The face interpolation of the
advected quantities (e.g. upwind, average) can be controlled through the
`*_advection_interpolation` action parameters.

!alert-end!

## Automatically defined variables

The NavierStokesFV action automatically sets up the variables which are
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

In the following examples we present how the NavierStokesFV action can be
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
we recommend using first order `upwind` or second order TVD chemes.

!alert-end!

The same simulation can be set up using the action syntax which improves
input file readability:

!listing modules/navier_stokes/test/tests/finite_volume/ins/lid-driven/lid-driven-with-energy-action.i

It is visible that in this case we defined the [!param](/Modules/NavierStokesFV/compressibility)
parameter to be `incompressible`.
Furthermore, the energy (enthalpy) equation is solved as well. The user can
request this by setting [!param](/Modules/NavierStokesFV/add_energy_equation)
to `true`. The boundary types are grouped into +wall+, +inlet+ and +outlet+ types.
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
we recommend using first order `upwind` or second order TVD chemes.

!alert-end!

The same simulation can also be set up using the NavierStokesFV action syntax:

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/heated/2d-rc-heated-action.i

Compared to the previous example, we see that in this case the porous medium
treatment is enabled by setting [!param](/Modules/NavierStokesFV/porous_medium_treatment)
to `true`. The corresponding porosity can be supplied through the
[!param](/Modules/NavierStokesFV/porosity) parameter. Furthermore, the heat exchange
between the fluid and the homogenized structure is enabled using the
[!param](/Modules/NavierStokesFV/ambient_temperature) and
[!param](/Modules/NavierStokesFV/ambient_convection_alpha) parameters.


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
we recommend using first order `upwind` or second order TVD chemes.

!alert-end!

The same simulation can be set up using the action syntax as folows:

!listing modules/navier_stokes/test/tests/finite_volume/wcns/channel-flow/2d-transient-action.i

We note that the weakly-compressible handling can be enabled by setting
[!param](/Modules/NavierStokesFV/compressibility) to `weakly-compressible`.
As shown in the example, an arbitrary
energy source function can also be supplied to the incorporated
energy equation using the [!param](/Modules/NavierStokesFV/external_heat_source) parameter.



!syntax list /Modules/NavierStokesFV objects=True actions=False subsystems=False

!syntax list /Modules/NavierStokesFV objects=False actions=False subsystems=True

!syntax list /Modules/NavierStokesFV objects=False actions=True subsystems=False
