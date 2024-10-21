# NavierStokesFV Action

This class allows us to set up Navier-Stokes equations for porous medium or clean fluid flows using incompressible or weakly compressible approximations with a finite volume discretization.

!alert warning
This action is deprecated and is replaced by the `WCNSFVPhysics` classes. The deprecated
syntax can currently still be used. Please refer to the [section on how to transition to the new Physics syntax](NavierStokesFV/index.md#transition) for guidance on how to use the current syntax.

## Overview

This action is used for setting up the Navier-Stokes equations over a subdomain
using a finite volume discretization. Furthermore, the action is able to handle
regular (clean fluid flow) or porous medium (flow within homogenized structures)
formulations using incompressible and weakly-compressible approximations.
This action is triggered with */Modules/NavierStokesFV* input syntax.
For more information, visit [NavierStokesFV](/Modules/NavierStokesFV/index.md). For more information
regarding the friction implemenation, visit [Friction Kernel](PINSFVMomentumFriction.md)

## Example Input File Syntax

In this example, the equations, the wall/inlet/outlet boundary conditions and their parameters are all set automatically by the `NavierStokesFV` action.

!listing test/tests/finite_volume/ins/channel-flow/2d-rc-ambient-convection-action.i block=Modules/NavierStokesFV

!syntax parameters /Modules/NavierStokesFV
