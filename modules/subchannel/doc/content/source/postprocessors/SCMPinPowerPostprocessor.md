# SCMPinPowerPostprocessor

!syntax description /Postprocessors/SCMPinPowerPostprocessor

## Overview

!! this comment introduces vertical space

The user needs to specify a subchannel problem. Either a [QuadSubChannel1PhaseProblem.md] or a [TriSubChannel1PhaseProblem.md]. The postprocessor will calculate the total heat rate $[W]$ that goes into the coolant, based on the distribution of the aux variable q_prime $[W/m]$, on the fuel pins or the subchannels. It will integrate this variable over the heated section using an axial trapezoidal rule.

## Example Input File Syntax

!listing /test/tests/SCMQuadPower/test.i block=Postprocessors language=moose
!listing /test/tests/SCMTriPower/test.i block=Postprocessors language=moose

!syntax parameters /Postprocessors/SCMPinPowerPostprocessor

!syntax inputs /Postprocessors/SCMPinPowerPostprocessor

!syntax children /Postprocessors/SCMPinPowerPostprocessor
