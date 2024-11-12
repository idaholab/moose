# SCMMassFlowRateAux

!syntax description /AuxKernels/SCMMassFlowRateAux

## Overview

<!-- -->

This is the usual kernel that the user can use to define the mass-flow rate distribution at the inlet [!param](/AuxKernels/SCMMassFlowRateAux/boundary).
In addition to the boundary, the user must define the [!param](/AuxKernels/SCMMassFlowRateAux/variable) (must be `mdot`) and a uniform [!param](/AuxKernels/SCMMassFlowRateAux/mass_flux),
which suggest a more or less uniform velocity profile at the boundary. This mass-flux can be either a `Real` or a [Postprocessor](/Postprocessors/index.md).
The mass-flow rate for each subchannel is calculated as the product of the mass-flux $\times$ the surface area of each subchannel at the boundary.

## Example Input File Syntax

!listing /examples/psbt/psbt_ss/psbt.i block=AuxKernels language=cpp

!syntax parameters /AuxKernels/SCMMassFlowRateAux

!syntax inputs /AuxKernels/SCMMassFlowRateAux

!syntax children /AuxKernels/SCMMassFlowRateAux
