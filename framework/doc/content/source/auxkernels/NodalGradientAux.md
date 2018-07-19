# Nodal Gradient Aux

!syntax description /AuxKernels/NodalGradientAux

## Description

This AuxKernel `NodalGradientAux` is derived from `NodalPatchRecovery` and implements the `computeValue` method by calculating the gradient at quadrature point.

This AuxKernel is intended for use to post process a smooth gradient field.
## Example Input File Syntax

!listing test/tests/auxkernels/nodal_gradient/refine_0.i block=AuxKernels/u_x_patch

An AuxVariable is required to store the `NodalGradientAux` AuxKernel information.

!syntax parameters /AuxKernels/NodalGradientAux

!syntax inputs /AuxKernels/NodalGradientAux

!syntax children /AuxKernels/NodalGradientAux
