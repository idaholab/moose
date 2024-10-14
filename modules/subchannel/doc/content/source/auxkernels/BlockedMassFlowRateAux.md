# BlockedMassFlowRateAux

!syntax description /AuxKernels/BlockedMassFlowRateAux

## Overview

<!-- -->

In the case were there is a partial blockage in the inlet of the subchannel sub-assembly,
the user can use the `BlockedMassFlowRateAux` kernel to define an appropiate mass flow distribution at the inlet.
The user must define a low mass-flux and a normal mass-flux. The low mass-flux (`blocked_mass_flux`) will be
applied to the blocked subchannels. Ideally that value would be zero, but since that would lead to numerical issues a low value is suggested instead.
The normal mass-flux (`unblocked_mass_flux`) will be applied to the unblocked subchannels. The flow rate for each subchannel is defined as the product
of the mass-flux $\times$ the surface area of each subchannel.

## Example Input File Syntax

!listing /test/tests/auxkernels/mass_flow_rate/blocked_test.i block=AuxKernels language=cpp

!listing /examples/Blockage/FFM-2B.i block=AuxKernels language=cpp

!syntax parameters /AuxKernels/BlockedMassFlowRateAux

!syntax inputs /AuxKernels/BlockedMassFlowRateAux

!syntax children /AuxKernels/BlockedMassFlowRateAux
