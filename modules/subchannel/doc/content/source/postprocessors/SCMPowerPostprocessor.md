# SCMPowerPostprocessor

!syntax description /Postprocessors/SCMPowerPostprocessor

## Overview

!! this comment introduces vertical space

The user needs to specify a subchannel problem. Either a QuadSubChannel1PhaseProblem or a TriSubChannel1PhaseProblem. The postprocessor will calculate the total power of the assembly based on the distribution of the aux variable q_prime, on the fuel pins or the subchannels. It will integrate this variable over the heated section using the trapezoidal rule.

## Example Input File Syntax

!listing /test/tests/SCMQuadPower/test.i block=Postprocessors language=moose
!listing /test/tests/SCMTriPower/test.i block=Postprocessors language=moose

!syntax parameters /Postprocessors/SCMPowerPostprocessor

!syntax inputs /Postprocessors/SCMPowerPostprocessor

!syntax children /Postprocessors/SCMPowerPostprocessor
