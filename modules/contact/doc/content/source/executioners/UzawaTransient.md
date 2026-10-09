# UzawaTransient

!syntax description /Executioner/UzawaTransient

## Description

`Transient` subclass that runs a 1D outer Newton on the load-control
scalar $s$ around the standard per-time-step SNES solve for
$(u, \lambda)$ with $s$ held fixed.  See the
[theory page](modules/contact/rigid_contact/theory.md#the-uzawa-outer-executioner)
for the split and the derivation of the outer scalar
Newton step size.

Displacement-controlled runs --- inputs with no
[!param](/Executioner/UzawaTransient/load_control_kernel) set --- fall
through the outer loop and behave identically to a plain `Transient`.

The [Vickers example](modules/contact/rigid_contact/examples/material_into_indenter.md) 
is the shipped end-to-end demonstration.

## Parameters

!syntax parameters /Executioner/UzawaTransient
