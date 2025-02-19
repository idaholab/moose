# SCMFlatMassFlowRateAux

!syntax description /AuxKernels/SCMFlatMassFlowRateAux

## Overview

<!-- -->

In the case were the user wants to define a uniform mass flow rate at the inlet, the `SCMFlatMassFlowRateAux`
kernel can be used. In this kernel the user must define a total [!param](/AuxKernels/SCMFlatMassFlowRateAux/mass_flow) rate  which will be divided equally among the subchannels.

## Example Input File Syntax

!listing /test/tests/auxkernels/uniform_mass_flow_rate/test.i block=AuxKernels language=cpp

!syntax parameters /AuxKernels/SCMFlatMassFlowRateAux

!syntax inputs /AuxKernels/SCMFlatMassFlowRateAux

!syntax children /AuxKernels/SCMFlatMassFlowRateAux
