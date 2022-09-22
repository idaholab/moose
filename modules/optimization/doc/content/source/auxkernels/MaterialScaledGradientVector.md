# MaterialScaledGradientVector.md

!syntax description /AuxKernels/MaterialScaledGradientVector

## Overview

This `AuxKernel` outputs a vector `AuxVariable` given by scaling the gradient of a variable by a scaling factor provided by a material property.  This auxVariable is used to linearize a nonlinear material property in the adjoint equation.  

## Example Input File Syntax

The use of this `AuxKernel` to compute the adjoint for a temperature dependent material is shown in this example:

!listing modules/combined/test/tests/invOpt_nonlinear/adjoint.i block=AuxKernels

where the `AuxVariable` dDdTgradT is defined as a constant monomial vector:

!listing modules/combined/test/tests/invOpt_nonlinear/adjoint.i block=AuxVariables/dDdTgradT  

In this example, [!param](/AuxKernels/MaterialScaledGradientVector/material_scaling) is the tangent modulus of the temperature dependent thermal conductivity.  `MaterialScaledGradientVector` provides an advection velocity in the adjoint problem solved by the `LevelSetAdvection` kernel.  For Hessian based inversion, `MaterialScaledGradientVector` provides the advection velocity to the `ConservativeAdvection` kernel.

!syntax parameters /AuxKernels/MaterialScaledGradientVector

!syntax inputs /AuxKernels/MaterialScaledGradientVector

!syntax children /AuxKernels/MaterialScaledGradientVector
