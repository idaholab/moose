# MaterialScaledGradientVector.md

!syntax description /AuxKernels/MaterialScaledGradientVector

## Overview

This AuxKernel outputs a vector AuxVariable given by scaling the gradient of a variable by a scaling factored provided by a material property.  This auxVariable is used to linearize a nonlinear material property in the adjoint equation.  The use of this AuxKernel to compute the adjoint for a temperature dependent material is shown in this example:

!listing modules/combined/test/tests/optimizationreporter/objective_gradient_minimize/nonlinear/adjoint.i

The material used for scaling provides the derivative of the thermal conductivity with respect to temperature.  The nonlinear material property produces an advection term in the adjoint problem solved by the LevelSetAdvection kernel.  

!syntax parameters /AuxKernels/MaterialScaledGradientVector.md

!syntax inputs /AuxKernels/MaterialScaledGradientVector.md

!syntax children /AuxKernels/MaterialScaledGradientVector.md
