# SCMDuctPowerPostprocessor

!syntax description /Postprocessors/SCMDuctPowerPostprocessor

## Overview

!! this comment introduces vertical space

The user needs to specify a subchannel problem. Either a QuadSubChannel1PhaseProblem or a TriSubChannel1PhaseProblem. The postprocessor will calculate the net power that goes into the coolant through the duct, based on the aux variable duct_heat_flux. It will integrate this variable over the duct surface using the trapezoidal rule.

## Example Input File Syntax

<!-- !listing /test/tests/SCMQuadPower/test.i block=Postprocessors language=moose
!listing /test/tests/SCMTriPower/test.i block=Postprocessors language=moose -->

!syntax parameters /Postprocessors/SCMDuctPowerPostprocessor

!syntax inputs /Postprocessors/SCMDuctPowerPostprocessor

!syntax children /Postprocessors/SCMDuctPowerPostprocessor
