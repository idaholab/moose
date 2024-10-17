# UniformlyDistributedMassFlowRateAux

!syntax description /AuxKernels/UniformlyDistributedMassFlowRateAux

## Overview

<!-- -->

In the case were the user wants to define a uniform mass flow rate at the inlet, the `UniformlyDistributedMassFlowRateAux`
kernel can be used. In this kernel the user must define a total [!param](/AuxKernels/UniformlyDistributedMassFlowRateAux/mass_flow) rate  which will be divided equally among the subchannels.

## Example Input File Syntax

!listing /test/tests/auxkernels/uniform_mass_flow_rate/test.i block=AuxKernels language=cpp

!syntax parameters /AuxKernels/UniformlyDistributedMassFlowRateAux

!syntax inputs /AuxKernels/UniformlyDistributedMassFlowRateAux

!syntax children /AuxKernels/UniformlyDistributedMassFlowRateAux
