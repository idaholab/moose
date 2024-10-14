# MassFlowRateAux

!syntax description /AuxKernels/MassFlowRateAux

## Overview

<!-- -->

This is the usual kernel that the user can use to define the mass flow rate distribution at the inlet boundary.
In this case the user must define a uniform mass flux (`mass_flux`) which suggest a more or less uniform velocity
profile at the inlet. This mass flux can be either a `Real` or a [Postprocessor](/Postprocessors/index.md).
The mass flow rate for each subchannel is calculated as the product of the mass flux $\times$
the surface area of each subchannel.

## Example Input File Syntax

!listing /examples/psbt/psbt_ss/psbt.i block=AuxKernels language=cpp

!syntax parameters /AuxKernels/MassFlowRateAux

!syntax inputs /AuxKernels/MassFlowRateAux

!syntax children /AuxKernels/MassFlowRateAux
