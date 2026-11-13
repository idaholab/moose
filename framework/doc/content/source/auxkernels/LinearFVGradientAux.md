# LinearFVGradientAux

!syntax description /AuxKernels/LinearFVGradientAux

## Overview

This `AuxKernel` copies the cell-centered gradient of a [MooseLinearVariableFV.md] into a
constant monomial vector auxiliary variable. By default it reads the gradient computed with the
method set by the `gradient_method` parameter of the variable. Setting
[!param](/AuxKernels/LinearFVGradientAux/gradient_method) instead reads the result of any other
built-in method or object in `[FVGradientMethods]`, and registers that method for the variable if
no other object has. Gradients from methods other than the default of the variable cannot be
accessed through the functor system, so this object is the way to output or postprocess them,
for example with [ElementVectorL2Error.md].

## Example Input File Syntax

In this example, the gradient of `u` is copied using both its default Green-Gauss method and a
limited Green-Gauss method:

!listing test/tests/variables/linearfv/gradient-aux.i block=AuxKernels FVGradientMethods

!syntax parameters /AuxKernels/LinearFVGradientAux

!syntax inputs /AuxKernels/LinearFVGradientAux

!syntax children /AuxKernels/LinearFVGradientAux
