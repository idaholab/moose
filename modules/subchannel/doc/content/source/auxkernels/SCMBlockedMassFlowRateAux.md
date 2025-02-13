# SCMBlockedMassFlowRateAux

!syntax description /AuxKernels/SCMBlockedMassFlowRateAux

## Overview

<!-- -->

In the case were there is a partial blockage in the inlet of the subchannel sub-assembly, the user can use the `SCMBlockedMassFlowRateAux` kernel to define an appropiate mass-flow distribution at the inlet.
The user must define the [!param](/AuxKernels/SCMBlockedMassFlowRateAux/variable) (must be `mdot`), a low mass-flux and a normal mass-flux, as well as the indexes of the blocked subchannels. The low mass-flux [!param](/AuxKernels/SCMBlockedMassFlowRateAux/blocked_mass_flux) will be
applied to the blocked subchannels who have the indexes [!param](/AuxKernels/SCMBlockedMassFlowRateAux/index_blockage). Ideally, the low mass-flux would be zero, but since that would lead to numerical instabilities
a low value is suggested instead. It is up to the user to decide what that value would be. The normal mass-flux [!param](/AuxKernels/SCMBlockedMassFlowRateAux/unblocked_mass_flux) will be applied to the unblocked subchannels.
The flow rate for each subchannel is defined as the product of the mass-flux $\times$ the surface area of each subchannel.
The user may opt to apply that condition at the inlet [!param](/AuxKernels/SCMBlockedMassFlowRateAux/boundary) or the whole domain.

## Example Input File Syntax

!listing /test/tests/auxkernels/mass_flow_rate/blocked_test.i block=AuxKernels language=cpp

!listing /examples/Blockage/FFM-2B.i block=AuxKernels language=cpp

!syntax parameters /AuxKernels/SCMBlockedMassFlowRateAux

!syntax inputs /AuxKernels/SCMBlockedMassFlowRateAux

!syntax children /AuxKernels/SCMBlockedMassFlowRateAux
