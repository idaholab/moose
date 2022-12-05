# FunctorElementalGradientAux

!syntax description /AuxKernels/FunctorElementalGradientAux

## Overview

This `AuxKernel` outputs a vector `AuxVariable` given by scaling the gradient of a functor scaled by a material property and/or another functor.

!alert note title=Use in Optimization module
This `AuxVariable` is used to linearize a nonlinear material property in the adjoint equation.  

## Example Input File Syntax

The use of this `AuxKernel` to compute the adjoint for a temperature dependent material is shown in this example:

!listing modules/combined/test/tests/invOpt_nonlinear/adjoint.i block=AuxKernels

where the `AuxVariable` dDdTgradT is defined as a constant monomial vector:

!listing modules/combined/test/tests/invOpt_nonlinear/adjoint.i block=AuxVariables/dDdTgradT  

In this example, [!param](/AuxKernels/FunctorElementalGradientAux/factor_matprop) is the tangent modulus of the temperature dependent thermal conductivity.
`FunctorElementalGradientAux` provides an advection velocity in the adjoint problem solved by the `LevelSetAdvection` kernel.
For Hessian based inversion, `FunctorElementalGradientAux` provides the advection velocity to the `ConservativeAdvection` kernel.

!syntax parameters /AuxKernels/FunctorElementalGradientAux

!syntax inputs /AuxKernels/FunctorElementalGradientAux

!syntax children /AuxKernels/FunctorElementalGradientAux
