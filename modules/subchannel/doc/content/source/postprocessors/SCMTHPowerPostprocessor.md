# SCMTHPowerPostprocessor

!syntax description /Postprocessors/SCMTHPowerPostprocessor

## Overview

!! this comment introduces vertical space

The user needs to specify a subchannel problem. Either a QuadSubChannel1PhaseProblem or a TriSubChannel1PhaseProblem. The postprocessor will calculate the net power that goes into the coolant based on the thermal-hydraulic balance of inlet and outlet.

## Example Input File Syntax

listing /test/tests/problems/SFR/sodium-19pin/test19_implicit.i block=Postprocessors language=moose

!syntax parameters /Postprocessors/SCMTHPowerPostprocessor

!syntax inputs /Postprocessors/SCMTHPowerPostprocessor

!syntax children /Postprocessors/SCMTHPowerPostprocessor
