# NSFVAction

!syntax description /Modules/NavierStokesFV/NSFVAction

## Overview

This action is used for setting up the Navier-Stokes equations over a subdomain
using a finite volume discretization. Furthermore, the action is able to handle
regular (clean fluid flow) or porous medium (with homogenized structures)
formulations using incompressible and weakly-compressible approximations.
This action is triggered with */Modules/NavierStokesFV* input syntax.
For more information, visit [NavierStokesFV](/Modules/NavierStokesFV/index.md).

## Example Input File Syntax

!syntax parameters /Modules/NavierStokesFV
