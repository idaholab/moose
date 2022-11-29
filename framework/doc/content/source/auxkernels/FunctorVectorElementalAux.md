# FunctorVectorElementalAux

!syntax description /AuxKernels/FunctorVectorElementalAux

## Overview

This object populates an elemental auxiliary variable by evaluating a functor vector
(functor material properties only currently) with an element argument, and selecting one of
its component using the [!param](/AuxKernels/FunctorVectorElementalAux/component) parameter.

!alert note
The version of this auxiliary kernel for automatic differentiation (AD) material properties is
`ADFunctorVectorElementalAux`.

!syntax parameters /AuxKernels/FunctorVectorElementalAux

!syntax inputs /AuxKernels/FunctorVectorElementalAux

!syntax children /AuxKernels/FunctorVectorElementalAux
