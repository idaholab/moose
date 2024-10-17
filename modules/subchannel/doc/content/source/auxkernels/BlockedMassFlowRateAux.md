# BlockedMassFlowRateAux

!syntax description /AuxKernels/BlockedMassFlowRateAux

## Overview

<!-- -->

In the case were there is a partial blockage in the inlet of the subchannel sub-assembly, the user can use the `BlockedMassFlowRateAux` kernel to define an appropiate mass-flow distribution at the inlet.
The user must define the [!param](/AuxKernels/BlockedMassFlowRateAux/variable) (must be `mdot`), a low mass-flux and a normal mass-flux, as well as the indexes of the blocked subchannels. The low mass-flux [!param](/AuxKernels/BlockedMassFlowRateAux/blocked_mass_flux) will be
applied to the blocked subchannels who have the indexes [!param](/AuxKernels/BlockedMassFlowRateAux/index_blockage). Ideally, the low mass-flux would be zero, but since that would lead to numerical instabilities
a low value is suggested instead. It is up to the user to decide what that value would be. The normal mass-flux [!param](/AuxKernels/BlockedMassFlowRateAux/unblocked_mass_flux) will be applied to the unblocked subchannels.
The flow rate for each subchannel is defined as the product of the mass-flux $\times$ the surface area of each subchannel.
The user may opt to apply that condition at the inlet [!param](/AuxKernels/BlockedMassFlowRateAux/boundary) or the whole domain.

## Example Input File Syntax

!listing /test/tests/auxkernels/mass_flow_rate/blocked_test.i block=AuxKernels language=cpp

!listing /examples/Blockage/FFM-2B.i block=AuxKernels language=cpp

!syntax parameters /AuxKernels/BlockedMassFlowRateAux

!syntax inputs /AuxKernels/BlockedMassFlowRateAux

!syntax children /AuxKernels/BlockedMassFlowRateAux
