# NavierStokesFV System

## Overview

The NavierStokesFV system is dedicated to decrease the effort required by the user to
prepare simulations which need to solve the Navier Stokes equations. The action
is capable of setting up:

- +Incompressible+ and +weakly-compressible+ simulations for
- +Clean fluids+ and flows in +porous media+

The action handles boundary conditions, necessary materials and objects for
different, finite volume discretization approaches.

## Automatically defined variables

The NavierStokesFV action automatically sets up the variables which are
necessary for the solution of a given problem. These variables can then be used
to couple fluid flow simulations with other physics. The list of variable names
commonly used in the action syntax is presented below:

!listing include/base/NS.h

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

The same simulation can be set up using the action syntax which improves
input file readability:

!listing modules/navier_stokes/test/tests/finite_volume/ins/lid-driven/lid-driven-with-energy-action.i

### Incompressible fluid flow in porous medium

The following input file sets up a simulation of an incompressible fluid flow
within a channel which contains a homogenized structure that is treated as a
porous medium. The model accounts for the heat exchange between the fluid and the
homogenized structure as well. For more description on the used model, visit
the [Porous medium Incompressible Navier Stokes](modules/navier_stokes/pinsfv.md) page.
First, the input file with the manually defined kernels and boundary conditions
is presented:

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/heated/2d-rc-heated.i

The same simulation can also be set up using the NavierStokesFV action syntax:

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/heated/2d-rc-heated-action.i

### Weakly-compressible fluid flow

The last example is dedicated to demonstrate a transient flow in a channel
using a weakly-compressible approximation. The following examples shows how
this simulation is set up by manually defining the kernels and boundary conditions.
For more information on the weakly-compressible treatment, visit
the [Weakly-compressible Navier Stokes](modules/navier_stokes/wcnsfv.md) page.

!listing modules/navier_stokes/test/tests/finite_volume/wcns/channel-flow/2d-transient.i

The same simulation can be set up using the action syntax as folows:

!listing modules/navier_stokes/test/tests/finite_volume/wcns/channel-flow/2d-transient-action.i

## Example Input File Syntax

!syntax list /Modules/NavierStokesFV objects=True actions=False subsystems=False

!syntax list /Modules/NavierStokesFV objects=False actions=False subsystems=True

!syntax list /Modules/NavierStokesFV objects=False actions=True subsystems=False
