# FunctorElementalAux

!syntax description /AuxKernels/FunctorElementalAux

## Overview

This object populates an elemental auxiliary variable by evaluating a functor
with an element argument. This functor may be a material property, a function or another
variable.

!alert note
The version of this auxiliary kernel for automatic differentiation (AD) functors
(in particular AD material properties) is `ADFunctorElementalAux`.

## Example input syntax

In this example, we use `FunctorElementalAux` to convert some material properties functors, defined by the fluid
properties material, to auxiliary variables, to examine them in an Exodus output.

!listing modules/navier_stokes/test/tests/finite_volume/wcns/materials/functorfluidprops.i block=AuxKernels

!syntax parameters /AuxKernels/FunctorElementalAux

!syntax inputs /AuxKernels/FunctorElementalAux

!syntax children /AuxKernels/FunctorElementalAux
